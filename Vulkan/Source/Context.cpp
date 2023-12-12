#define VK_USE_PLATFORM_WIN32_KHR

#include "CommandList.hpp"
#include "Common.hpp"
#include "FrameScheduler.hpp"
#include "RHI-Vulkan/Loader.hpp"
#include "Resources.hpp"

#undef CreateSemaphore
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

std::unique_ptr<RHI::Context> RHI::CreateVulkanRHI(const RHI::ApplicationInfo& appInfo, std::unique_ptr<RHI::DebugCallbacks> debugCallbacks)
{
    auto context = std::make_unique<Vulkan::Context>(std::move(debugCallbacks));
    auto result = context->Init(appInfo);
    RHI_ASSERT(result == VK_SUCCESS);
    return std::move(context);
}

namespace Vulkan
{

    inline static VkBool32 VKAPI_CALL DebugMessengerCallbacks(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        (void)messageTypes;
        (void)pUserData;

        auto debugCallback = reinterpret_cast<RHI::DebugCallbacks*>(pUserData);

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

    Context::~Context()
    {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        vmaDestroyAllocator(m_allocator);
        vkDestroyInstance(m_instance, nullptr);
    }

    VkResult Context::Init(const RHI::ApplicationInfo& appInfo)
    {
        // Create Vulkan instance
        std::vector<const char*> enabledLayersNames = {
            "VK_LAYER_KHRONOS_validation",
        };

        std::vector<const char*> enabledExtensionsNames = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            RHI_VULKAN_USE_CURRENT_PLATFORM_SURFACE_EXTENSION_NAME,
            VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        };

        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pNext = nullptr;
        applicationInfo.pApplicationName = appInfo.applicationName;
        applicationInfo.applicationVersion = appInfo.engineVersion;
        applicationInfo.pEngineName = appInfo.engineName;
        applicationInfo.engineVersion = appInfo.engineVersion;
        applicationInfo.apiVersion = VK_API_VERSION_1_3;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugMessengerCallbacks;
        debugCreateInfo.pUserData = m_debugMessenger.get();

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
                VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                VK_KHR_MAINTENANCE2_EXTENSION_NAME,
                VK_KHR_MULTIVIEW_EXTENSION_NAME,
                VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
                VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
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

        VULKAN_ASSERT_SUCCESS(result);
        return result;
    }

    std::unique_ptr<RHI::Swapchain> Context::CreateSwapchain(const RHI::SwapchainCreateInfo& createInfo)
    {
        auto swapchain = std::make_unique<Swapchain>(this);
        auto result = swapchain->Init(createInfo);
        RHI_ASSERT(result == VK_SUCCESS);
        return swapchain;
    }

    std::unique_ptr<RHI::ShaderModule> Context::CreateShaderModule(const RHI::ShaderModuleCreateInfo& createInfo)
    {
        auto shaderModule = std::make_unique<ShaderModule>(this);
        auto result = shaderModule->Init(createInfo);
        RHI_ASSERT(result == VK_SUCCESS);
        return shaderModule;
    }

    std::unique_ptr<RHI::FrameScheduler> Context::CreateFrameScheduler()
    {
        auto scheduler = std::make_unique<FrameScheduler>(this);
        auto result = scheduler->Init();
        RHI_ASSERT(result == VK_SUCCESS);
        return scheduler;
    }

    std::unique_ptr<RHI::CommandListAllocator> Context::CreateCommandListAllocator(RHI::QueueType queueType)
    {
        auto allocator = std::make_unique<CommandListAllocator>(this, 3);
        auto result = allocator->Init(GetQueueFamilyIndex(queueType));
        RHI_ASSERT(result == VK_SUCCESS);
        return allocator;
    }

    RHI::Handle<RHI::BindGroupLayout> Context::CreateBindGroupLayout(const RHI::BindGroupLayoutCreateInfo& createInfo)
    {
        auto [handle, bindGroupLayout] = m_bindGroupLayoutsOwner.InsertZerod();
        auto result = bindGroupLayout.Init(this, createInfo);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return handle;
    }

