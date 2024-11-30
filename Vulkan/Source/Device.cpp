
#if RHI_PLATFORM_WINDOWS
    #define VK_USE_PLATFORM_WIN32_KHR
    #define VULKAN_SURFACE_OS_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
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

#include "RHI-Vulkan/Loader.hpp"

#include "Common.hpp"
#include "CommandPool.hpp"
#include "CommandList.hpp"
#include "Swapchain.hpp"
#include "Device.hpp"
#include "Queue.hpp"
#include "StagingBuffer.hpp"

#include <TL/Assert.hpp>
#include <TL/Log.hpp>
#include <TL/Allocator/Allocator.hpp>

#include <tracy/Tracy.hpp>

#include <format>

#define VULKAN_DEVICE_FUNC_LOAD(device, proc) reinterpret_cast<PFN_##proc>(vkGetDeviceProcAddr(device, #proc));
#define VULKAN_INSTANCE_FUNC_LOAD(instance, proc) reinterpret_cast<PFN_##proc>(vkGetInstanceProcAddr(instance, #proc));

namespace RHI
{
    Device* CreateVulkanDevice(const ApplicationInfo& appInfo)
    {
        ZoneScoped;
        auto device = new Vulkan::IDevice();
        auto result = device->Init(appInfo);
        TL_ASSERT(IsSuccess(result));
        return std::move(device);
    }

    void DestroyVulkanDevice(Device* _device)
    {
        auto device = (Vulkan::IDevice*)_device;
        device->Shutdown();
        delete device;
    }
} // namespace RHI

