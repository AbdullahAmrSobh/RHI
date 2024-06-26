#include "RHI-Vulkan/Loader.hpp"

#include "Common.hpp"
#include "Resources.hpp"
#include "CommandList.hpp"
#include "Swapchain.hpp"
#include "RenderGraphCompiler.hpp"
#include "Context.hpp"
#include "VulkanFunctions.hpp"

#include <tracy/Tracy.hpp>

#include <format>

#if RHI_PLATFORM_WINDOWS
    #define VULKAN_SURFACE_OS_EXTENSION_NAME "VK_KHR_win32_surface"
#elif RHI_PLATFORM_MACOS
    #define VULKAN_SURFACE_OS_EXTENSION_NAME VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#elif RHI_PLATFORM_ANDROID
    #define VULKAN_SURFACE_OS_EXTENSION_NAME VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#elif RHI_PLATFORM_XLIB
    #define VULKAN_SURFACE_OS_EXTENSION_NAME VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#elif RHI_PLATFORM_WAYLAND
    #define VULKAN_SURFACE_OS_EXTENSION_NAME VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
#elif RHI_PLATFORM_ANDROID
    #define VULKAN_SURFACE_OS_EXTENSION_NAME VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#elif RHI_PLATFORM_IOS
    #define VULKAN_SURFACE_OS_EXTENSION_NAME VK_MVK_IOS_SURFACE_EXTENSION_NAME
#endif // VK_USE_PLATFORM_WIN32_KHR

namespace RHI
{
    Ptr<Context> CreateVulkanContext(const ApplicationInfo& appInfo, Ptr<DebugCallbacks> debugCallbacks)
    {
        ZoneScoped;

        auto context = CreatePtr<Vulkan::IContext>(std::move(debugCallbacks));
        auto result = context->Init(appInfo);
        RHI_ASSERT(IsSucess(result));
        return std::move(context);
    }
} // namespace RHI