    void Context::DestroyBindGroupLayout(RHI::Handle<RHI::BindGroupLayout> handle)
    {
        auto layout = m_bindGroupLayoutsOwner.Get(handle);
        layout->Shutdown(this);
        m_bindGroupLayoutsOwner.Remove(handle);
    }

    RHI::Handle<RHI::PipelineLayout> Context::CreatePipelineLayout(const RHI::PipelineLayoutCreateInfo& createInfo)
    {
        auto [handle, pipelineLayout] = m_pipelineLayoutOwner.InsertZerod();
        auto result = pipelineLayout.Init(this, createInfo);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return handle;
    }

    void Context::DestroyPipelineLayout(RHI::Handle<RHI::PipelineLayout> handle)
    {
        auto layout = m_pipelineLayoutOwner.Get(handle);
        layout->Shutdown(this);
        m_pipelineLayoutOwner.Remove(handle);
    }

    std::unique_ptr<RHI::BindGroupAllocator> Context::CreateBindGroupAllocator()
    {
        auto bindGroupAllocator = std::make_unique<BindGroupAllocator>(this);
        auto result = bindGroupAllocator->Init();
        RHI_ASSERT(result == VK_SUCCESS);
        return bindGroupAllocator;
    }

    std::unique_ptr<RHI::BufferPool> Context::CreateBufferPool(const RHI::PoolCreateInfo& createInfo)
    {
        auto pool = std::make_unique<BufferPool>(this);
        auto result = pool->Init(createInfo);
        RHI_ASSERT(result == VK_SUCCESS);
        return pool;
    }

    std::unique_ptr<RHI::ImagePool> Context::CreateImagePool(const RHI::PoolCreateInfo& createInfo)
    {
        auto pool = std::make_unique<ImagePool>(this);
        auto result = pool->Init(createInfo);
        RHI_ASSERT(result == VK_SUCCESS);
        return pool;
    }

    RHI::Handle<RHI::GraphicsPipeline> Context::CreateGraphicsPipeline(const RHI::GraphicsPipelineCreateInfo& createInfo)
    {
        auto [handle, pipeline] = m_graphicsPipelineOwner.InsertZerod();
        auto result = pipeline.Init(this, createInfo);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return handle;
    }

    void Context::DestroyGraphicsPipeline(RHI::Handle<RHI::GraphicsPipeline> handle)
    {
        auto layout = m_graphicsPipelineOwner.Get(handle);
        layout->Shutdown(this);
        m_graphicsPipelineOwner.Remove(handle);
    }

    RHI::Handle<RHI::ComputePipeline> Context::CreateComputePipeline(const RHI::ComputePipelineCreateInfo& createInfo)
    {
        auto [handle, pipeline] = m_computePipelineOwner.InsertZerod();
        auto result = pipeline.Init(this, createInfo);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return handle;
    }

    void Context::DestroyComputePipeline(RHI::Handle<RHI::ComputePipeline> handle)
    {
        auto layout = m_computePipelineOwner.Get(handle);
        layout->Shutdown(this);
        m_computePipelineOwner.Remove(handle);
    }

    RHI::Handle<RHI::Sampler> Context::CreateSampler(const RHI::SamplerCreateInfo& createInfo)
    {
        auto [handle, sampler] = m_samplerOwner.InsertZerod();
        auto result = sampler.Init(this, createInfo);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return handle;
    }

    void Context::DestroySampler(RHI::Handle<RHI::Sampler> handle)
    {
        auto sampler = m_samplerOwner.Get(handle);
        sampler->Shutdown(this);
        m_samplerOwner.Remove(handle);
    }

    RHI::Handle<RHI::ImageView> Context::CreateImageView(RHI::Handle<RHI::Image> imageHandle, const RHI::ImageAttachmentUseInfo& useInfo)
    {
        auto [handle, imageView] = m_imageViewOwner.InsertZerod();
        auto result = imageView.Init(this, imageHandle, useInfo);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return handle;
    }