namespace RHI::Vulkan
{
    /// @todo: add support for a custom sink, so vulkan errors are spereated
    VkBool32 DebugMessengerCallbacks(
        [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
        [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        [[maybe_unused]] void*                                       pUserData)
    {
        // Collect additional information for logging
        TL::String additionalInfo;

        // Log the object names, debug markers, and queue information if available
        if (pCallbackData->objectCount > 0)
        {
            for (uint32_t i = 0; i < pCallbackData->objectCount; ++i)
            {
                additionalInfo += std::format(
                    "Object[{}]: Type: {}, Name: {}\n",
                    i,
                    ObjectTypeToName(pCallbackData->pObjects[i].objectType),
                    pCallbackData->pObjects[i].pObjectName ? pCallbackData->pObjects[i].pObjectName : "Unnamed");
            }
        }

        // Log any labels from the active debug marker stack
        if (pCallbackData->cmdBufLabelCount > 0)
        {
            additionalInfo += "Active Debug Markers:\n";
            for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i)
            {
                additionalInfo += std::format(
                    "Label[{}]: {} (color: [{:.2f}, {:.2f}, {:.2f}, {:.2f}])\n",
                    i,
                    pCallbackData->pCmdBufLabels[i].pLabelName ? pCallbackData->pCmdBufLabels[i].pLabelName : "Unnamed",
                    pCallbackData->pCmdBufLabels[i].color[0],
                    pCallbackData->pCmdBufLabels[i].color[1],
                    pCallbackData->pCmdBufLabels[i].color[2],
                    pCallbackData->pCmdBufLabels[i].color[3]);
            }
        }

        // Log queue information if available
        if (pCallbackData->queueLabelCount > 0)
        {
            additionalInfo += "Queue Labels:\n";
            for (uint32_t i = 0; i < pCallbackData->queueLabelCount; ++i)
            {
                additionalInfo += std::format(
                    "Queue[{}]: {} (color: [{:.2f}, {:.2f}, {:.2f}, {:.2f}])\n",
                    i,
                    pCallbackData->pQueueLabels[i].pLabelName ? pCallbackData->pQueueLabels[i].pLabelName : "Unnamed",
                    pCallbackData->pQueueLabels[i].color[0],
                    pCallbackData->pQueueLabels[i].color[1],
                    pCallbackData->pQueueLabels[i].color[2],
                    pCallbackData->pQueueLabels[i].color[3]);
            }
        }

        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            TL_LOG_INFO("{}\nMessage: {}", additionalInfo, pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: TL_LOG_INFO("{}\nMessage: {}", additionalInfo, pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            TL_LOG_WARNNING("{}\nMessage: {}", additionalInfo, pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: TL_LOG_ERROR("{}\nMessage: {}", additionalInfo, pCallbackData->pMessage); break;
        default:                                            TL_UNREACHABLE();
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
    {
        m_queue[(int)QueueType::Graphics] = TL::CreatePtr<IQueue>();
        m_queue[(int)QueueType::Compute]  = TL::CreatePtr<IQueue>();
        m_queue[(int)QueueType::Transfer] = TL::CreatePtr<IQueue>();
        m_destroyQueue                    = TL::CreatePtr<DeleteQueue>();
        m_bindGroupAllocator              = TL::CreatePtr<BindGroupAllocator>();
        m_commandsAllocator               = TL::CreatePtr<CommandAllocator>();
        m_stagingAllocator                = TL::CreatePtr<StagingBufferAllocator>();
    }

    IDevice::~IDevice() = default;

    ResultCode IDevice::Init(const ApplicationInfo& appInfo)
    {
        ZoneScoped;

        uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t transferQueueFamilyIndex = UINT32_MAX;
        uint32_t computeQueueFamilyIndex  = UINT32_MAX;

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

            auto appVersion    = appInfo.applicationVersion;
            auto engineVersion = appInfo.engineVersion;

            VkApplicationInfo applicationInfo{
                .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pNext              = nullptr,
                .pApplicationName   = appInfo.applicationName,
                .applicationVersion = VK_MAKE_API_VERSION(0, appVersion.major, appVersion.minor, appVersion.patch),
                .pEngineName        = appInfo.engineName,
                .engineVersion      = VK_MAKE_API_VERSION(0, engineVersion.major, engineVersion.minor, engineVersion.patch),
                .apiVersion         = VK_API_VERSION_1_3,
            };

#if RHI_DEBUG
            VkDebugUtilsMessengerCreateInfoEXT debugUtilsCI{
                .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .flags           = 0,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
                .pfnUserCallback = DebugMessengerCallbacks,
                .pUserData       = this,
            };

            bool debugExtensionEnabled = false;

            for (VkExtensionProperties extension : GetAvailableInstanceExtensions())
            {
                auto extensionName = extension.extensionName;
                if (strcmp(extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
                {
                    debugExtensionEnabled = true;
                }
            }

            if (debugExtensionEnabled)
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
                .pNext = debugExtensionEnabled ? &debugUtilsCI : nullptr,
#else
                .pNext = nullptr,
#endif
                .flags = {}, .pApplicationInfo = &applicationInfo, .enabledLayerCount = static_cast<uint32_t>(layers.size()),
                .ppEnabledLayerNames = layers.data(), .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
                .ppEnabledExtensionNames = extensions.data(),
            };
            result = vkCreateInstance(&instanceCI, nullptr, &m_instance);
            if (result != VK_SUCCESS) return ConvertResult(result);

#if RHI_DEBUG
            if (debugExtensionEnabled)
            {
                m_pfn.m_vkCreateDebugUtilsMessengerEXT  = VULKAN_INSTANCE_FUNC_LOAD(m_instance, vkCreateDebugUtilsMessengerEXT);
                m_pfn.m_vkDestroyDebugUtilsMessengerEXT = VULKAN_INSTANCE_FUNC_LOAD(m_instance, vkDestroyDebugUtilsMessengerEXT);
                result                                  = m_pfn.m_vkCreateDebugUtilsMessengerEXT(m_instance, &debugUtilsCI, nullptr, &m_debugUtilsMessenger);
                if (result != VK_SUCCESS) return ConvertResult(result);
            }
#endif

            for (VkPhysicalDevice physicalDevice : GetAvailablePhysicalDevices(m_instance))
            {
                bool swapchainExtension           = false;
                bool dynamicRenderingExtension    = false;
                bool maintenance2Extension        = false;
                bool multiviewExtension           = false;
                bool createRenderpass2Extension   = false;
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

                if (swapchainExtension && dynamicRenderingExtension && maintenance2Extension && multiviewExtension &&
                    createRenderpass2Extension && depthStencilResolveExtension)
                {
                    m_physicalDevice = physicalDevice;
                    break;
                }
            }

            TL::Vector<const char*> deviceLayerNames = {

            };

            TL::Vector<const char*> deviceExtensionNames = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME};

            auto queueFamilyProperties = GetPhysicalDeviceQueueFamilyProperties(m_physicalDevice);
            for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyProperties.size(); queueFamilyIndex++)
            {
                auto queueFamilyProperty = queueFamilyProperties[queueFamilyIndex];

                // Search for main queue that should be able to do all work (graphics, compute and transfer)
                if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    graphicsQueueFamilyIndex = queueFamilyIndex;
                    transferQueueFamilyIndex = queueFamilyIndex;
                    computeQueueFamilyIndex  = queueFamilyIndex;
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

            float                               queuePriority = 1.0f;
            TL::Vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

            VkDeviceQueueCreateInfo queueCI{
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext            = nullptr,
                .flags            = 0,
                .queueCount       = 1,
                .pQueuePriorities = &queuePriority,
            };

            if (graphicsQueueFamilyIndex != UINT32_MAX)
            {
                queueCI.queueFamilyIndex = graphicsQueueFamilyIndex;
                queueCreateInfos.push_back(queueCI);
            }
            else if (computeQueueFamilyIndex != UINT32_MAX)
            {
                queueCI.queueFamilyIndex = computeQueueFamilyIndex;
                queueCreateInfos.push_back(queueCI);
            }
            else if (transferQueueFamilyIndex != UINT32_MAX)
            {
                queueCI.queueFamilyIndex = transferQueueFamilyIndex;
                queueCreateInfos.push_back(queueCI);
            }
            else
            {
                TL_UNREACHABLE();
            }

            VkPhysicalDeviceVulkan13Features features13{
                .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
                .pNext            = nullptr,
                .synchronization2 = VK_TRUE,
                .dynamicRendering = VK_TRUE,
            };
            VkPhysicalDeviceVulkan12Features features12{
                .sType                                    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                .pNext                                    = &features13,
                .descriptorIndexing                       = VK_TRUE,
                .descriptorBindingPartiallyBound          = VK_TRUE,
                .descriptorBindingVariableDescriptorCount = VK_TRUE,
                .runtimeDescriptorArray                   = VK_TRUE,
                .timelineSemaphore                        = VK_TRUE,
            };
            VkPhysicalDeviceVulkan11Features features11{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
                .pNext = &features12,
            };
            VkPhysicalDeviceFeatures2 features{
                .sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext    = &features11,
                .features = {.samplerAnisotropy = VK_TRUE},
            };

            VkDeviceCreateInfo deviceCI{
                .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext                   = &features,
                .flags                   = 0,
                .queueCreateInfoCount    = (uint32_t)queueCreateInfos.size(),
                .pQueueCreateInfos       = queueCreateInfos.data(),
                .enabledLayerCount       = (uint32_t)deviceLayerNames.size(),
                .ppEnabledLayerNames     = deviceLayerNames.data(),
                .enabledExtensionCount   = (uint32_t)deviceExtensionNames.size(),
                .ppEnabledExtensionNames = deviceExtensionNames.data(),
                .pEnabledFeatures        = nullptr,
            };

            result = vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device);
            if (result != VK_SUCCESS) return ConvertResult(result);

            VmaAllocatorCreateInfo vmaCI{
                .physicalDevice   = m_physicalDevice,
                .device           = m_device,
                .instance         = m_instance,
                .vulkanApiVersion = VK_API_VERSION_1_3,
            };

            result = vmaCreateAllocator(&vmaCI, &m_allocator);
            if (result != VK_SUCCESS) return ConvertResult(result);

#if RHI_DEBUG
            if (m_pfn.m_vkCreateDebugUtilsMessengerEXT)
            {
                m_pfn.m_vkCmdBeginDebugUtilsLabelEXT    = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdBeginDebugUtilsLabelEXT);
                m_pfn.m_vkCmdEndDebugUtilsLabelEXT      = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdEndDebugUtilsLabelEXT);
                m_pfn.m_vkCmdInsertDebugUtilsLabelEXT   = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdInsertDebugUtilsLabelEXT);
                m_pfn.m_vkQueueBeginDebugUtilsLabelEXT  = VULKAN_DEVICE_FUNC_LOAD(m_device, vkQueueBeginDebugUtilsLabelEXT);
                m_pfn.m_vkQueueEndDebugUtilsLabelEXT    = VULKAN_DEVICE_FUNC_LOAD(m_device, vkQueueEndDebugUtilsLabelEXT);
                m_pfn.m_vkQueueInsertDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkQueueInsertDebugUtilsLabelEXT);
                m_pfn.m_vkSetDebugUtilsObjectNameEXT    = VULKAN_DEVICE_FUNC_LOAD(m_device, vkSetDebugUtilsObjectNameEXT);
                m_pfn.m_vkSetDebugUtilsObjectTagEXT     = VULKAN_DEVICE_FUNC_LOAD(m_device, vkSetDebugUtilsObjectTagEXT);
                m_pfn.m_vkSubmitDebugUtilsMessageEXT    = VULKAN_INSTANCE_FUNC_LOAD(m_instance, vkSubmitDebugUtilsMessageEXT);
            }
#endif
            m_pfn.m_vkCmdBeginConditionalRenderingEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdBeginConditionalRenderingEXT);
            m_pfn.m_vkCmdEndConditionalRenderingEXT   = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdEndConditionalRenderingEXT);

            // return result;
        }

        {
            m_limits = TL::CreatePtr<DeviceLimits>();
        }

        ResultCode resultCode;

        resultCode = m_queue[(uint32_t)QueueType::Graphics]->Init(this, graphicsQueueFamilyIndex, 0);
        if (IsError(resultCode)) return resultCode;

        resultCode = m_queue[(uint32_t)QueueType::Compute]->Init(this, computeQueueFamilyIndex, 0);
        if (IsError(resultCode)) return resultCode;

        resultCode = m_queue[(uint32_t)QueueType::Transfer]->Init(this, transferQueueFamilyIndex, 0);
        if (IsError(resultCode)) return resultCode;

        resultCode = m_stagingAllocator->Init(this);
        if (IsError(resultCode)) return resultCode;

        resultCode = m_commandsAllocator->Init(this);
        if (IsError(resultCode)) return resultCode;

        resultCode = m_bindGroupAllocator->Init(this);
        if (IsError(resultCode)) return resultCode;

        resultCode = m_destroyQueue->Init(this);
        if (IsError(resultCode)) return resultCode;

        VkSemaphoreTypeCreateInfo typeCreateInfo{
            .sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext         = nullptr,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue  = m_timelineValue,
        };
        VkSemaphoreCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &typeCreateInfo,
            .flags = 0,
        };

        auto result = vkCreateSemaphore(m_device, &createInfo, nullptr, &m_timelineSemaphore);
        return ConvertResult(result);
    }

    void IDevice::Shutdown()
    {
        vkDeviceWaitIdle(m_device);

        for (auto& queue : m_queue)
        {
            queue->Shutdown();
        }

        if (auto count = m_imageOwner.ReportLiveResourcesCount(); count != 0)
        {
            TL_LOG_WARNNING("Detected {} Image leaked", count);
        }
        if (auto count = m_bufferOwner.ReportLiveResourcesCount(); count != 0)
        {
            TL_LOG_WARNNING("Detected {} Buffer leaked", count);
        }
        if (auto count = m_bindGroupLayoutsOwner.ReportLiveResourcesCount(); count != 0)
        {
            TL_LOG_WARNNING("Detected {} BindGroupLayout leaked", count);
        }
        if (auto count = m_bindGroupOwner.ReportLiveResourcesCount(); count != 0)
        {
            TL_LOG_WARNNING("Detected {} BindGroup leaked", count);
        }
        if (auto count = m_pipelineLayoutOwner.ReportLiveResourcesCount(); count != 0)
        {
            TL_LOG_WARNNING("Detected {} PipelineLayout leaked", count);
        }
        if (auto count = m_graphicsPipelineOwner.ReportLiveResourcesCount(); count != 0)
        {
            TL_LOG_WARNNING("Detected {} GraphicsPipeline leaked", count);
        }
        if (auto count = m_computePipelineOwner.ReportLiveResourcesCount(); count != 0)
        {
            TL_LOG_WARNNING("Detected {} ComputePipeline leaked", count);
        }
        if (auto count = m_samplerOwner.ReportLiveResourcesCount(); count != 0)
        {
            TL_LOG_WARNNING("Detected {} Sampler leaked", count);
        }

        m_stagingAllocator->Shutdown();
        m_commandsAllocator->Shutdown();
        m_bindGroupAllocator->Shutdown();
        m_destroyQueue->Shutdown();

        vkDestroySemaphore(m_device, m_timelineSemaphore, 0);
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

    void IDevice::SetDebugName(VkObjectType type, uint64_t handle, const char* name) const
    {
        if (handle == 0 /* VK_NULL_HANDLE */) return;

        if (auto fn = m_pfn.m_vkSetDebugUtilsObjectNameEXT; fn && name)
        {
            VkDebugUtilsObjectNameInfoEXT nameInfo{
                .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext        = nullptr,
                .objectType   = type,
                .objectHandle = handle,
                .pObjectName  = name,
            };
            fn(m_device, &nameInfo);
        }
    }

    uint64_t IDevice::GetTimelineValue() const
    {
        uint64_t value  = 0;
        VkResult result = vkGetSemaphoreCounterValue(m_device, m_timelineSemaphore, &value);
        TL_ASSERT(result == VK_SUCCESS);
        return value;
    }

    uint64_t IDevice::GetPendingTimelineValue() const
    {
        return m_timelineValue.load(std::memory_order_relaxed);
    }

    VkSemaphore IDevice::GetTimelineSemaphore() const
    {
        return m_timelineSemaphore;
    }

    uint64_t IDevice::AdvanceTimeline()
    {
        return {};
    }

    Swapchain* IDevice::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        auto swapchain = new ISwapchain();
        auto result    = swapchain->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        return swapchain;
    }

    void IDevice::DestroySwapchain(Swapchain* _swapchain)
    {
        auto swapchain = (ISwapchain*)_swapchain;
        swapchain->Shutdown();
        delete swapchain;
    }

    TL::Ptr<ShaderModule> IDevice::CreateShaderModule(const ShaderModuleCreateInfo& createInfo)
    {
        auto shaderModule = TL::CreatePtr<IShaderModule>();
        auto result       = shaderModule->Init(this, createInfo.code);
        TL_ASSERT(IsSuccess(result));
        return shaderModule;
    }

    CommandList* IDevice::CreateCommandList(const CommandListCreateInfo& createInfo)
    {
        // TODO: change this
        return m_commandsAllocator->AllocateCommandList(createInfo.queueType).release();
    }

    Handle<BindGroupLayout> IDevice::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        auto [handle, result] = m_bindGroupLayoutsOwner.Create(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        return handle;
    }

    void IDevice::DestroyBindGroupLayout(Handle<BindGroupLayout> handle)
    {
        m_destroyQueue->Push(
            GetPendingTimelineValue(),
            [handle](IDevice* device)
        {
            device->m_bindGroupLayoutsOwner.Destroy(handle, device);
        });
    }

    Handle<BindGroup> IDevice::CreateBindGroup(Handle<BindGroupLayout> handle)
    {
        auto [bindGroupHandle, result] = m_bindGroupOwner.Create(this, handle);
        TL_ASSERT(IsSuccess(result));
        return bindGroupHandle;
    }

    void IDevice::DestroyBindGroup(Handle<BindGroup> handle)
    {
        m_destroyQueue->Push(
            GetPendingTimelineValue(),
            [handle](IDevice* device)
        {
            device->m_bindGroupOwner.Destroy(handle, device);
        });
    }

    void IDevice::UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo)
    {
        auto bindGroup = m_bindGroupOwner.Get(handle);
        bindGroup->Write(this, updateInfo);
    }

    Handle<PipelineLayout> IDevice::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        auto [handle, result] = m_pipelineLayoutOwner.Create(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        return handle;
    }

    void IDevice::DestroyPipelineLayout(Handle<PipelineLayout> handle)
    {
        m_destroyQueue->Push(
            GetPendingTimelineValue(),
            [handle](IDevice* device)
        {
            device->m_pipelineLayoutOwner.Destroy(handle, device);
        });
    }

    Handle<GraphicsPipeline> IDevice::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        auto [handle, result] = m_graphicsPipelineOwner.Create(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        return handle;
    }

    void IDevice::DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)
    {
        m_destroyQueue->Push(
            GetPendingTimelineValue(),
            [handle](IDevice* device)
        {
            device->m_graphicsPipelineOwner.Destroy(handle, device);
        });
    }

    Handle<ComputePipeline> IDevice::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        auto [handle, result] = m_computePipelineOwner.Create(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        return handle;
    }

    void IDevice::DestroyComputePipeline(Handle<ComputePipeline> handle)
    {
        m_destroyQueue->Push(
            GetPendingTimelineValue(),
            [handle](IDevice* device)
        {
            device->m_computePipelineOwner.Destroy(handle, device);
        });
    }

    Handle<Sampler> IDevice::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        auto [handle, result] = m_samplerOwner.Create(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        return handle;
    }

    void IDevice::DestroySampler(Handle<Sampler> handle)
    {
        m_destroyQueue->Push(
            GetPendingTimelineValue(),
            [handle](IDevice* device)
        {
            device->m_samplerOwner.Destroy(handle, device);
        });
    }

    Result<Handle<Image>> IDevice::CreateImage(const ImageCreateInfo& createInfo)
    {
        auto [handle, result] = m_imageOwner.Create(this, createInfo);
        if (IsSuccess(result)) return (Handle<Image>)handle;
        return result;
    }

    void IDevice::DestroyImage(Handle<Image> handle)
    {
        m_destroyQueue->Push(
            GetPendingTimelineValue(),
            [handle](IDevice* device)
        {
            device->m_imageOwner.Destroy(handle, device);
        });
    }

    Result<Handle<Buffer>> IDevice::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        auto [handle, result] = m_bufferOwner.Create(this, createInfo);
        if (IsSuccess(result)) return (Handle<Buffer>)handle;
        return result;
    }

    void IDevice::DestroyBuffer(Handle<Buffer> handle)
    {
        m_destroyQueue->Push(
            GetPendingTimelineValue(),
            [handle](IDevice* device)
        {
            device->m_bufferOwner.Destroy(handle, device);
        });
    }

    DeviceMemoryPtr IDevice::MapBuffer(Handle<Buffer> handle)
    {
        auto            resource   = m_bufferOwner.Get(handle);
        auto            allocation = resource->allocation.handle;
        DeviceMemoryPtr memoryPtr;
        Validate(vmaMapMemory(m_allocator, allocation, &memoryPtr));
        return memoryPtr;
    }

    void IDevice::UnmapBuffer(Handle<Buffer> handle)
    {
        auto resource = m_bufferOwner.Get(handle)->allocation.handle;
        vmaUnmapMemory(m_allocator, resource);
    }

    void IDevice::QueueBeginLabel(QueueType type, const char* name, float color[4])
    {
        m_queue[(int)type]->BeginLabel(name, color);
    }

    void IDevice::QueueEndLabel(QueueType type)
    {
        m_queue[(int)type]->EndLabel();
    }

    uint64_t IDevice::QueueSubmit(const SubmitInfo& submitInfo)
    {
        m_queue[(int)QueueType::Graphics]->Submit(submitInfo);

        // for (auto commandList : submitInfo.commandLists)
        // {
        //     delete commandList;
        // }

        return GetPendingTimelineValue();
    }

    StagingBuffer IDevice::StagingAllocate(size_t size)
    {
        return m_stagingAllocator->Allocate(size);
    }

    uint64_t IDevice::UploadImage(const ImageUploadInfo& uploadInfo)
    {
        auto image = m_imageOwner.Get(uploadInfo.image);

        VkImageSubresourceRange subresourceRange{
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = uploadInfo.baseMipLevel,
            .levelCount     = uploadInfo.levelCount,
            .baseArrayLayer = uploadInfo.baseArrayLayer,
            .layerCount     = uploadInfo.layerCount,
        };

        auto _commandList = CreateCommandList({.queueType = QueueType::Transfer});
        auto commandList  = (ICommandList*)_commandList;

        commandList->Begin();
        commandList->PipelineBarrier({
            .imageBarriers = {{
                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext               = nullptr,
                .srcStageMask        = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                .srcAccessMask       = VK_ACCESS_2_NONE,
                .dstStageMask        = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .dstAccessMask       = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image               = image->handle,
                .subresourceRange    = subresourceRange,
            }},
        });

        commandList->CopyBufferToImage({
            .image = uploadInfo.image,
            .subresource =
                {
                    .imageAspects = ImageAspect::Color,
                    .mipLevel     = uploadInfo.baseMipLevel,
                    .arrayBase    = uploadInfo.baseArrayLayer,
                    .arrayCount   = uploadInfo.layerCount,
                },
            .imageSize    = image->size,
            .imageOffset  = {0, 0, 0},
            .buffer       = uploadInfo.srcBuffer,
            .bufferOffset = uploadInfo.srcBufferOffset,
            .bufferSize   = uploadInfo.sizeBytes,

        });
        commandList->PipelineBarrier({
            .imageBarriers = {{
                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext               = nullptr,
                .srcStageMask        = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .srcAccessMask       = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .dstStageMask        = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                .dstAccessMask       = VK_ACCESS_2_NONE,
                .oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image               = image->handle,
                .subresourceRange    = subresourceRange,
            }},
        });
        commandList->End();

        return m_queue[(int)QueueType::Transfer]->Submit({.commandLists = commandList});
    }

    void IDevice::CollectResources()
    {
        m_destroyQueue->DestroyObjects(false);
        m_stagingAllocator->ReleaseAll();
    }

    void IDevice::WaitTimelineValue(uint64_t value)
    {
        VkSemaphoreWaitInfo waitInfo{
            .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .pNext          = nullptr,
            .flags          = 0,
            .semaphoreCount = 1,
            .pSemaphores    = &m_timelineSemaphore,
            .pValues        = &value,
        };
        vkWaitSemaphores(m_device, &waitInfo, UINT64_MAX);
    }

} // namespace RHI::Vulkan