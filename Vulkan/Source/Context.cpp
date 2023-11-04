#define VK_USE_PLATFORM_WIN32_KHR
#include "RHI-Vulkan/Loader.hpp"

#include "Common.hpp"
#include "Context.hpp"
#include "FrameGraph.hpp"
#include "Resources.hpp"

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

// todo: make this a cmake build option
#define RHI_DEBUG 1

std::unique_ptr<RHI::Context> RHI::CreateVulkanRHI(const RHI::ApplicationInfo& appInfo, std::unique_ptr<RHI::DebugCallbacks> debugCallbacks)
{
    auto context = std::make_unique<Vulkan::Context>();
    auto result = context->Init(appInfo, std::move(debugCallbacks));
    RHI_ASSERT(result == VK_SUCCESS);
    return context;
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

    Context::Context()
        : m_resourceManager(std::make_unique<ResourceManager>(this))
    {
    }

    Context::~Context()
    {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        vmaDestroyAllocator(m_allocator);
        vkDestroyInstance(m_instance, nullptr);
    }

    VkResult Context::Init(const RHI::ApplicationInfo& appInfo, std::unique_ptr<RHI::DebugCallbacks> debugCallbacks)
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

        uint32_t version = VK_MAKE_VERSION(0, 1, 0);

        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pNext = nullptr;
        applicationInfo.pApplicationName = appInfo.applicationName.c_str();
        applicationInfo.applicationVersion = version;
        applicationInfo.pEngineName = "AAMS Renderer Hardware Engine (RHI)";
        applicationInfo.engineVersion = version;
        applicationInfo.apiVersion = VK_API_VERSION_1_3;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugMessengerCallbacks;
        debugCreateInfo.pUserData = debugCallbacks.get();

        bool debugExtensionFound = false;

        for (VkExtensionProperties extension : GetAvailableInstanceExtensions())
        {
            auto extensionName = extension.extensionName;
#if RHI_DEBUG
            if (!strcmp(extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
            {
                debugExtensionFound = true;
                continue;
            }
#endif
        }

#if RHI_DEBUG
        if (debugExtensionFound)
            enabledExtensionsNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        else
            debugCallbacks->LogWarnning("RHI Vulkan: Debug extension not present.\n Vulkan layer validation is disabled.");
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
        RHIVK_RETURN_VKERR_CODE(result);

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
                if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
                {
                    swapchainExtension = true;
                }
                else if (strcmp(extension.extensionName, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
                {
                    dynamicRenderingExtension = true;
                }
                else if (strcmp(extension.extensionName, VK_KHR_MAINTENANCE2_EXTENSION_NAME) == 0)
                {
                    maintenance2Extension = true;
                }
                else if (strcmp(extension.extensionName, VK_KHR_MULTIVIEW_EXTENSION_NAME) == 0)
                {
                    multiviewExtension = true;
                }
                else if (strcmp(extension.extensionName, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME) == 0)
                {
                    createRenderpass2Extension = true;
                }
                else if (strcmp(extension.extensionName, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME) == 0)
                {
                    depthStencilResolveExtension = true;
                }
            }

            if (swapchainExtension && dynamicRenderingExtension && maintenance2Extension && multiviewExtension &&
                createRenderpass2Extension && depthStencilResolveExtension)
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
                // #if RHI_DEBUG
                // VK_EXT_DEBUG_MARKER_EXTENSION_NAME
                // #endif
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
                    m_transferQueueFamilyIndex = queueFamilyIndex;

                // Search for transfer queue
                if ((queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT && (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
                    m_computeQueueFamilyIndex = queueFamilyIndex;
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
            RHIVK_RETURN_VKERR_CODE(result);

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
            RHIVK_RETURN_VKERR_CODE(result);
        }

        // load function pointers
        m_vkCmdDebugMarkerBeginEXT = reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetDeviceProcAddr(m_device, "vkCmdDebugMarkerBeginEXT"));
        m_vkCmdDebugMarkerInsertEXT = reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(vkGetDeviceProcAddr(m_device, "vkCmdDebugMarkerInsertEXT"));
        m_vkCmdDebugMarkerEndEXT = reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetDeviceProcAddr(m_device, "vkCmdDebugMarkerEndEXT"));

        // RHI_ASSERT(m_vkCmdDebugMarkerBeginEXT != nullptr);
        // RHI_ASSERT(m_vkCmdDebugMarkerInsertEXT != nullptr);
        // RHI_ASSERT(m_vkCmdDebugMarkerEndEXT != nullptr);

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

    std::unique_ptr<RHI::ResourcePool> Context::CreateResourcePool(const RHI::ResourcePoolCreateInfo& createInfo)
    {
        auto resourcePool = std::make_unique<ResourcePool>(this);
        auto result = resourcePool->Init(createInfo);
        RHI_ASSERT(result == VK_SUCCESS);
        return resourcePool;
    }

    RHI::Handle<RHI::GraphicsPipeline> Context::CreateGraphicsPipeline(const RHI::GraphicsPipelineCreateInfo& createInfo)
    {
        auto [handle, result] = m_resourceManager->CreateGraphicsPipeline(createInfo);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return handle;
    }

    RHI::Handle<RHI::ComputePipeline> Context::CreateComputePipeline(const RHI::ComputePipelineCreateInfo& createInfo)
    {
        auto [handle, result] = m_resourceManager->CreateComputePipeline(createInfo);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return handle;
    }

    RHI::Handle<RHI::Sampler> Context::CreateSampler(const RHI::SamplerCreateInfo& createInfo)
    {
        auto [handle, result] = m_resourceManager->CreateSampler(createInfo);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return handle;
    }

    std::unique_ptr<RHI::FrameScheduler> Context::CreateFrameScheduler()
    {
        auto scheduler = std::make_unique<FrameScheduler>(this);
        auto result = scheduler->Init();
        RHI_ASSERT(result == VK_SUCCESS);
        return scheduler;
    }

    std::unique_ptr<RHI::ShaderBindGroupAllocator> Context::CreateShaderBindGroupAllocator()
    {
        auto allocator = std::make_unique<ShaderBindGroupAllocator>(this);
        auto result = allocator->Init();
        RHI_ASSERT(result == VK_SUCCESS);
        return allocator;
    }

    RHI::DeviceMemoryPtr Context::MapResource(RHI::Handle<RHI::Image> image)
    {
        auto resource = m_resourceManager->m_imageOwner.Get(image);
        RHI_ASSERT(resource);

        auto allocation = resource->allocation.handle;

        RHI::DeviceMemoryPtr memoryPtr = nullptr;
        VkResult result = vmaMapMemory(m_allocator, allocation, &memoryPtr);
        RHI_ASSERT(result == VK_SUCCESS);
        return memoryPtr;
    }

    RHI::DeviceMemoryPtr Context::MapResource(RHI::Handle<RHI::Buffer> buffer)
    {
        auto resource = m_resourceManager->m_bufferOwner.Get(buffer);
        RHI_ASSERT(resource);

        auto allocation = resource->allocation.handle;

        RHI::DeviceMemoryPtr memoryPtr = nullptr;
        VkResult result = vmaMapMemory(m_allocator, allocation, &memoryPtr);
        RHI_ASSERT(result == VK_SUCCESS);
        return memoryPtr;
    }

    void Context::Unmap(RHI::Handle<RHI::Image> image)
    {
        auto resource = m_resourceManager->m_bufferOwner.Get(image)->allocation.handle;

        vmaUnmapMemory(m_allocator, resource);
    }

    void Context::Unmap(RHI::Handle<RHI::Buffer> buffer)
    {
        auto resource = m_resourceManager->m_bufferOwner.Get(buffer)->allocation.handle;

        vmaUnmapMemory(m_allocator, resource);
    }

    void Context::Free(RHI::Handle<RHI::GraphicsPipeline> pso)
    {
        m_resourceManager->FreeGraphicsPipeline(RHI::Handle<GraphicsPipeline>(pso));
    }

    void Context::Free(RHI::Handle<RHI::ComputePipeline> pso)
    {
        m_resourceManager->FreeComputePipeline(RHI::Handle<ComputePipeline>(pso));
    }

    void Context::Free(RHI::Handle<RHI::Sampler> sampler)
    {
        m_resourceManager->FreeSampler(RHI::Handle<Sampler>(sampler));
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

} // namespace Vulkan