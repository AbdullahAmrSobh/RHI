#include "RHI-Vulkan/Loader.hpp"

#include "Common.hpp"
#include "Resources.hpp"
#include "CommandList.hpp"
#include "CommandPool.hpp"
#include "Swapchain.hpp"
#include "RenderGraphCompiler.hpp"
#include "Context.hpp"

#include <tracy/Tracy.hpp>

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
        RHI_ASSERT(result == VK_SUCCESS);
        return std::move(context);
    }
} // namespace RHI

namespace RHI::Vulkan
{
    // inline static TL::Vector<VkLayerProperties> GetAvailableInstanceLayerExtensions()
    // {
    //     uint32_t instanceLayerCount;
    //     vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
    //     TL::Vector<VkLayerProperties> layers;
    //     layers.resize(instanceLayerCount);
    //     vkEnumerateInstanceLayerProperties(&instanceLayerCount, layers.data());
    //     return layers;
    // }

    inline static TL::Vector<VkExtensionProperties> GetAvailableInstanceExtensions()
    {
        uint32_t instanceExtensionsCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, nullptr);
        TL::Vector<VkExtensionProperties> extensions;
        extensions.resize(instanceExtensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, extensions.data());
        return extensions;
    }

    // inline static TL::Vector<VkLayerProperties> GetAvailableDeviceLayerExtensions(VkPhysicalDevice physicalDevice)
    // {
    //     uint32_t instanceLayerCount;
    //     vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, nullptr);
    //     TL::Vector<VkLayerProperties> layers;
    //     layers.resize(instanceLayerCount);
    //     vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, layers.data());
    //     return layers;
    // }

    inline static TL::Vector<VkExtensionProperties> GetAvailableDeviceExtensions(VkPhysicalDevice physicalDevice)
    {
        uint32_t extensionsCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr);
        TL::Vector<VkExtensionProperties> extnesions;
        extnesions.resize(extensionsCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, extnesions.data());
        return extnesions;
    }

    inline static TL::Vector<VkPhysicalDevice> GetAvailablePhysicalDevices(VkInstance instance)
    {
        uint32_t physicalDeviceCount;
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
        TL::Vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
        VkResult result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
        RHI_ASSERT(result == VK_SUCCESS);
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
    {
    }

    VkResult IContext::Init(const ApplicationInfo& appInfo)
    {
        ZoneScoped;

        bool debugExtensionEnabled = false;

        VkResult result = VK_ERROR_UNKNOWN;

        result = InitInstance(appInfo, &debugExtensionEnabled);
        VULKAN_RETURN_VKERR_CODE(result);

        result = InitDevice();
        VULKAN_RETURN_VKERR_CODE(result);

        result = InitMemoryAllocator();
        VULKAN_RETURN_VKERR_CODE(result);

        vkGetDeviceQueue(m_device, m_graphicsQueueFamilyIndex, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, m_computeQueueFamilyIndex, 0, &m_computeQueue);
        vkGetDeviceQueue(m_device, m_transferQueueFamilyIndex, 0, &m_transferQueue);

        result = LoadFunctions(debugExtensionEnabled);
        VULKAN_RETURN_VKERR_CODE(result);

        m_bindGroupAllocator = CreatePtr<BindGroupAllocator>(m_device);

        // TracyVkContextHostCalibrated(m_physicalDevice, m_device, vkResetQueryPool, m_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR, m_vkGetCalibratedTimestampsKHR);

        m_commandPool = CreatePtr<ICommandPool>(this);
        m_commandPool->Init();

        return VK_SUCCESS;
    }

    void IContext::SetDebugName(VkDebugReportObjectTypeEXT type, uint64_t handle, const char* name) const
    {
        if (m_vkDebugMarkerSetObjectNameEXT && name != nullptr)
        {
            VkDebugMarkerObjectNameInfoEXT nameInfo{};
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
            nameInfo.pNext = nullptr;
            nameInfo.pObjectName = name;
            nameInfo.object = handle;
            nameInfo.objectType = type;
            m_vkDebugMarkerSetObjectNameEXT(m_device, &nameInfo);
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
        auto result = vkCreateSemaphore(m_device, &createInfo, nullptr, &semaphore);
        VULKAN_ASSERT_SUCCESS(result);
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
    void IContext::Internal_OnShutdown()
    {
        vkDeviceWaitIdle(m_device);

        // TracyVkDestroy(m_tracyContext);

        m_bindGroupAllocator->Shutdown();

        vmaDestroyAllocator(m_allocator);
        vkDestroyDevice(m_device, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }

    void IContext::Internal_OnCollectResources()
    {
        for (auto& destroyResource : m_resourceDestroyQueue)
        {
            destroyResource();
        }
        m_resourceDestroyQueue.clear();
    }

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

    Ptr<ShaderModule> IContext::Internal_CreateShaderModule(TL::Span<const uint8_t> shaderBlob)
    {
        auto shaderModule = CreatePtr<IShaderModule>(this);
        auto result = shaderModule->Init(shaderBlob);
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create shader module");
        }
        return shaderModule;
    }

    Ptr<Fence> IContext::Internal_CreateFence()
    {
        auto fence = CreatePtr<IFence>(this);
        auto result = fence->Init();
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create a fence object");
        }
        return fence;
    }

    Ptr<CommandPool> IContext::Internal_CreateCommandPool()
    {
        auto commandPool = CreatePtr<ICommandPool>(this);
        auto result = commandPool->Init();
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create a command_list_allocator object");
        }
        return commandPool;
    }

    Ptr<ResourcePool> IContext::Internal_CreateResourcePool(const ResourcePoolCreateInfo& createInfo)
    {
        auto resourcePool = CreatePtr<IResourcePool>(this);
        auto result = resourcePool->Init(createInfo);
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create a resource_pool object");
        }
        return resourcePool;
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
        auto bindGroupLayout = m_bindGroupLayoutsOwner.Get(handle);
        // clang-format off
        IContext* self = this;
        m_resourceDestroyQueue.push_back([=](){ bindGroupLayout->Shutdown(self); });
        // clang-format on
    }

    Handle<BindGroup> IContext::Internal_CreateBindGroup(Handle<BindGroupLayout> layoutHandle)
    {
        IBindGroup bindGroup{};
        auto result = bindGroup.Init(this, layoutHandle);
        if (IsError(result))
        {
            DebugLogError("Failed to create bindGroup");
        }
        auto handle = m_bindGroupOwner.Emplace(std::move(bindGroup));
        return handle;
    }

    void IContext::Internal_DestroyBindGroup(Handle<BindGroup> handle)
    {
        auto bindGroup = m_bindGroupOwner.Get(handle);
        // clang-format off
        IContext* self = this;
        m_resourceDestroyQueue.push_back([=](){ bindGroup->Shutdown(self); });
        // clang-format on
    }

    void IContext::Internal_UpdateBindGroup(Handle<BindGroup> handle, const BindGroupData& data)
    {
        auto bindGroup = m_bindGroupOwner.Get(handle);
        bindGroup->Write(this, data);
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
        // clang-format off
        IContext* self = this;
        m_resourceDestroyQueue.push_back([=](){ pipelineLayout->Shutdown(self); });
        // clang-format on
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
        auto graphicsPipeline = m_graphicsPipelineOwner.Get(handle);
        // clang-format off
        IContext* self = this;
        m_resourceDestroyQueue.push_back([=](){ graphicsPipeline->Shutdown(self); });
        // clang-format on
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
        auto computePipeline = m_computePipelineOwner.Get(handle);
        // clang-format off
        IContext* self = this;
        m_resourceDestroyQueue.push_back([=](){ computePipeline->Shutdown(self); });
        // clang-format on
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
        auto sampler = m_samplerOwner.Get(handle);
        // clang-format off
        IContext* self = this;
        m_resourceDestroyQueue.push_back([=](){ sampler->Shutdown(self); });
        // clang-format on
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
        auto image = m_imageOwner.Get(handle);
        // clang-format off
        IContext* self = this;
        m_resourceDestroyQueue.push_back([=](){ image->Shutdown(self); });
        // clang-format on
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
        auto buffer = m_bufferOwner.Get(handle);
        // clang-format off
        IContext* self = this;
        m_resourceDestroyQueue.push_back([=](){ buffer->Shutdown(self); });
        // clang-format on
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
        auto imageView = m_imageViewOwner.Get(handle);
        // clang-format off
        IContext* self = this;
        m_resourceDestroyQueue.push_back([=](){ imageView->Shutdown(self); });
        // clang-format on
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
        auto imageView = m_bufferViewOwner.Get(handle);
        // clang-format off
        IContext* self = this;
        m_resourceDestroyQueue.push_back([=](){ imageView->Shutdown(self); });
        // clang-format on
    }

    void IContext::Internal_DispatchGraph(RenderGraph& renderGraph, Fence* signalFence)
    {
        for (auto passHandle : renderGraph.m_passes)
        {
            auto pass = renderGraph.m_passOwner.Get(passHandle);
            RenderGraphCompiler::CompilePass(this, renderGraph, pass);
            auto submitData = (IPassSubmitData*)pass->submitData;
            TL::Span commandLists { (const ICommandList**)pass->commandList.data(), pass->commandList.size() };
            QueueSubmit(pass->queueType, commandLists, submitData->waitSemaphores, submitData->signalSemaphores, (IFence*)signalFence);
            submitData->Clear();
        }

    }

    DeviceMemoryPtr IContext::Internal_MapBuffer(Handle<Buffer> handle)
    {
        auto resource = m_bufferOwner.Get(handle);
        auto allocation = resource->allocation.handle;

        DeviceMemoryPtr memoryPtr = nullptr;
        VkResult result = vmaMapMemory(m_allocator, allocation, &memoryPtr);
        VULKAN_ASSERT_SUCCESS(result);
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

        BufferToImageCopyInfo copyInfo{};
        copyInfo.dstImage = imageHandle;
        copyInfo.dstSubresource = subresources;
        copyInfo.srcBuffer = buffer;
        copyInfo.srcOffset = bufferOffset;
        copyInfo.dstSize   = image->size;
        auto commandList = (ICommandList*)m_commandPool->Allocate(QueueType::Transfer);

        commandList->Begin();
        commandList->PipelineBarrier({}, {}, barrier);
        commandList->Copy(copyInfo);

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
        buffer->waitSemaphore = CreateSemaphore("BufferWriteSemaphore");

        BufferCopyInfo copyInfo{};
        copyInfo.dstBuffer = bufferHandle;
        copyInfo.dstOffset = offset;
        copyInfo.srcBuffer = srcBuffer;
        copyInfo.srcOffset = srcOffset;
        copyInfo.size = size;
        auto commandList = (ICommandList*)m_commandPool->Allocate(QueueType::Transfer);
        commandList->Begin();
        commandList->Copy(copyInfo);
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

        if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            context->DebugLogError(pCallbackData->pMessage);
        }
        else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            context->DebugLogWarn(pCallbackData->pMessage);
        }
        else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            context->DebugLogInfo(pCallbackData->pMessage);
        }
        else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
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

        // m_limits->stagingMemoryLimit = 256 * 1000 * 1000;

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

    VkResult IContext::LoadFunctions(bool debugExtensionEnabled)
    {
#if RHI_DEBUG
        if (debugExtensionEnabled)
        {
            m_vkCmdDebugMarkerBeginEXT = VULKAN_LOAD_PROC(m_device, vkCmdDebugMarkerBeginEXT);
            m_vkCmdDebugMarkerInsertEXT = VULKAN_LOAD_PROC(m_device, vkCmdDebugMarkerInsertEXT);
            m_vkCmdDebugMarkerEndEXT = VULKAN_LOAD_PROC(m_device, vkCmdDebugMarkerEndEXT);
            m_vkDebugMarkerSetObjectNameEXT = VULKAN_LOAD_PROC(m_device, vkDebugMarkerSetObjectNameEXT);

            RHI_ASSERT(m_vkCmdDebugMarkerBeginEXT);
            RHI_ASSERT(m_vkCmdDebugMarkerInsertEXT);
            RHI_ASSERT(m_vkCmdDebugMarkerEndEXT);
            RHI_ASSERT(m_vkDebugMarkerSetObjectNameEXT);
        }
#endif

        m_vkCmdBeginConditionalRenderingEXT = VULKAN_LOAD_PROC(m_device, vkCmdBeginConditionalRenderingEXT);
        m_vkCmdEndConditionalRenderingEXT = VULKAN_LOAD_PROC(m_device, vkCmdEndConditionalRenderingEXT);

        return VK_SUCCESS;
    }
} // namespace RHI::Vulkan