
#include "RHI-Vulkan/Loader.hpp"

#include "Common.hpp"
#include "CommandPool.hpp"
#include "CommandList.hpp"
#include "Swapchain.hpp"
#include "Device.hpp"
#include "Queue.hpp"

#include <TL/Assert.hpp>
#include <TL/Log.hpp>
#include <TL/UniquePtr.hpp>

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

#define VULKAN_DEVICE_FUNC_LOAD(device, proc) reinterpret_cast<PFN_##proc>(vkGetDeviceProcAddr(device, #proc));
#define VULKAN_INSTANCE_FUNC_LOAD(instance, proc) reinterpret_cast<PFN_##proc>(vkGetInstanceProcAddr(instance, #proc));

namespace RHI
{
    TL::Ptr<Device> CreateVulkanDevice(const ApplicationInfo& appInfo)
    {
        ZoneScoped;

        auto device = TL::CreatePtr<Vulkan::IDevice>();
        auto result = device->Init(appInfo);
        TL_ASSERT(IsSuccess(result));
        return std::move(device);
    }
} // namespace RHI

namespace RHI::Vulkan
{

    /// @todo: add support for a custom sink, so vulkan errors are spereated
    VkBool32 DebugMessengerCallbacks(
        [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        [[maybe_unused]] void* pUserData)
    {
        // Collect additional information for logging
        TL::String additionalInfo;

        // Log the object names, debug markers, and queue information if available
        if (pCallbackData->objectCount > 0)
        {
            for (uint32_t i = 0; i < pCallbackData->objectCount; ++i)
            {
                // clang-format off
                additionalInfo += std::format("Object[{}]: Type: {}, Name: {}\n",
                                          i,
                                          ObjectTypeToName(pCallbackData->pObjects[i].objectType),
                                          pCallbackData->pObjects[i].pObjectName ? pCallbackData->pObjects[i].pObjectName : "Unnamed");
                // clang-format on
            }
        }

        // Log any labels from the active debug marker stack
        if (pCallbackData->cmdBufLabelCount > 0)
        {
            additionalInfo += "Active Debug Markers:\n";
            for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i)
            {
                // clang-format off
                additionalInfo += std::format("Label[{}]: {} (color: [{:.2f}, {:.2f}, {:.2f}, {:.2f}])\n",
                                          i,
                                          pCallbackData->pCmdBufLabels[i].pLabelName ? pCallbackData->pCmdBufLabels[i].pLabelName : "Unnamed",
                                          pCallbackData->pCmdBufLabels[i].color[0],
                                          pCallbackData->pCmdBufLabels[i].color[1],
                                          pCallbackData->pCmdBufLabels[i].color[2],
                                          pCallbackData->pCmdBufLabels[i].color[3]);
                // clang-format on
            }
        }

        // Log queue information if available
        if (pCallbackData->queueLabelCount > 0)
        {
            additionalInfo += "Queue Labels:\n";
            for (uint32_t i = 0; i < pCallbackData->queueLabelCount; ++i)
            {
                // clang-format off
                additionalInfo += std::format("Queue[{}]: {} (color: [{:.2f}, {:.2f}, {:.2f}, {:.2f}])\n",
                                          i,
                                          pCallbackData->pQueueLabels[i].pLabelName ? pCallbackData->pQueueLabels[i].pLabelName : "Unnamed",
                                          pCallbackData->pQueueLabels[i].color[0],
                                          pCallbackData->pQueueLabels[i].color[1],
                                          pCallbackData->pQueueLabels[i].color[2],
                                          pCallbackData->pQueueLabels[i].color[3]);
                // clang-format on
            }
        }

        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: TL_LOG_INFO("{}\nMessage: {}", additionalInfo, pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    TL_LOG_INFO("{}\nMessage: {}", additionalInfo, pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: TL_LOG_WARNNING("{}\nMessage: {}", additionalInfo, pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   TL_LOG_ERROR("{}\nMessage: {}", additionalInfo, pCallbackData->pMessage); break;
        default:
            TL_UNREACHABLE();
        }

        return VK_FALSE;
    }

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

    IDevice::IDevice()
        : Device()
        , m_instance(VK_NULL_HANDLE)
        , m_physicalDevice(VK_NULL_HANDLE)
        , m_device(VK_NULL_HANDLE)
        , m_allocator(VK_NULL_HANDLE)
        , m_pfn()
        , m_queue()
        , m_deleteQueue(this)
        , m_bindGroupAllocator(TL::CreatePtr<BindGroupAllocator>(this))
        , m_commandPool(TL::CreatePtr<ICommandPool>(this))
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

    IDevice::~IDevice()
    {
        ZoneScoped;

        vkDeviceWaitIdle(m_device);

        delete m_commandPool.release();
        m_bindGroupAllocator->Shutdown();

        m_deleteQueue.Shutdown();

        vmaDestroyAllocator(m_allocator);
        vkDestroyDevice(m_device, nullptr);

#if RHI_DEBUG
        if (auto fn = m_pfn.m_vkDestroyDebugUtilsMessengerEXT; fn && m_debugUtilsMessenger)
        {
            fn(m_instance, m_debugUtilsMessenger, nullptr);
        }
#endif

        vkDestroyInstance(m_instance, nullptr);
    }

    ResultCode IDevice::Init(const ApplicationInfo& appInfo)
    {
        ZoneScoped;

        bool debugExtensionEnabled = false;

        TRY_OR_RETURN(InitInstance(appInfo, &debugExtensionEnabled));
        TRY_OR_RETURN(InitDevice());
        TRY_OR_RETURN(InitMemoryAllocator());

        TRY_OR_RETURN_VK(m_bindGroupAllocator->Init());
        TRY_OR_RETURN_VK(m_commandPool->Init(CommandPoolFlags::Reset));

        return ResultCode::Success;
    }

    void IDevice::SetDebugName(VkObjectType type, uint64_t handle, const char* name) const
    {
        if (handle == 0 /* VK_NULL_HANDLE */) return;

        if (auto fn = m_pfn.m_vkSetDebugUtilsObjectNameEXT; fn && name)
        {
            VkDebugUtilsObjectNameInfoEXT nameInfo{};
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            nameInfo.pNext = nullptr;
            nameInfo.pObjectName = name;
            nameInfo.objectType = type;
            nameInfo.objectHandle = handle;
            fn(m_device, &nameInfo);
        }
    }

    uint32_t IDevice::GetMemoryTypeIndex(MemoryType memoryType)
    {
        VkPhysicalDeviceMemoryProperties2 memoryProperties;
        vkGetPhysicalDeviceMemoryProperties2(m_physicalDevice, &memoryProperties);

        // Loop through all available memory types to find the appropriate one
        for (uint32_t i = 0; i < memoryProperties.memoryProperties.memoryTypeCount; ++i)
        {
            const VkMemoryType& vkMemoryType = memoryProperties.memoryProperties.memoryTypes[i];

            switch (memoryType)
            {
            case MemoryType::CPU:
                // Host visible and coherent memory for CPU access
                if (vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT &&
                    vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                {
                    return i;
                }
                break;

            case MemoryType::GPULocal:
                // Device local memory for GPU-only access
                if (vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                {
                    return i;
                }
                break;

            case MemoryType::GPUShared:
                // Device local but also host visible and coherent, used for streaming resources
                if (vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT &&
                    vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT &&
                    vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                {
                    return i;
                }
                break;

            default:
                TL_UNREACHABLE(); // Invalid MemoryType
            }
        }

        // If no suitable memory type was found, return an invalid index
        TL_LOG_ERROR("Failed to find suitable memory type for {}", static_cast<int>(memoryType));
        return UINT32_MAX; // Return an invalid index to indicate failure
    }

    void IDevice::FillLimits()
    {
        DeviceLimits& limits = *m_limits;

        // Get physical device properties
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);

        // General image limits
        limits.maxImage2DSize = physicalDeviceProperties.limits.maxImageDimension2D;
        limits.maxImageCubeSize = physicalDeviceProperties.limits.maxImageDimensionCube;
        limits.maxImage3DSize = physicalDeviceProperties.limits.maxImageDimension3D;
        limits.maxImageArrayLayers = physicalDeviceProperties.limits.maxImageArrayLayers;

        // Buffer size limits
        limits.maxUniformBufferSize = physicalDeviceProperties.limits.maxUniformBufferRange;
        limits.maxStorageBufferSize = physicalDeviceProperties.limits.maxStorageBufferRange;
        limits.maxConstantBufferSize = physicalDeviceProperties.limits.maxUniformBufferRange;

        // Bind group (resource binding) limits
        limits.maxBindGroups = 4; // Adjust based on your device
        // limits.maxResourcesPerBindGroup = maxDescriptorSetBindings; // Adjust based on your device
        // limits.maxSamplersPerBindGroup = maxPerStageDescriptorSamplers;
        // limits.maxImagesPerBindGroup = maxPerStageDescriptorImages;
        // limits.maxBuffersPerBindGroup = maxPerStageDescriptorStorageBuffers;

        // Descriptor indexing limits (for Vulkan 1.2 and above)
        if (physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_2)
        {
            VkPhysicalDeviceDescriptorIndexingProperties descriptorIndexingProperties;
            descriptorIndexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
            descriptorIndexingProperties.pNext = nullptr;

            VkPhysicalDeviceProperties2 properties2 = {};
            properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            properties2.pNext = &descriptorIndexingProperties;

            vkGetPhysicalDeviceProperties2(m_physicalDevice, &properties2);

            limits.maxDynamicBuffers = descriptorIndexingProperties.maxUpdateAfterBindDescriptorsInAllPools;
            limits.maxDescriptorsPerSet = descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindSamplers; // Adjust as needed
        }

        // Buffer alignment requirements
        limits.uniformBufferAlignment = (uint32_t)physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
        limits.storageBufferAlignment = (uint32_t)physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;
        limits.dynamicBufferAlignment = (uint32_t)physicalDeviceProperties.limits.minUniformBufferOffsetAlignment; // Adjust if needed

        // Memory heap sizes
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

        // Memory allocation limits
        limits.maxMemoryAllocationSize = physicalDeviceProperties.limits.maxMemoryAllocationCount; // Adjust as needed
        limits.memoryTypeCount = memoryProperties.memoryTypeCount;                                 // Define this function to return the number of memory types
        for (uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i)
        {
            if (i < memoryProperties.memoryHeapCount)
            {
                limits.memoryHeapSize[i] = memoryProperties.memoryHeaps[i].size;
            }
            else
            {
                limits.memoryHeapSize[i] = 0; // No more heaps available
            }
        }
    }

    ////////////////////////////////////////////////////////////
    // Interface implementation
    ////////////////////////////////////////////////////////////
    TL::Ptr<Swapchain> IDevice::Impl_CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        auto swapchain = TL::CreatePtr<ISwapchain>(this);
        auto result = swapchain->Init(createInfo);
        if (result != VK_SUCCESS)
        {
            TL_LOG_ERROR("Failed to create swapchain object");
        }
        return swapchain;
    }

    TL::Ptr<ShaderModule> IDevice::Impl_CreateShaderModule(TL::Span<const uint32_t> shaderBlob)
    {
        auto shaderModule = TL::CreatePtr<IShaderModule>(this);
        auto result = shaderModule->Init(shaderBlob);
        if (result != ResultCode::Success)
        {
            TL_LOG_ERROR("Failed to create shader module");
        }
        return shaderModule;
    }

    TL::Ptr<Fence> IDevice::Impl_CreateFence()
    {
        auto fence = TL::CreatePtr<IFence>(this);
        auto result = fence->Init();
        if (result != ResultCode::Success)
        {
            TL_LOG_ERROR("Failed to create a fence object");
        }
        return fence;
    }

    TL::Ptr<CommandPool> IDevice::Impl_CreateCommandPool(CommandPoolFlags flags)
    {
        auto commandPool = TL::CreatePtr<ICommandPool>(this);
        auto result = commandPool->Init(flags);
        if (result != ResultCode::Success)
        {
            TL_LOG_ERROR("Failed to create a command_list_allocator object");
        }
        return commandPool;
    }

    Handle<BindGroupLayout> IDevice::Impl_CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        IBindGroupLayout bindGroupLayout{};
        auto result = bindGroupLayout.Init(this, createInfo);
        if (IsError(result))
        {
            TL_LOG_ERROR("Failed to create bindGroupLayout");
        }
        return m_bindGroupLayoutsOwner.Emplace(std::move(bindGroupLayout));
    }

    void IDevice::Impl_DestroyBindGroupLayout(Handle<BindGroupLayout> handle)
    {
        TL_ASSERT(handle != NullHandle);

        auto bindGroupLayout = m_bindGroupLayoutsOwner.Get(handle);
        bindGroupLayout->Shutdown(this);
        m_bindGroupLayoutsOwner.Release(handle);
    }

    Handle<BindGroup> IDevice::Impl_CreateBindGroup(Handle<BindGroupLayout> layoutHandle)
    {
        IBindGroup bindGroup{};
        auto result = bindGroup.Init(this, layoutHandle);
        if (IsError(result))
        {
            TL_LOG_ERROR("Failed to create bindGroup");
        }
        auto handle = m_bindGroupOwner.Emplace(std::move(bindGroup));
        return handle;
    }

    void IDevice::Impl_DestroyBindGroup(Handle<BindGroup> handle)
    {
        TL_ASSERT(handle != NullHandle);

        auto bindGroup = m_bindGroupOwner.Get(handle);
        bindGroup->Shutdown(this);
        m_bindGroupOwner.Release(handle);
    }

    void IDevice::Impl_UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo)
    {
        auto bindGroup = m_bindGroupOwner.Get(handle);
        bindGroup->Write(this, updateInfo);
    }

    Handle<PipelineLayout> IDevice::Impl_CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        IPipelineLayout pipelineLayout{};
        auto result = pipelineLayout.Init(this, createInfo);
        if (IsError(result))
        {
            TL_LOG_ERROR("Failed to create pipelineLayout");
        }
        auto handle = m_pipelineLayoutOwner.Emplace(std::move(pipelineLayout));
        return handle;
    }