    void Context::DestroyImageView(RHI::Handle<RHI::ImageView> handle)
    {
        auto imageView = m_imageViewOwner.Get(handle);
        imageView->Shutdown(this);
        m_imageViewOwner.Remove(handle);
    }

    RHI::Handle<RHI::BufferView> Context::CreateBufferView(RHI::Handle<RHI::Buffer> bufferHandle, const RHI::BufferAttachmentUseInfo& useInfo)
    {
        auto [handle, bufferView] = m_bufferViewOwner.InsertZerod();
        auto result = bufferView.Init(this, bufferHandle, useInfo);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return handle;
    }

    void Context::DestroyBufferView(RHI::Handle<RHI::BufferView> handle)
    {
        auto bufferView = m_bufferViewOwner.Get(handle);
        bufferView->Shutdown(this);
    }

    VkSemaphore Context::CreateSemaphore()
    {
        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        VkSemaphore semaphore = VK_NULL_HANDLE;
        auto result = vkCreateSemaphore(m_device, &createInfo, nullptr, &semaphore);
        VULKAN_ASSERT_SUCCESS(result);
        return semaphore;
    }

    void Context::FreeSemaphore(VkSemaphore semaphore)
    {
        vkDestroySemaphore(m_device, semaphore, nullptr);
    }

    VkFence Context::CreateFence()
    {
        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        VkFence fence = VK_NULL_HANDLE;
        auto result = vkCreateFence(m_device, &createInfo, nullptr, &fence);
        VULKAN_ASSERT_SUCCESS(result);
        return fence;
    }

    void Context::FreeFence(VkFence fence)
    {
        vkDestroyFence(m_device, fence, nullptr);
    }

    std::vector<VkLayerProperties> Context::GetAvailableInstanceLayerExtensions() const
    {
        uint32_t instanceLayerCount;
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> layers;
        layers.resize(instanceLayerCount);
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, layers.data());
        return layers;
    }

    std::vector<VkExtensionProperties> Context::GetAvailableInstanceExtensions() const
    {
        uint32_t instanceExtensionsCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, nullptr);
        std::vector<VkExtensionProperties> extensions;
        extensions.resize(instanceExtensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, extensions.data());
        return extensions;
    }

    std::vector<VkLayerProperties> Context::GetAvailableDeviceLayerExtensions(VkPhysicalDevice physicalDevice) const
    {
        uint32_t instanceLayerCount;
        vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> layers;
        layers.resize(instanceLayerCount);
        vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, layers.data());
        return layers;
    }

    std::vector<VkExtensionProperties> Context::GetAvailableDeviceExtensions(VkPhysicalDevice physicalDevice) const
    {
        uint32_t extensionsCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr);
        std::vector<VkExtensionProperties> extnesions;
        extnesions.resize(extensionsCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, extnesions.data());
        return extnesions;
    }

    std::vector<VkPhysicalDevice> Context::GetAvailablePhysicalDevices() const
    {
        uint32_t physicalDeviceCount;
        vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
        VkResult result = vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());
        RHI_ASSERT(result == VK_SUCCESS);
        return physicalDevices;
    }

    std::vector<VkQueueFamilyProperties> Context::GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice) const
    {
        uint32_t queueFamilyPropertiesCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties{};
        queueFamilyProperties.resize(queueFamilyPropertiesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());
        return queueFamilyProperties;
    }

    uint32_t Context::GetMemoryTypeIndex(RHI::MemoryType memoryType)
    {
        VkMemoryPropertyFlags flags = 0;
        VkMemoryPropertyFlags negateFlags = 0;
        switch (memoryType)
        {
        case RHI::MemoryType::CPU: flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; break;
        case RHI::MemoryType::GPULocal:
            flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            negateFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
        case RHI::MemoryType::GPUShared: flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; break;
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

} // namespace Vulkan