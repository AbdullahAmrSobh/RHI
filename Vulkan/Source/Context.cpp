
#include "RHI-Vulkan/Loader.hpp"

#define VK_USE_PLATFORM_WIN32_KHR
#include "Common.hpp"
#include "Resources.hpp"
#include "CommandList.hpp"
#include "FrameScheduler.hpp"

#include "Context.hpp"

#ifdef VK_USE_PLATFORM_WIN32_KHR
    #define RHI_VULKAN_USE_CURRENT_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
    #define RHI_VULKAN_USE_CURRENT_PLATFORM_SURFACE_EXTENSION_NAME VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    #define RHI_VULKAN_USE_CURRENT_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    #define RHI_VULKAN_USE_CURRENT_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    #define RHI_VULKAN_USE_CURRENT_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    #define RHI_VULKAN_USE_CURRENT_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    #define RHI_VULKAN_USE_CURRENT_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_MIR_KHR || VK_USE_PLATFORM_DISPLAY_KHR)
    #define RHI_VULKAN_USE_CURRENT_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_DISPLAY_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    #define RHI_VULKAN_USE_CURRENT_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    #define RHI_VULKAN_USE_CURRENT_PLATFORM_SURFACE_EXTENSION_NAME VK_MVK_IOS_SURFACE_EXTENSION_NAME
#endif // VK_USE_PLATFORM_WIN32_KHR

namespace RHI
{

    Ptr<Context> CreateVulkanContext(const ApplicationInfo& appInfo, Ptr<DebugCallbacks> debugCallbacks)
    {
        auto context = CreatePtr<Vulkan::IContext>(std::move(debugCallbacks));
        auto result = context->Init(appInfo);
        RHI_ASSERT(result == VK_SUCCESS);
        return std::move(context);
    }
} // namespace RHI

namespace RHI::Vulkan
{