    void IDevice::Impl_DestroyPipelineLayout(Handle<PipelineLayout> handle)
    {
        TL_ASSERT(handle != NullHandle);

        auto pipelineLayout = m_pipelineLayoutOwner.Get(handle);
        pipelineLayout->Shutdown(this);
        m_pipelineLayoutOwner.Release(handle);
    }

    Handle<GraphicsPipeline> IDevice::Impl_CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        IGraphicsPipeline graphicsPipeline{};
        auto result = graphicsPipeline.Init(this, createInfo);
        if (IsError(result))
        {
            TL_LOG_ERROR("Failed to create graphicsPipeline");
        }
        auto handle = m_graphicsPipelineOwner.Emplace(std::move(graphicsPipeline));
        return handle;
    }

    void IDevice::Impl_DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)
    {
        TL_ASSERT(handle != NullHandle);

        auto pipeline = m_graphicsPipelineOwner.Get(handle);
        pipeline->Shutdown(this);
        m_graphicsPipelineOwner.Release(handle);
    }

    Handle<ComputePipeline> IDevice::Impl_CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        IComputePipeline computePipeline{};
        auto result = computePipeline.Init(this, createInfo);
        if (IsError(result))
        {
            TL_LOG_ERROR("Failed to create computePipeline");
        }
        auto handle = m_computePipelineOwner.Emplace(std::move(computePipeline));
        return handle;
    }

    void IDevice::Impl_DestroyComputePipeline(Handle<ComputePipeline> handle)
    {
        TL_ASSERT(handle != NullHandle);

        auto pipeline = m_computePipelineOwner.Get(handle);
        pipeline->Shutdown(this);
        m_computePipelineOwner.Release(handle);
    }

    Handle<Sampler> IDevice::Impl_CreateSampler(const SamplerCreateInfo& createInfo)
    {
        ISampler sampler{};
        auto result = sampler.Init(this, createInfo);
        if (IsError(result))
        {
            TL_LOG_ERROR("Failed to create sampler");
        }
        auto handle = m_samplerOwner.Emplace(std::move(sampler));
        return handle;
    }

    void IDevice::Impl_DestroySampler(Handle<Sampler> handle)
    {
        TL_ASSERT(handle != NullHandle);

        auto sampler = m_samplerOwner.Get(handle);
        sampler->Shutdown(this);
        m_samplerOwner.Release(handle);
    }

    Result<Handle<Image>> IDevice::Impl_CreateImage(const ImageCreateInfo& createInfo)
    {
        IImage image{};
        auto result = image.Init(this, createInfo);
        SetDebugName(image.handle, createInfo.name);
        if (IsError(result))
        {
            TL_LOG_ERROR("Failed to create image");
            return result;
        }
        auto handle = m_imageOwner.Emplace(std::move(image));
        return Result<Handle<Image>>(handle);
    }

    void IDevice::Impl_DestroyImage(Handle<Image> handle)
    {
        TL_ASSERT(handle != NullHandle);

        auto image = m_imageOwner.Get(handle);
        image->Shutdown(this);
        m_imageOwner.Release(handle);
    }

    Result<Handle<Buffer>> IDevice::Impl_CreateBuffer(const BufferCreateInfo& createInfo)
    {
        IBuffer buffer{};
        auto result = buffer.Init(this, createInfo);
        if (IsError(result))
        {
            TL_LOG_ERROR("Failed to create buffer");
            return result;
        }
        auto handle = m_bufferOwner.Emplace(std::move(buffer));
        return Result<Handle<Buffer>>(handle);
    }

    void IDevice::Impl_DestroyBuffer(Handle<Buffer> handle)
    {
        TL_ASSERT(handle != NullHandle);

        auto buffer = m_bufferOwner.Get(handle);
        buffer->Shutdown(this);
        m_bufferOwner.Release(handle);
    }

    Handle<ImageView> IDevice::Impl_CreateImageView(const ImageViewCreateInfo& createInfo)
    {
        IImageView imageView{};
        auto result = imageView.Init(this, createInfo);
        if (IsError(result))
        {
            TL_LOG_ERROR("Failed to create image view");
        }
        auto handle = m_imageViewOwner.Emplace(std::move(imageView));
        return handle;
    }

    void IDevice::Impl_DestroyImageView(Handle<ImageView> handle)
    {
        TL_ASSERT(handle != NullHandle);

        auto imageView = m_imageViewOwner.Get(handle);
        imageView->Shutdown(this);
        m_imageViewOwner.Release(handle);
    }

    Handle<BufferView> IDevice::Impl_CreateBufferView(const BufferViewCreateInfo& createInfo)
    {
        IBufferView bufferView{};
        auto result = bufferView.Init(this, createInfo);
        if (IsError(result))
        {
            TL_LOG_ERROR("Failed to create buffer view");
        }
        auto handle = m_bufferViewOwner.Emplace(std::move(bufferView));
        return handle;
    }

    void IDevice::Impl_DestroyBufferView(Handle<BufferView> handle)
    {
        TL_ASSERT(handle != NullHandle);

        auto bufferView = m_bufferViewOwner.Get(handle);
        bufferView->Shutdown(this);
        m_bufferViewOwner.Release(handle);
    }

    DeviceMemoryPtr IDevice::Impl_MapBuffer(Handle<Buffer> handle)
    {
        auto resource = m_bufferOwner.Get(handle);
        auto allocation = resource->allocation.handle;

        DeviceMemoryPtr memoryPtr = nullptr;
        Validate(vmaMapMemory(m_allocator, allocation, &memoryPtr));
        return memoryPtr;
    }

    void IDevice::Impl_UnmapBuffer(Handle<Buffer> handle)
    {
        auto resource = m_bufferOwner.Get(handle)->allocation.handle;
        vmaUnmapMemory(m_allocator, resource);
    }

    Handle<Semaphore> IDevice::Impl_CreateSemaphore(const SemaphoreCreateInfo& createInfo)
    {
        ISemaphore semaphore{};
        auto result = semaphore.Init(this, createInfo);
        if (IsError(result))
        {
            TL_LOG_ERROR("Failed to create semaphore");
        }
        auto handle = m_semaphoreOwner.Emplace(std::move(semaphore));
        return handle;
    }

    void IDevice::Impl_DestroySemaphore(Handle<Semaphore> handle)
    {
        TL_ASSERT(handle != NullHandle);

        auto semaphore = m_semaphoreOwner.Get(handle);
        semaphore->Shutdown(this);
        m_semaphoreOwner.Release(handle);
    }

    Queue* IDevice::Impl_GetQueue(QueueType queueType)
    {
        return &m_queue[(uint32_t)queueType];
    }

    void IDevice::Impl_CollectResources()
    {
        m_deleteQueue.ExecuteDeletions();
    }

    ////////////////////////////////////////////////////////////
    // Interface implementation
    ////////////////////////////////////////////////////////////

    VkResult IDevice::InitInstance(const ApplicationInfo& appInfo, bool* debugExtensionEnabled)
    {
        VkResult result;

        TL::Vector<const char*> layers = {
            "VK_LAYER_KHRONOS_validation",
        };

        TL::Vector<const char*> extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VULKAN_SURFACE_OS_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        };

        VkApplicationInfo applicationInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = appInfo.applicationName,
            .applicationVersion = VK_MAKE_API_VERSION(0, appInfo.applicationVersion.major, appInfo.applicationVersion.minor, appInfo.applicationVersion.patch),
            .pEngineName = appInfo.engineName,
            .engineVersion = VK_MAKE_API_VERSION(0, appInfo.engineVersion.major, appInfo.engineVersion.minor, appInfo.engineVersion.patch),
            .apiVersion = VK_API_VERSION_1_3,
        };

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsCI{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .flags = 0,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
            .pfnUserCallback = DebugMessengerCallbacks,
            .pUserData = this,
        };