namespace RHI::Vulkan
{
    inline static TL::Vector<VkLayerProperties> GetAvailableInstanceLayerExtensions()
    {
        uint32_t instanceLayerCount;
        Validate(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
        TL::Vector<VkLayerProperties> layers;
        layers.resize(instanceLayerCount);
        Validate(vkEnumerateInstanceLayerProperties(&instanceLayerCount, layers.data()));
        return layers;
    }

    inline static TL::Vector<VkExtensionProperties> GetAvailableInstanceExtensions()
    {
        uint32_t instanceExtensionsCount;
        Validate(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, nullptr));
        TL::Vector<VkExtensionProperties> extensions;
        extensions.resize(instanceExtensionsCount);
        Validate(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, extensions.data()));
        return extensions;
    }

    inline static TL::Vector<VkLayerProperties> GetAvailableDeviceLayerExtensions(VkPhysicalDevice physicalDevice)
    {
        uint32_t instanceLayerCount;
        Validate(vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, nullptr));
        TL::Vector<VkLayerProperties> layers;
        layers.resize(instanceLayerCount);
        Validate(vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, layers.data()));
        return layers;
    }

    inline static TL::Vector<VkExtensionProperties> GetAvailableDeviceExtensions(VkPhysicalDevice physicalDevice)
    {
        uint32_t extensionsCount;
        Validate(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr));
        TL::Vector<VkExtensionProperties> extnesions;
        extnesions.resize(extensionsCount);
        Validate(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, extnesions.data()));
        return extnesions;
    }

    inline static TL::Vector<VkPhysicalDevice> GetAvailablePhysicalDevices(VkInstance instance)
    {
        uint32_t physicalDeviceCount;
        Validate(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
        TL::Vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
        Validate(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));
        return physicalDevices;
    }

    inline static TL::Vector<VkQueueFamilyProperties> GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice)
    {
        uint32_t queueFamilyPropertiesCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
        TL::Vector<VkQueueFamilyProperties> queueFamilyProperties{};
        queueFamilyProperties.resize(queueFamilyPropertiesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());
        return queueFamilyProperties;
    }

    IContext::IContext(Ptr<DebugCallbacks> debugCallbacks)
        : Context(std::move(debugCallbacks))
        , m_instance(VK_NULL_HANDLE)
        , m_physicalDevice(VK_NULL_HANDLE)
        , m_device(VK_NULL_HANDLE)
        , m_allocator(VK_NULL_HANDLE)
        , m_presentQueue(VK_NULL_HANDLE)
        , m_graphicsQueue(VK_NULL_HANDLE)
        , m_computeQueue(VK_NULL_HANDLE)
        , m_transferQueue(VK_NULL_HANDLE)
        , m_graphicsQueueFamilyIndex(UINT32_MAX)
        , m_computeQueueFamilyIndex(UINT32_MAX)
        , m_transferQueueFamilyIndex(UINT32_MAX)
        , m_fnTable(CreatePtr<FunctionsTable>())
        , m_bindGroupAllocator(CreatePtr<BindGroupAllocator>(this))
        , m_commandPool(CreatePtr<ICommandPool>(this))
        , m_deleteQueue(CreatePtr<DeleteQueue>(this))
        , m_imageOwner()
        , m_bufferOwner()
        , m_imageViewOwner()
        , m_bufferViewOwner()
        , m_bindGroupLayoutsOwner()
        , m_bindGroupOwner()
        , m_pipelineLayoutOwner()
        , m_graphicsPipelineOwner()
        , m_computePipelineOwner()
        , m_samplerOwner()
    {
    }

    template<typename T>
    inline static constexpr void LeakReportBuilder(TL::String& reportBody, const char* message, const HandlePool<T>& pool)
    {
        (void)reportBody;
        (void)message;
        (void)pool;
#if RHI_REPORT_RESOURCE_LEAKS
    #if RHI_REPORT_RESOURCE_LEAKS_COUNT
        reportBody += TL::String(std::format(message, pool.ReportLiveResourcesCount()));
    #else
        reportBody += TL::String(std::format(message, pool.ReportLiveResources()));
    #endif
#endif
    }

    IContext::~IContext()
    {
        ZoneScoped;

        vkDeviceWaitIdle(m_device);

        {
            TL::String leakReport = "";
            LeakReportBuilder(leakReport, "Leaked ({}) Images \n", m_imageOwner);
            LeakReportBuilder(leakReport, "Leaked ({}) Buffers \n", m_bufferOwner);
            LeakReportBuilder(leakReport, "Leaked ({}) ImageViews \n", m_imageViewOwner);
            LeakReportBuilder(leakReport, "Leaked ({}) BufferViews \n", m_bufferViewOwner);
            LeakReportBuilder(leakReport, "Leaked ({}) BindGroupLayouts \n", m_bindGroupLayoutsOwner);
            LeakReportBuilder(leakReport, "Leaked ({}) BindGroups \n", m_bindGroupOwner);
            LeakReportBuilder(leakReport, "Leaked ({}) PipelineLayouts \n", m_pipelineLayoutOwner);
            LeakReportBuilder(leakReport, "Leaked ({}) GraphicsPipelines \n", m_graphicsPipelineOwner);
            LeakReportBuilder(leakReport, "Leaked ({}) ComputePipelines \n", m_computePipelineOwner);
            LeakReportBuilder(leakReport, "Leaked ({}) Samplers \n", m_samplerOwner);
        }

        Shutdown();

        // m_commandPool->Shutdown();
        // m_bindGroupAllocator->Shutdown();

        vmaDestroyAllocator(m_allocator);
        vkDestroyDevice(m_device, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }

    ResultCode IContext::Init(const ApplicationInfo& appInfo)
    {
        ZoneScoped;

        bool debugExtensionEnabled = false;

        TryValidateVk(InitInstance(appInfo, &debugExtensionEnabled));
        TryValidateVk(InitDevice());
        TryValidateVk(InitMemoryAllocator());

        vkGetDeviceQueue(m_device, m_graphicsQueueFamilyIndex, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, m_computeQueueFamilyIndex, 0, &m_computeQueue);
        vkGetDeviceQueue(m_device, m_transferQueueFamilyIndex, 0, &m_transferQueue);

        m_fnTable->Init(this, debugExtensionEnabled);
        TryValidate(m_bindGroupAllocator->Init());
        TryValidate(m_commandPool->Init(CommandPoolFlags::Transient));

        return ResultCode::Success;
    }

    void IContext::SetDebugName(VkDebugReportObjectTypeEXT type, uint64_t handle, const char* name) const
    {
        if (m_fnTable->m_debugMarkerSetObjectNameEXT && name != nullptr)
        {
            VkDebugMarkerObjectNameInfoEXT nameInfo{};
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
            nameInfo.pNext = nullptr;
            nameInfo.pObjectName = name;
            nameInfo.object = handle;
            nameInfo.objectType = type;
            m_fnTable->m_debugMarkerSetObjectNameEXT(m_device, &nameInfo);
        }
    }

    VkSemaphore IContext::CreateSemaphore(const char* name, bool timeline, uint64_t initialValue)
    {
        VkSemaphoreTypeCreateInfo timelineInfo{};
        timelineInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        timelineInfo.initialValue = initialValue;
        timelineInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = timeline ? &timelineInfo : nullptr;
        createInfo.flags = 0;
        VkSemaphore semaphore = VK_NULL_HANDLE;
        Validate(vkCreateSemaphore(m_device, &createInfo, nullptr, &semaphore));
        SetDebugName(semaphore, name);
        return semaphore;
    }

    void IContext::DestroySemaphore(VkSemaphore semaphore)
    {
        if (semaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device, semaphore, nullptr);
        }
    }

    uint32_t IContext::GetMemoryTypeIndex(MemoryType memoryType)
    {
        VkMemoryPropertyFlags flags = 0;
        VkMemoryPropertyFlags negateFlags = 0;
        switch (memoryType)
        {
        case MemoryType::CPU: flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; break;
        case MemoryType::GPULocal:
            flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            negateFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
        case MemoryType::GPUShared: flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; break;
        }

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

        // TODO: if multiple memory types with the desired flags are present,
        // then select the based on size, performance charactersitcs ...
        uint32_t index = UINT32_MAX;
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        {
            VkMemoryType type = memoryProperties.memoryTypes[i];

            if ((type.propertyFlags & flags) == flags && (type.propertyFlags & negateFlags) == 0)
            {
                index = type.heapIndex;
            }
        }

        return index;
    }

    uint32_t IContext::GetQueueFamilyIndex(QueueType queueType)
    {
        (void)queueType;
        return m_graphicsQueueFamilyIndex;

        // switch (queueType)
        // {
        // case QueueType::Graphics: return m_graphicsQueueFamilyIndex;
        // case QueueType::Compute:  return m_computeQueueFamilyIndex;
        // case QueueType::Transfer: return m_transferQueueFamilyIndex;
        // default:                  RHI_UNREACHABLE(); return UINT32_MAX;
        // }
    }

    VkQueue IContext::GetQueue(QueueType queueType)
    {
        (void)queueType;
        return m_graphicsQueue;

        // switch (queueType)
        // {
        // case QueueType::Graphics: return m_graphicsQueue;
        // case QueueType::Compute:  return m_computeQueue;
        // case QueueType::Transfer: return m_transferQueue;
        // default:                  RHI_UNREACHABLE(); return VK_NULL_HANDLE;
        // }
    }

    void IContext::QueueSubmit(QueueType queueType,
                               TL::Span<const ICommandList* const> commandLists,
                               TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2> waitSemaphores,
                               TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2> signalSemaphores,
                               IFence* signalFence)
    {
        ZoneScoped;

        TL::Vector<VkSemaphoreSubmitInfo> waitSemaphoreSubmitInfos;
        TL::Vector<VkSemaphoreSubmitInfo> signalSemaphoreSubmitInfos;
        TL::Vector<VkCommandBufferSubmitInfo> commandBufferSubmitInfos;

        for (auto commandList : commandLists)
        {
            VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
            commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            commandBufferSubmitInfo.commandBuffer = commandList->m_commandBuffer;
            commandBufferSubmitInfos.push_back(commandBufferSubmitInfo);
        }

        for (auto waitSemaphore : waitSemaphores)
        {
            VkSemaphoreSubmitInfo waitSemaphoreSubmitInfo{};
            waitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            waitSemaphoreSubmitInfo.semaphore = waitSemaphore.first;
            waitSemaphoreSubmitInfo.stageMask = waitSemaphore.second;
            waitSemaphoreSubmitInfos.push_back(waitSemaphoreSubmitInfo);
        }

        for (auto signalSemaphore : signalSemaphores)
        {
            VkSemaphoreSubmitInfo signalSemaphoreSubmitInfo{};
            signalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            signalSemaphoreSubmitInfo.semaphore = signalSemaphore.first;
            signalSemaphoreSubmitInfo.stageMask = signalSemaphore.second;
            signalSemaphoreSubmitInfos.push_back(signalSemaphoreSubmitInfo);
        }

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreInfoCount = (uint32_t)waitSemaphoreSubmitInfos.size();
        submitInfo.pWaitSemaphoreInfos = waitSemaphoreSubmitInfos.data();
        submitInfo.commandBufferInfoCount = (uint32_t)commandBufferSubmitInfos.size();
        submitInfo.pCommandBufferInfos = commandBufferSubmitInfos.data();
        submitInfo.signalSemaphoreInfoCount = (uint32_t)signalSemaphoreSubmitInfos.size();
        submitInfo.pSignalSemaphoreInfos = signalSemaphoreSubmitInfos.data();
        vkQueueSubmit2(GetQueue(queueType), 1, &submitInfo, signalFence ? signalFence->UseFence() : VK_NULL_HANDLE);
    }

    ////////////////////////////////////////////////////////////
    // Interface implementation
    ////////////////////////////////////////////////////////////
    Ptr<Swapchain> IContext::Internal_CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        auto swapchain = CreatePtr<ISwapchain>(this);
        auto result = swapchain->Init(createInfo);
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create swapchain object");
        }
        return swapchain;
    }

    Ptr<ShaderModule> IContext::Internal_CreateShaderModule(TL::Span<const uint32_t> shaderBlob)
    {
        auto shaderModule = CreatePtr<IShaderModule>(this);
        auto result = shaderModule->Init(shaderBlob);
        if (result != ResultCode::Success)
        {
            DebugLogError("Failed to create shader module");
        }
        return shaderModule;
    }

    Ptr<Fence> IContext::Internal_CreateFence()
    {
        auto fence = CreatePtr<IFence>(this);
        auto result = fence->Init();
        if (result != ResultCode::Success)
        {
            DebugLogError("Failed to create a fence object");
        }
        return fence;
    }

    Ptr<CommandPool> IContext::Internal_CreateCommandPool(CommandPoolFlags flags)
    {
        auto commandPool = CreatePtr<ICommandPool>(this);
        auto result = commandPool->Init(flags);
        if (result != ResultCode::Success)
        {
            DebugLogError("Failed to create a command_list_allocator object");
        }
        return commandPool;
    }

    Handle<BindGroupLayout> IContext::Internal_CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        IBindGroupLayout bindGroupLayout{};
        auto result = bindGroupLayout.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create bindGroupLayout");
        }
        return m_bindGroupLayoutsOwner.Emplace(std::move(bindGroupLayout));
    }

    void IContext::Internal_DestroyBindGroupLayout(Handle<BindGroupLayout> handle)
    {
        m_deleteQueue->Destroy(GetCurrentFrameIndex(), [this, handle]()
        {
            auto bindGroupLayout = m_bindGroupLayoutsOwner.Get(handle);
            bindGroupLayout->Shutdown(this);
        });
    }

    Handle<BindGroup> IContext::Internal_CreateBindGroup(Handle<BindGroupLayout> layoutHandle, uint32_t bindlessElementsCount)
    {
        IBindGroup bindGroup{};
        auto result = bindGroup.Init(this, layoutHandle, bindlessElementsCount);
        if (IsError(result))
        {
            DebugLogError("Failed to create bindGroup");
        }
        auto handle = m_bindGroupOwner.Emplace(std::move(bindGroup));
        return handle;
    }

    void IContext::Internal_DestroyBindGroup(Handle<BindGroup> handle)
    {
        m_deleteQueue->Destroy(GetCurrentFrameIndex(), [this, handle]()
        {
            auto bindGroup = m_bindGroupOwner.Get(handle);
            bindGroup->Shutdown(this);
        });
    }

    void IContext::Internal_UpdateBindGroup(Handle<BindGroup> handle, TL::Span<const ResourceBinding> bindings)
    {
        auto bindGroup = m_bindGroupOwner.Get(handle);
        bindGroup->Write(this, bindings);
    }

    Handle<PipelineLayout> IContext::Internal_CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        IPipelineLayout pipelineLayout{};
        auto result = pipelineLayout.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create pipelineLayout");
        }
        auto handle = m_pipelineLayoutOwner.Emplace(std::move(pipelineLayout));
        return handle;
    }

    void IContext::Internal_DestroyPipelineLayout(Handle<PipelineLayout> handle)
    {
        auto pipelineLayout = m_pipelineLayoutOwner.Get(handle);
        pipelineLayout->Shutdown(this);
    }

    Handle<GraphicsPipeline> IContext::Internal_CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        IGraphicsPipeline graphicsPipeline{};
        auto result = graphicsPipeline.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create graphicsPipeline");
        }
        auto handle = m_graphicsPipelineOwner.Emplace(std::move(graphicsPipeline));
        return handle;
    }

    void IContext::Internal_DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)
    {
        m_deleteQueue->Destroy(GetCurrentFrameIndex(), [this, handle]()
        {
            auto graphicsPipeline = m_graphicsPipelineOwner.Get(handle);
            graphicsPipeline->Shutdown(this);
        });
    }

    Handle<ComputePipeline> IContext::Internal_CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        IComputePipeline computePipeline{};
        auto result = computePipeline.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create computePipeline");
        }
        auto handle = m_computePipelineOwner.Emplace(std::move(computePipeline));
        return handle;
    }

    void IContext::Internal_DestroyComputePipeline(Handle<ComputePipeline> handle)
    {
        m_deleteQueue->Destroy(GetCurrentFrameIndex(), [this, handle]()
        {
            auto computePipeline = m_computePipelineOwner.Get(handle);
            computePipeline->Shutdown(this);
        });
    }

    Handle<Sampler> IContext::Internal_CreateSampler(const SamplerCreateInfo& createInfo)
    {
        ISampler sampler{};
        auto result = sampler.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create sampler");
        }
        auto handle = m_samplerOwner.Emplace(std::move(sampler));
        return handle;
    }

    void IContext::Internal_DestroySampler(Handle<Sampler> handle)
    {
        m_deleteQueue->Destroy(GetCurrentFrameIndex(), [this, handle]()
        {
            auto sampler = m_samplerOwner.Get(handle);
            sampler->Shutdown(this);
        });
    }

    Result<Handle<Image>> IContext::Internal_CreateImage(const ImageCreateInfo& createInfo)
    {
        IImage image{};
        auto result = image.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create image");
            return result;
        }
        auto handle = m_imageOwner.Emplace(std::move(image));
        return Result<Handle<Image>>(handle);
    }

    void IContext::Internal_DestroyImage(Handle<Image> handle)
    {
        m_deleteQueue->Destroy(GetCurrentFrameIndex(), [this, handle]()
        {
            auto image = m_imageOwner.Get(handle);
            image->Shutdown(this);
        });
    }

    Result<Handle<Buffer>> IContext::Internal_CreateBuffer(const BufferCreateInfo& createInfo)
    {
        IBuffer buffer{};
        auto result = buffer.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create buffer");
            return result;
        }
        auto handle = m_bufferOwner.Emplace(std::move(buffer));
        return Result<Handle<Buffer>>(handle);
    }

    void IContext::Internal_DestroyBuffer(Handle<Buffer> handle)
    {
        m_deleteQueue->Destroy(GetCurrentFrameIndex(), [this, handle]()
        {
            auto buffer = m_bufferOwner.Get(handle);
            buffer->Shutdown(this);
        });
    }

    Handle<ImageView> IContext::Internal_CreateImageView(const ImageViewCreateInfo& createInfo)
    {
        IImageView imageView{};
        auto result = imageView.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create image view");
        }
        auto handle = m_imageViewOwner.Emplace(std::move(imageView));
        return handle;
    }

    void IContext::Internal_DestroyImageView(Handle<ImageView> handle)
    {
        m_deleteQueue->Destroy(GetCurrentFrameIndex(), [this, handle]()
        {
            auto imageView = m_imageViewOwner.Get(handle);
            imageView->Shutdown(this);
        });
    }

    Handle<BufferView> IContext::Internal_CreateBufferView(const BufferViewCreateInfo& createInfo)
    {
        IBufferView bufferView{};
        auto result = bufferView.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create buffer view");
        }
        auto handle = m_bufferViewOwner.Emplace(std::move(bufferView));
        return handle;
    }

    void IContext::Internal_DestroyBufferView(Handle<BufferView> handle)
    {
        m_deleteQueue->Destroy(GetCurrentFrameIndex(), [this, handle]()
        {
            auto imageView = m_bufferViewOwner.Get(handle);
            imageView->Shutdown(this);
        });
    }

    void IContext::Internal_DispatchGraph(RenderGraph& renderGraph, Fence* signalFence)
    {
        for (auto passHandle : renderGraph.m_passes)
        {
            auto pass = renderGraph.m_passOwner.Get(passHandle);
            RenderGraphCompiler::CompilePass(this, renderGraph, pass);
            auto submitData = (IPassSubmitData*)pass->submitData;
            TL::Span commandLists{ (const ICommandList**)pass->commandList.data(), pass->commandList.size() };
            QueueSubmit(pass->queueType, commandLists, submitData->waitSemaphores, submitData->signalSemaphores, (IFence*)signalFence);
            submitData->Clear();
        }
    }

    DeviceMemoryPtr IContext::Internal_MapBuffer(Handle<Buffer> handle)
    {
        auto resource = m_bufferOwner.Get(handle);
        auto allocation = resource->allocation.handle;

        DeviceMemoryPtr memoryPtr = nullptr;
        Validate(vmaMapMemory(m_allocator, allocation, &memoryPtr));
        return memoryPtr;
    }

    void IContext::Internal_UnmapBuffer(Handle<Buffer> handle)
    {
        auto resource = m_bufferOwner.Get(handle)->allocation.handle;
        vmaUnmapMemory(m_allocator, resource);
    }

    void IContext ::Internal_StageResourceWrite(Handle<Image> imageHandle, ImageSubresourceLayers subresources, Handle<Buffer> buffer, size_t bufferOffset)
    {
        auto image = m_imageOwner.Get(imageHandle);
        image->waitSemaphore = CreateSemaphore("ImageWriteSemaphore");

        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.pNext = nullptr;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_NONE;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image->handle;
        barrier.subresourceRange.aspectMask = FormatToAspect(image->format);
        barrier.subresourceRange.baseMipLevel = subresources.mipLevel;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = subresources.arrayBase;
        barrier.subresourceRange.layerCount = subresources.arrayCount;

        BufferImageCopyInfo copyInfo{};
        copyInfo.image = imageHandle;
        copyInfo.subresource = subresources;
        copyInfo.buffer = buffer;
        copyInfo.bufferOffset = bufferOffset;
        copyInfo.imageSize.width = image->extent.width;
        copyInfo.imageSize.height = image->extent.height;
        copyInfo.imageSize.depth = image->extent.depth;
        auto commandList = (ICommandList*)m_commandPool->Allocate(QueueType::Transfer, CommandListLevel::Primary, 1).front();

        commandList->Begin();
        commandList->PipelineBarrier({}, {}, barrier);
        commandList->CopyBufferToImage(copyInfo);

        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_NONE;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // TODO: deduce correct layout from the image usage
        commandList->PipelineBarrier({}, {}, barrier);
        commandList->End();

        QueueSubmit(QueueType::Transfer, commandList, {}, { { image->waitSemaphore, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT } });
        m_commandPool->Release(commandList);
    }

    void IContext ::Internal_StageResourceWrite(Handle<Buffer> bufferHandle, size_t offset, size_t size, Handle<Buffer> srcBuffer, size_t srcOffset)
    {
        auto buffer = m_bufferOwner.Get(bufferHandle);
        auto semaphore = buffer->waitSemaphore = CreateSemaphore("BufferWriteSemaphore");
        DestroySemaphore(semaphore);

        BufferCopyInfo copyInfo{};
        copyInfo.dstBuffer = bufferHandle;
        copyInfo.dstOffset = offset;
        copyInfo.srcBuffer = srcBuffer;
        copyInfo.srcOffset = srcOffset;
        copyInfo.size = size;
        auto commandList = (ICommandList*)m_commandPool->Allocate(QueueType::Transfer, CommandListLevel::Primary, 1).front();
        commandList->Begin();
        commandList->CopyBuffer(copyInfo);
        commandList->End();

        QueueSubmit(QueueType::Transfer, commandList, {}, { { buffer->waitSemaphore, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT } });
        m_commandPool->Release(commandList);
    }

    void IContext ::Internal_StageResourceRead(Handle<Image> image, ImageSubresourceLayers subresources, Handle<Buffer> buffer, size_t bufferOffset, Fence* fence)
    {
        (void)image;
        (void)subresources;
        (void)buffer;
        (void)bufferOffset;
        (void)fence;
        RHI_UNREACHABLE();
    }

    void IContext ::Internal_StageResourceRead(Handle<Buffer> buffer, size_t offset, size_t size, Handle<Buffer> srcBuffer, size_t srcOffset, Fence* fence)
    {
        (void)buffer;
        (void)offset;
        (void)size;
        (void)srcBuffer;
        (void)srcOffset;
        (void)fence;
        RHI_UNREACHABLE();
    }

    ////////////////////////////////////////////////////////////
    // Interface implementation
    ////////////////////////////////////////////////////////////

    VkBool32 IContext::DebugMessengerCallbacks(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                               VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                               const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                               void* pUserData)
    {
        (void)messageTypes;
        auto context = (IContext*)pUserData;

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            context->DebugLogError(pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            context->DebugLogWarn(pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            context->DebugLogInfo(pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            context->DebugLogInfo(pCallbackData->pMessage);
        }

        return VK_FALSE;
    }

    VkResult IContext::InitInstance(const ApplicationInfo& appInfo, bool* debugExtensionEnabled)
    {
        TL::Vector<const char*> enabledLayersNames = {
            "VK_LAYER_KHRONOS_validation",
        };

        TL::Vector<const char*> enabledExtensionsNames = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VULKAN_SURFACE_OS_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        };

        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pNext = nullptr;
        applicationInfo.pApplicationName = appInfo.applicationName;
        applicationInfo.applicationVersion = VK_MAKE_API_VERSION(0, appInfo.applicationVersion.major, appInfo.applicationVersion.minor, appInfo.applicationVersion.patch);
        applicationInfo.pEngineName = appInfo.engineName;
        applicationInfo.engineVersion = VK_MAKE_API_VERSION(0, appInfo.engineVersion.major, appInfo.engineVersion.minor, appInfo.engineVersion.patch);
        applicationInfo.apiVersion = VK_API_VERSION_1_3;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugMessengerCallbacks;
        debugCreateInfo.pUserData = this;

#if RHI_DEBUG
        for (VkExtensionProperties extension : GetAvailableInstanceExtensions())
        {
            auto extensionName = extension.extensionName;
            if (!strcmp(extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
            {
                *debugExtensionEnabled = true;
                continue;
            }
        }

        if (*debugExtensionEnabled)
        {
            enabledExtensionsNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            enabledExtensionsNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
        else
        {
            DebugLogWarn("RHI Vulkan: Debug extension not present.\n Vulkan layer validation is disabled.");
        }
#endif

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = *debugExtensionEnabled ? &debugCreateInfo : nullptr;
        createInfo.flags = {};
        createInfo.pApplicationInfo = &applicationInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayersNames.size());
        createInfo.ppEnabledLayerNames = enabledLayersNames.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensionsNames.size());
        createInfo.ppEnabledExtensionNames = enabledExtensionsNames.data();
        return vkCreateInstance(&createInfo, nullptr, &m_instance);
    }

    VkResult IContext::InitDevice()
    {
        for (VkPhysicalDevice physicalDevice : GetAvailablePhysicalDevices(m_instance))
        {
            bool swapchainExtension = false;
            bool dynamicRenderingExtension = false;
            bool maintenance2Extension = false;
            bool multiviewExtension = false;
            bool createRenderpass2Extension = false;
            bool depthStencilResolveExtension = false;

            for (auto extension : GetAvailableDeviceExtensions(physicalDevice))
            {
                swapchainExtension |= strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0;
                dynamicRenderingExtension |= strcmp(extension.extensionName, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0;
                maintenance2Extension |= strcmp(extension.extensionName, VK_KHR_MAINTENANCE2_EXTENSION_NAME) == 0;
                multiviewExtension |= strcmp(extension.extensionName, VK_KHR_MULTIVIEW_EXTENSION_NAME) == 0;
                createRenderpass2Extension |= strcmp(extension.extensionName, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME) == 0;
                depthStencilResolveExtension |= strcmp(extension.extensionName, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME) == 0;
            }

            if (swapchainExtension && dynamicRenderingExtension && maintenance2Extension && multiviewExtension && createRenderpass2Extension && depthStencilResolveExtension)
            {
                m_physicalDevice = physicalDevice;
                break;
            }
        }

        TL::Vector<const char*> deviceLayerNames = {

        };

        TL::Vector<const char*> deviceExtensionNames = {
#if RHI_DEBUG
            VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
#endif
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME
        };

        auto queueFamilyProperties = GetPhysicalDeviceQueueFamilyProperties(m_physicalDevice);
        for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyProperties.size(); queueFamilyIndex++)
        {
            auto queueFamilyProperty = queueFamilyProperties[queueFamilyIndex];

            // Search for main queue that should be able to do all work (graphics, compute and transfer)
            if ((queueFamilyProperty.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) == (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
            {
                m_graphicsQueueFamilyIndex = queueFamilyIndex;

                m_transferQueueFamilyIndex = queueFamilyIndex;
                m_computeQueueFamilyIndex = queueFamilyIndex;
                break;
            }

            // Search for transfer queue
            if ((queueFamilyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT && (queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)
            {
                m_transferQueueFamilyIndex = queueFamilyIndex;
            }

            // Search for transfer queue
            if ((queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT && (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
            {
                m_computeQueueFamilyIndex = queueFamilyIndex;
            }
        }

        float queuePriority = 1.0f;
        TL::Vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = nullptr;
        queueCreateInfo.flags = 0;

        if (m_graphicsQueueFamilyIndex != UINT32_MAX)
        {
            queueCreateInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        else if (m_computeQueueFamilyIndex != UINT32_MAX)
        {
            queueCreateInfo.queueFamilyIndex = m_computeQueueFamilyIndex;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        else if (m_transferQueueFamilyIndex != UINT32_MAX)
        {
            queueCreateInfo.queueFamilyIndex = m_transferQueueFamilyIndex;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        else
        {
            RHI_UNREACHABLE();
        }

        VkPhysicalDeviceFeatures enabledFeatures{};
        enabledFeatures.samplerAnisotropy = VK_TRUE;

        VkPhysicalDeviceSynchronization2Features syncFeature{};
        syncFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        syncFeature.synchronization2 = VK_TRUE;

        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
        dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamicRenderingFeatures.pNext = &syncFeature;
        dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &dynamicRenderingFeatures;
        createInfo.flags = 0;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(deviceLayerNames.size());
        createInfo.ppEnabledLayerNames = deviceLayerNames.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionNames.size());
        createInfo.ppEnabledExtensionNames = deviceExtensionNames.data();
        createInfo.pEnabledFeatures = &enabledFeatures;
        auto result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);
        vkGetDeviceQueue(m_device, m_graphicsQueueFamilyIndex, 0, &m_presentQueue);

        m_limits->stagingMemoryLimit = 256 * 1000 * 1000;

        return result;
    }

    VkResult IContext::InitMemoryAllocator()
    {
        VmaAllocatorCreateInfo createInfo{};
        createInfo.physicalDevice = m_physicalDevice;
        createInfo.device = m_device;
        createInfo.instance = m_instance;
        createInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        return vmaCreateAllocator(&createInfo, &m_allocator);
    }

} // namespace RHI::Vulkan