    inline static VkBool32 VKAPI_CALL DebugMessengerCallbacks(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        (void)messageTypes;
        (void)pUserData;

        auto debugCallback = reinterpret_cast<DebugCallbacks*>(pUserData);

        if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            debugCallback->LogInfo(pCallbackData->pMessage);
        }
        else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            debugCallback->LogInfo(pCallbackData->pMessage);
        }
        else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            debugCallback->LogWarnning(pCallbackData->pMessage);
        }
        else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            debugCallback->LogError(pCallbackData->pMessage);
        }

        return VK_FALSE;
    }

    IContext::IContext(Ptr<DebugCallbacks> debugCallbacks)
    {
        m_debugCallbacks = std::move(debugCallbacks);
        m_frameScheduler = CreatePtr<IFrameScheduler>(this);
        m_bindGroupAllocator = CreatePtr<BindGroupAllocator>(m_device);
        m_stagingBuffer = CreatePtr<IStagingBuffer>(this);
    }

    IContext::~IContext()
    {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        vmaDestroyAllocator(m_allocator);
        vkDestroyInstance(m_instance, nullptr);
    }

    VkResult IContext::Init(const ApplicationInfo& appInfo)
    {
        // Create Vulkan instance
        std::vector<const char*> enabledLayersNames = {
            "VK_LAYER_KHRONOS_validation",
        };

        std::vector<const char*> enabledExtensionsNames = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            RHI_VULKAN_USE_CURRENT_PLATFORM_SURFACE_EXTENSION_NAME,
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
        debugCreateInfo.pUserData = m_debugCallbacks.get();

        bool debugExtensionFound = false;

#if RHI_DEBUG
        for (VkExtensionProperties extension : GetAvailableInstanceExtensions())
        {
            auto extensionName = extension.extensionName;
            if (!strcmp(extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
            {
                debugExtensionFound = true;
                continue;
            }
        }
#endif

#if RHI_DEBUG
        if (debugExtensionFound)
            enabledExtensionsNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        else
            m_debugMessenger->LogWarnning("RHI Vulkan: Debug extension not present.\n Vulkan layer validation is disabled.");
#endif

        {
            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pNext = debugExtensionFound ? &debugCreateInfo : nullptr;
            createInfo.flags = {};
            createInfo.pApplicationInfo = &applicationInfo;
            createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayersNames.size());
            createInfo.ppEnabledLayerNames = enabledLayersNames.data();
            createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensionsNames.size());
            createInfo.ppEnabledExtensionNames = enabledExtensionsNames.data();

            VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
            VULKAN_RETURN_VKERR_CODE(result);
        }

        for (VkPhysicalDevice physicalDevice : GetAvailablePhysicalDevices())
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
                // extensions required for a physical device to be eligable
                m_physicalDevice = physicalDevice;
                break;
            }
        }

        // create logical device
        {
            std::vector<const char*> deviceLayerNames = {

            };

            std::vector<const char*> deviceExtensionNames = {
#if RHI_DEBUG
                VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
#endif
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            };

            auto queueFamilyProperties = GetPhysicalDeviceQueueFamilyProperties(m_physicalDevice);
            for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyProperties.size(); queueFamilyIndex++)
            {
                auto queueFamilyProperty = queueFamilyProperties[queueFamilyIndex];

                // Search for main queue that should be able to do all work (graphics, compute and transfer)
                if ((queueFamilyProperty.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) == (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
                {
                    m_graphicsQueueFamilyIndex = queueFamilyIndex;
                    break; // todo remove this;
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
            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

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

            VkDeviceCreateInfo deviceCreateInfo{};
            deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceCreateInfo.pNext = &dynamicRenderingFeatures;
            deviceCreateInfo.flags = 0;
            deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
            deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(deviceLayerNames.size());
            deviceCreateInfo.ppEnabledLayerNames = deviceLayerNames.data();
            deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionNames.size());
            deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionNames.data();
            deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

            VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
            VULKAN_RETURN_VKERR_CODE(result);

            vkGetDeviceQueue(m_device, m_graphicsQueueFamilyIndex, 0, &m_graphicsQueue);

            m_computeQueue = m_graphicsQueue;
            m_transferQueue = m_graphicsQueue;

            // if (m_computeQueueFamilyIndex != UINT32_MAX)
            //     vkGetDeviceQueue(m_device, m_computeQueueFamilyIndex, 0, &m_computeQueue);

            // if (m_transferQueueFamilyIndex != UINT32_MAX)
            //     vkGetDeviceQueue(m_device, m_transferQueueFamilyIndex, 0, &m_transferQueue);

            if (result != VK_SUCCESS)
                return result;
        }

        {
            VmaAllocatorCreateInfo createInfo{};
            createInfo.physicalDevice = m_physicalDevice;
            createInfo.device = m_device;
            createInfo.instance = m_instance;
            createInfo.vulkanApiVersion = VK_API_VERSION_1_3;

            VkResult result = vmaCreateAllocator(&createInfo, &m_allocator);
            VULKAN_RETURN_VKERR_CODE(result);
        }

        // load function pointers

        if (debugExtensionFound)
        {
            m_vkCmdDebugMarkerBeginEXT = VULKAN_LOAD_PROC(m_device, vkCmdDebugMarkerBeginEXT);
            m_vkCmdDebugMarkerInsertEXT = VULKAN_LOAD_PROC(m_device, vkCmdDebugMarkerInsertEXT);
            m_vkCmdDebugMarkerEndEXT = VULKAN_LOAD_PROC(m_device, vkCmdDebugMarkerEndEXT);

            RHI_ASSERT(m_vkCmdDebugMarkerBeginEXT != nullptr);
            RHI_ASSERT(m_vkCmdDebugMarkerInsertEXT != nullptr);
            RHI_ASSERT(m_vkCmdDebugMarkerEndEXT != nullptr);
        }

        auto scheduler = (IFrameScheduler*)m_frameScheduler.get();
        scheduler->Init();
        scheduler->SetBufferedFramesCount(2);

        m_bindGroupAllocator = CreatePtr<BindGroupAllocator>(m_device);

        return VK_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    // Interface implementation
    ////////////////////////////////////////////////////////////
    Ptr<Swapchain> IContext::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        auto swapchain = CreatePtr<ISwapchain>(this);
        auto result = swapchain->Init(createInfo);
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create swapchain object");
        }
        return swapchain;
    }

    Ptr<ShaderModule> IContext::CreateShaderModule(TL::Span<const uint8_t> shaderBlob)
    {
        auto shaderModule = CreatePtr<IShaderModule>(this);
        auto result = shaderModule->Init(shaderBlob);
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create shader module");
        }
        return shaderModule;
    }

    Ptr<Fence> IContext::CreateFence()
    {
        auto fence = CreatePtr<IFence>(this);
        auto result = fence->Init();
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create a fence object");
        }
        return fence;
    }

    Ptr<CommandListAllocator> IContext::CreateCommandListAllocator(QueueType queueType)
    {
        auto commandListAllocator = CreatePtr<ICommandListAllocator>(this);
        auto result = commandListAllocator->Init(queueType);
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create a command_list_allocator object");
        }
        return commandListAllocator;
    }

    Ptr<ResourcePool> IContext::CreateResourcePool(const ResourcePoolCreateInfo& createInfo)
    {
        auto resourcePool = CreatePtr<IResourcePool>(this);
        auto result = resourcePool->Init(createInfo);
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create a resource_pool object");
        }
        return resourcePool;
    }

    Handle<BindGroupLayout> IContext::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        auto [handle, bindGroupLayout] = m_bindGroupLayoutsOwner.InsertZerod();
        auto result = bindGroupLayout.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create bindGroupLayout");
        }
        return handle;
    }

    void IContext::DestroyBindGroupLayout(Handle<BindGroupLayout> handle)
    {
        auto bindGroupLayout = m_bindGroupLayoutsOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ bindGroupLayout->Shutdown(this); });
        // clang-format on
    }

    Handle<BindGroup> IContext::CreateBindGroup(Handle<BindGroupLayout> layoutHandle)
    {
        auto [handle, bindGroup] = m_bindGroupOwner.InsertZerod();
        auto result = bindGroup.Init(this, layoutHandle);
        if (IsError(result))
        {
            DebugLogError("Failed to create bindGroup");
        }
        return handle;
    }

    void IContext::DestroyBindGroup(Handle<BindGroup> handle)
    {
        auto bindGroup = m_bindGroupOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ bindGroup->Shutdown(this); });
        // clang-format on
    }

    void IContext::UpdateBindGroup(Handle<BindGroup> handle, const BindGroupData& data)
    {
        auto bindGroup = m_bindGroupOwner.Get(handle);
        bindGroup->Write(this, data);
    }

    Handle<PipelineLayout> IContext::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        auto [handle, pipelineLayout] = m_pipelineLayoutOwner.InsertZerod();
        auto result = pipelineLayout.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create pipelineLayout");
        }
        return handle;
    }

    void IContext::DestroyPipelineLayout(Handle<PipelineLayout> handle)
    {
        auto pipelineLayout = m_pipelineLayoutOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ pipelineLayout->Shutdown(this); });
        // clang-format on
    }

    Handle<GraphicsPipeline> IContext::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        auto [handle, graphicsPipeline] = m_graphicsPipelineOwner.InsertZerod();
        auto result = graphicsPipeline.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create graphicsPipeline");
        }
        return handle;
    }

    void IContext::DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)
    {
        auto graphicsPipeline = m_graphicsPipelineOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ graphicsPipeline->Shutdown(this); });
        // clang-format on
    }

    Handle<ComputePipeline> IContext::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        auto [handle, computePipeline] = m_computePipelineOwner.InsertZerod();
        auto result = computePipeline.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create computePipeline");
        }
        return handle;
    }

    void IContext::DestroyComputePipeline(Handle<ComputePipeline> handle)
    {
        auto computePipeline = m_computePipelineOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ computePipeline->Shutdown(this); });
        // clang-format on
    }

    Handle<Sampler> IContext::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        auto [handle, sampler] = m_samplerOwner.InsertZerod();
        auto result = sampler.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create sampler");
        }
        return handle;
    }

    void IContext::DestroySampler(Handle<Sampler> handle)
    {
        auto sampler = m_samplerOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ sampler->Shutdown(this); });
        // clang-format on
    }

    Result<Handle<Image>> IContext::CreateImage(const ImageCreateInfo& createInfo)
    {
        auto [handle, image] = m_imageOwner.InsertZerod();
        auto result = image.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create image");
            return result;
        }
        return Result<Handle<Image>>(handle);
    }

    void IContext::DestroyImage(Handle<Image> handle)
    {
        auto image = m_imageOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ image->Shutdown(this); });
        // clang-format on
    }

    Result<Handle<Buffer>> IContext::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        auto [handle, buffer] = m_bufferOwner.InsertZerod();
        auto result = buffer.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create buffer");
            return result;
        }
        return Result<Handle<Buffer>>(handle);
    }

    void IContext::DestroyBuffer(Handle<Buffer> handle)
    {
        auto buffer = m_bufferOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ buffer->Shutdown(this); });
        // clang-format on
    }

    Handle<ImageView> IContext::CreateImageView(const ImageViewCreateInfo& createInfo)
    {
        auto [handle, imageView] = m_imageViewOwner.InsertZerod();
        auto result = imageView.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create image view");
        }
        return handle;
    }

    void IContext::DestroyImageView(Handle<ImageView> handle)
    {
        auto imageView = m_imageViewOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ imageView->Shutdown(this); });
        // clang-format on
    }

    Handle<BufferView> IContext::CreateBufferView(const BufferViewCreateInfo& createInfo)
    {
        auto [handle, bufferView] = m_bufferViewOwner.InsertZerod();
        auto result = bufferView.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create buffer view");
        }
        return handle;
    }

    void IContext::DestroyBufferView(Handle<BufferView> handle)
    {
        auto imageView = m_imageViewOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ imageView->Shutdown(this); });
        // clang-format on
    }

    DeviceMemoryPtr IContext::MapBuffer(Handle<Buffer> handle)
    {
        // TODO: Remove from here (wrap as new function Resources.hpp)
        auto resource = m_bufferOwner.Get(handle);
        auto allocation = resource->allocation.handle;

        DeviceMemoryPtr memoryPtr = nullptr;
        VkResult result = vmaMapMemory(m_allocator, allocation, &memoryPtr);
        VULKAN_ASSERT_SUCCESS(result);
        return memoryPtr;
    }

    void IContext::UnmapBuffer(Handle<Buffer> handle)
    {
        // TODO: Remove from here (wrap as new function Resources.hpp)
        auto resource = m_bufferOwner.Get(handle)->allocation.handle;
        vmaUnmapMemory(m_allocator, resource);
    }

    ////////////////////////////////////////////////////////////
    // Interface implementation
    ////////////////////////////////////////////////////////////

    void IContext::DestroyResources()
    {
        for (auto destroyItem : m_deferDeleteQueue)
        {
            destroyItem();
        }
        m_deferDeleteQueue.clear();
    }

    // VkSemaphore IContext::CreateSemaphore()
    // {
    //     VkSemaphoreCreateInfo createInfo{};
    //     createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    //     createInfo.pNext = nullptr;
    //     createInfo.flags = 0;
    //     VkSemaphore semaphore = VK_NULL_HANDLE;
    //     auto result = vkCreateSemaphore(m_device, &createInfo, nullptr, &semaphore);
    //     VULKAN_ASSERT_SUCCESS(result);
    //     return semaphore;
    // }

    void IContext::FreeSemaphore(VkSemaphore semaphore)
    {
        if (semaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device, semaphore, nullptr);
        }
    }

    std::vector<VkLayerProperties> IContext::GetAvailableInstanceLayerExtensions() const
    {
        uint32_t instanceLayerCount;
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> layers;
        layers.resize(instanceLayerCount);
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, layers.data());
        return layers;
    }

    std::vector<VkExtensionProperties> IContext::GetAvailableInstanceExtensions() const
    {
        uint32_t instanceExtensionsCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, nullptr);
        std::vector<VkExtensionProperties> extensions;
        extensions.resize(instanceExtensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, extensions.data());
        return extensions;
    }

    std::vector<VkLayerProperties> IContext::GetAvailableDeviceLayerExtensions(VkPhysicalDevice physicalDevice) const
    {
        uint32_t instanceLayerCount;
        vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> layers;
        layers.resize(instanceLayerCount);
        vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, layers.data());
        return layers;
    }

    std::vector<VkExtensionProperties> IContext::GetAvailableDeviceExtensions(VkPhysicalDevice physicalDevice) const
    {
        uint32_t extensionsCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr);
        std::vector<VkExtensionProperties> extnesions;
        extnesions.resize(extensionsCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, extnesions.data());
        return extnesions;
    }

    std::vector<VkPhysicalDevice> IContext::GetAvailablePhysicalDevices() const
    {
        uint32_t physicalDeviceCount;
        vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
        VkResult result = vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());
        RHI_ASSERT(result == VK_SUCCESS);
        return physicalDevices;
    }

    std::vector<VkQueueFamilyProperties> IContext::GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice) const
    {
        uint32_t queueFamilyPropertiesCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties{};
        queueFamilyProperties.resize(queueFamilyPropertiesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());
        return queueFamilyProperties;
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

} // namespace RHI::Vulkan