#if RHI_DEBUG
        for (VkExtensionProperties extension : GetAvailableInstanceExtensions())
        {
            auto extensionName = extension.extensionName;
            if (strcmp(extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
            {
                *debugExtensionEnabled = true;
            }
        }

        if (*debugExtensionEnabled)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else
        {
            TL_LOG_WARNNING("RHI Vulkan: Debug extension not present.");
        }
#endif

        VkInstanceCreateInfo instanceCI
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#if RHI_DEBUG
            .pNext = *debugExtensionEnabled ? &debugUtilsCI : nullptr,
#else
            .pNext = nullptr,
#endif
            .flags = {},
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = static_cast<uint32_t>(layers.size()),
            .ppEnabledLayerNames = layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };
        result = vkCreateInstance(&instanceCI, nullptr, &m_instance);
        if (result != VK_SUCCESS)
            return result;

#if RHI_DEBUG
        if (debugExtensionEnabled)
        {
            m_pfn.m_vkCreateDebugUtilsMessengerEXT = VULKAN_INSTANCE_FUNC_LOAD(m_instance, vkCreateDebugUtilsMessengerEXT);
            m_pfn.m_vkDestroyDebugUtilsMessengerEXT = VULKAN_INSTANCE_FUNC_LOAD(m_instance, vkDestroyDebugUtilsMessengerEXT);
            result = m_pfn.m_vkCreateDebugUtilsMessengerEXT(m_instance, &debugUtilsCI, nullptr, &m_debugUtilsMessenger);
        }
#endif
        return result;
    }

    VkResult IDevice::InitDevice()
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
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME
        };

        uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t transferQueueFamilyIndex = UINT32_MAX;
        uint32_t computeQueueFamilyIndex = UINT32_MAX;

        auto queueFamilyProperties = GetPhysicalDeviceQueueFamilyProperties(m_physicalDevice);
        for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyProperties.size(); queueFamilyIndex++)
        {
            auto queueFamilyProperty = queueFamilyProperties[queueFamilyIndex];

            // Search for main queue that should be able to do all work (graphics, compute and transfer)
            if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphicsQueueFamilyIndex = queueFamilyIndex;
                transferQueueFamilyIndex = queueFamilyIndex;
                computeQueueFamilyIndex = queueFamilyIndex;
                // TODO: remove this break to support multiple queues for different tasks
                break;
            }
            else if (queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                computeQueueFamilyIndex = queueFamilyIndex;
            }
            else if (queueFamilyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                transferQueueFamilyIndex = queueFamilyIndex;
            }
        }

        float queuePriority = 1.0f;
        TL::Vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

        VkDeviceQueueCreateInfo queueCI{};
        queueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCI.pNext = nullptr;
        queueCI.flags = 0;

        if (graphicsQueueFamilyIndex != UINT32_MAX)
        {
            queueCI.queueFamilyIndex = graphicsQueueFamilyIndex;
            queueCI.queueCount = 1;
            queueCI.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCI);
        }
        else if (computeQueueFamilyIndex != UINT32_MAX)
        {
            queueCI.queueFamilyIndex = computeQueueFamilyIndex;
            queueCI.queueCount = 1;
            queueCI.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCI);
        }
        else if (transferQueueFamilyIndex != UINT32_MAX)
        {
            queueCI.queueFamilyIndex = transferQueueFamilyIndex;
            queueCI.queueCount = 1;
            queueCI.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCI);
        }
        else
        {
            TL_UNREACHABLE();
        }

        VkPhysicalDeviceFeatures2 features{};
        VkPhysicalDeviceVulkan11Features features11{};
        VkPhysicalDeviceVulkan12Features features12{};
        VkPhysicalDeviceVulkan13Features features13{};

        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features.pNext = &features11;
        features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        features11.pNext = &features12;
        features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        features12.pNext = &features13;
        features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        features13.pNext = nullptr;

        features.features.samplerAnisotropy = VK_TRUE;
        features12.descriptorBindingPartiallyBound = VK_TRUE;
        features12.descriptorBindingVariableDescriptorCount = VK_TRUE;
        features12.runtimeDescriptorArray = VK_TRUE;
        features13.synchronization2 = VK_TRUE;
        features13.dynamicRendering = VK_TRUE;

        VkDeviceCreateInfo deviceCI{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features,
            .flags = 0,
            .queueCreateInfoCount = (uint32_t)queueCreateInfos.size(),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledLayerCount = (uint32_t)deviceLayerNames.size(),
            .ppEnabledLayerNames = deviceLayerNames.data(),
            .enabledExtensionCount = (uint32_t)deviceExtensionNames.size(),
            .ppEnabledExtensionNames = deviceExtensionNames.data(),
            .pEnabledFeatures = nullptr,
        };
        auto result = vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device);

#if RHI_DEBUG
        if (m_pfn.m_vkCreateDebugUtilsMessengerEXT)
        {
            m_pfn.m_vkCmdBeginDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdBeginDebugUtilsLabelEXT);
            m_pfn.m_vkCmdEndDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdEndDebugUtilsLabelEXT);
            m_pfn.m_vkCmdInsertDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdInsertDebugUtilsLabelEXT);
            m_pfn.m_vkQueueBeginDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkQueueBeginDebugUtilsLabelEXT);
            m_pfn.m_vkQueueEndDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkQueueEndDebugUtilsLabelEXT);
            m_pfn.m_vkQueueInsertDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkQueueInsertDebugUtilsLabelEXT);
            m_pfn.m_vkSetDebugUtilsObjectNameEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkSetDebugUtilsObjectNameEXT);
            m_pfn.m_vkSetDebugUtilsObjectTagEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkSetDebugUtilsObjectTagEXT);
            m_pfn.m_vkSubmitDebugUtilsMessageEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkSubmitDebugUtilsMessageEXT);
        }
#endif
        m_pfn.m_vkCmdBeginConditionalRenderingEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdBeginConditionalRenderingEXT);
        m_pfn.m_vkCmdEndConditionalRenderingEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdEndConditionalRenderingEXT);

        m_queue[(uint32_t)QueueType::Graphics] = IQueue(this, graphicsQueueFamilyIndex);
        m_queue[(uint32_t)QueueType::Compute] = IQueue(this, computeQueueFamilyIndex);
        m_queue[(uint32_t)QueueType::Transfer] = IQueue(this, transferQueueFamilyIndex);

        return result;
    }

    VkResult IDevice::InitMemoryAllocator()
    {
        VmaAllocatorCreateInfo vmaCI{};
        vmaCI.physicalDevice = m_physicalDevice;
        vmaCI.device = m_device;
        vmaCI.instance = m_instance;
        vmaCI.vulkanApiVersion = VK_API_VERSION_1_3;
        return vmaCreateAllocator(&vmaCI, &m_allocator);
    }

} // namespace RHI::Vulkan