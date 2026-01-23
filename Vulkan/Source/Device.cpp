#define VMA_IMPLEMENTATION

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

#include "Device.hpp"
#include "Common.hpp"
#include "Queue.hpp"
#include "Frame.hpp"
#include "RHI-Vulkan/Loader.hpp"

#include <TL/Log.hpp>
#include <TL/Assert.hpp>
#include <TL/Containers/Optional.hpp>
#include <TL/Allocator/Allocator.hpp>

#include <algorithm>
#include <format>

#include <tracy/Tracy.hpp>

#define VULKAN_DEVICE_FUNC_LOAD(device, proc) reinterpret_cast<PFN_##proc>(vkGetDeviceProcAddr(device, #proc));
#define VULKAN_INSTANCE_FUNC_LOAD(instance, proc) reinterpret_cast<PFN_##proc>(vkGetInstanceProcAddr(instance, #proc));

namespace RHI
{
    Device* CreateVulkanDevice(const ApplicationInfo& appInfo)
    {
        ZoneScoped;
        auto device = TL::construct<Vulkan::IDevice>();
        auto result = device->Init(appInfo);
        TL_ASSERT(IsSuccess(result));
        return device;
    }

    void DestroyVulkanDevice(Device* _device)
    {
        auto device = (Vulkan::IDevice*)_device;
        device->Shutdown();
        TL::destruct(device);
    }
} // namespace RHI

namespace RHI::Vulkan
{

    /// @todo: add support for a custom sink, so vulkan errors are spereated
    inline static VkBool32 DebugMessengerCallbacks(TL_MAYBE_UNUSED VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, TL_MAYBE_UNUSED VkDebugUtilsMessageTypeFlagsEXT messageTypes, TL_MAYBE_UNUSED const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, TL_MAYBE_UNUSED void* pUserData)
    {
        TL::String message = std::format("Vulkan Validation: {}\n", pCallbackData->pMessage);

        if (pCallbackData->objectCount > 0)
        {
            message += "Objects:\n";
            for (uint32_t i = 0; i < pCallbackData->objectCount; ++i)
            {
                message += std::format(
                    "  [{}] Type: {}, Name: {}\n",
                    i,
                    ObjectTypeToName(pCallbackData->pObjects[i].objectType),
                    pCallbackData->pObjects[i].pObjectName ? pCallbackData->pObjects[i].pObjectName : "Unnamed");
            }
        }

        if (pCallbackData->cmdBufLabelCount > 0)
        {
            message += "Debug Markers:\n";
            for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i)
            {
                message += std::format(
                    "  [{}] {} (color: [{:.2f}, {:.2f}, {:.2f}, {:.2f}])\n",
                    i,
                    pCallbackData->pCmdBufLabels[i].pLabelName ? pCallbackData->pCmdBufLabels[i].pLabelName : "Unnamed",
                    pCallbackData->pCmdBufLabels[i].color[0],
                    pCallbackData->pCmdBufLabels[i].color[1],
                    pCallbackData->pCmdBufLabels[i].color[2],
                    pCallbackData->pCmdBufLabels[i].color[3]);
            }
        }

        if (pCallbackData->queueLabelCount > 0)
        {
            message += "Queue Labels:\n";
            for (uint32_t i = 0; i < pCallbackData->queueLabelCount; ++i)
            {
                message += std::format(
                    "  [{}] {} (color: [{:.2f}, {:.2f}, {:.2f}, {:.2f}])\n",
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
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: TL_LOG_INFO("{}", message); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    TL_LOG_INFO("{}", message); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: TL_LOG_WARNNING("{}", message); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   TL_LOG_ERROR("{}", message); break;
        default:                                              TL_UNREACHABLE();
        }

        return VK_FALSE;
    }

    inline static TL::Vector<VkLayerProperties> GetAvailableInstanceLayerExtensions()
    {
        VulkanResult result;
        uint32_t     instanceLayerCount;
        result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        TL_ASSERT(result);
        TL::Vector<VkLayerProperties> layers;
        layers.resize(instanceLayerCount);
        result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, layers.data());
        TL_ASSERT(result);
        return layers;
    }

    inline static TL::Vector<VkExtensionProperties> GetAvailableInstanceExtensions()
    {
        VulkanResult result;
        uint32_t     instanceExtensionsCount;
        result = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, nullptr);
        TL_ASSERT(result);
        TL::Vector<VkExtensionProperties> extensions;
        extensions.resize(instanceExtensionsCount);
        result = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, extensions.data());
        TL_ASSERT(result);
        return extensions;
    }

    inline static TL::Vector<VkLayerProperties> GetAvailableDeviceLayerExtensions(VkPhysicalDevice physicalDevice)
    {
        VulkanResult result;
        uint32_t     instanceLayerCount;
        result = vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, nullptr);
        TL_ASSERT(result);
        TL::Vector<VkLayerProperties> layers;
        layers.resize(instanceLayerCount);
        result = vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, layers.data());
        TL_ASSERT(result);
        return layers;
    }

    inline static TL::Vector<VkExtensionProperties> GetAvailableDeviceExtensions(VkPhysicalDevice physicalDevice)
    {
        VulkanResult result;
        uint32_t     extensionsCount;
        result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr);
        TL_ASSERT(result);
        TL::Vector<VkExtensionProperties> extnesions;
        extnesions.resize(extensionsCount);
        result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, extnesions.data());
        TL_ASSERT(result);
        return extnesions;
    }

    inline static TL::Vector<VkPhysicalDevice> GetAvailablePhysicalDevices(VkInstance instance)
    {
        VulkanResult result;
        uint32_t     physicalDeviceCount;
        result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
        TL::Vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
        result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
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
        m_destroyQueue       = TL::CreatePtr<DeleteQueue>();
        m_bindGroupAllocator = TL::CreatePtr<BindGroupAllocator>();
    }

    IDevice::~IDevice() = default;

    ResultCode IDevice::Init(const ApplicationInfo& appInfo)
    {
        ZoneScoped;

        m_backend = BackendType::Vulkan1_3;

#if RHI_DEBUG
        constexpr bool DebugLayerEnabled = true;
#else
        constexpr bool DebugLayerEnabled = false;
#endif
        constexpr bool EnableAsyncQueues = true;

        TL::Map<TL::String, VkLayerProperties> availableInstanceLayers;
        for (VkLayerProperties layer : GetAvailableInstanceLayerExtensions())
        {
            availableInstanceLayers[layer.layerName] = layer;
        }

        TL::Map<TL::String, VkExtensionProperties> availableInstanceExtensions;
        for (VkExtensionProperties extension : GetAvailableInstanceExtensions())
        {
            availableInstanceExtensions[extension.extensionName] = extension;
        }

        TL::Vector<const char*> requiredInstanceLayers;
        TL::Vector<const char*> requiredInstanceExtensions{
            VK_KHR_SURFACE_EXTENSION_NAME,
            VULKAN_SURFACE_OS_EXTENSION_NAME,
        };

        if constexpr (DebugLayerEnabled)
        {
            if (availableInstanceLayers.contains("VK_LAYER_KHRONOS_validation"))
            {
                requiredInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
            }

            if (availableInstanceExtensions.contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
            {
                requiredInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            else
            {
                TL_LOG_WARNNING("RHI Vulkan: Debug extension not present.");
            }
        }

        VkApplicationInfo applicationInfo{
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext              = nullptr,
            .pApplicationName   = appInfo.applicationName,
            .applicationVersion = VK_MAKE_API_VERSION(0, appInfo.applicationVersion.major, appInfo.applicationVersion.minor, appInfo.applicationVersion.patch),
            .pEngineName        = appInfo.engineName,
            .engineVersion      = VK_MAKE_API_VERSION(0, appInfo.engineVersion.major, appInfo.engineVersion.minor, appInfo.engineVersion.patch),
            .apiVersion         = VK_API_VERSION_1_3,
        };

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsCI{
            .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .flags           = 0,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = DebugMessengerCallbacks,
            .pUserData       = this,
        };

        VulkanResult result;

        VkInstanceCreateInfo instanceCI{
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext                   = DebugLayerEnabled ? &debugUtilsCI : nullptr,
            .flags                   = {},
            .pApplicationInfo        = &applicationInfo,
            .enabledLayerCount       = static_cast<uint32_t>(requiredInstanceLayers.size()),
            .ppEnabledLayerNames     = requiredInstanceLayers.data(),
            .enabledExtensionCount   = static_cast<uint32_t>(requiredInstanceExtensions.size()),
            .ppEnabledExtensionNames = requiredInstanceExtensions.data(),
        };
        result = vkCreateInstance(&instanceCI, nullptr, &m_instance);
        if (!result)
        {
            Shutdown();
            return result;
        }

        if constexpr (DebugLayerEnabled)
        {
            if (availableInstanceExtensions.contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
            {
                m_pfn.m_vkCreateDebugUtilsMessengerEXT  = VULKAN_INSTANCE_FUNC_LOAD(m_instance, vkCreateDebugUtilsMessengerEXT);
                m_pfn.m_vkDestroyDebugUtilsMessengerEXT = VULKAN_INSTANCE_FUNC_LOAD(m_instance, vkDestroyDebugUtilsMessengerEXT);
                result                                  = m_pfn.m_vkCreateDebugUtilsMessengerEXT(m_instance, &debugUtilsCI, nullptr, &m_debugUtilsMessenger);
                if (!result)
                {
                    Shutdown();
                    return result;
                }
            }
        }

        // Select the physical device

        TL::Vector<const char*> requiredDeviceLayers;
        TL::Vector<const char*> requiredDeviceExtensions{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            // VK_KHR_CALIBRATED_TIMESTAMPS_EXTENSION_NAME,
        };

        for (VkPhysicalDevice physicalDevice : GetAvailablePhysicalDevices(m_instance))
        {
            TL::Map<TL::String, VkLayerProperties> availableDeviceLayers;
            for (VkLayerProperties layer : GetAvailableDeviceLayerExtensions(physicalDevice))
            {
                availableDeviceLayers[layer.layerName] = layer;
            }

            TL::Map<TL::String, VkExtensionProperties> availableDeviceExtensions;
            for (VkExtensionProperties extension : GetAvailableDeviceExtensions(physicalDevice))
            {
                availableDeviceExtensions[extension.extensionName] = extension;
            }

            // search for a suitable physical device if it contains the required extensions
            bool containAllLayers = std::all_of(requiredDeviceLayers.begin(), requiredDeviceLayers.end(), [&](const char* layer)
                {
                    return availableDeviceExtensions.contains(layer);
                });

            bool containAllExtensions = std::all_of(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end(), [&](const char* ext)
                {
                    return availableDeviceExtensions.contains(ext);
                });

            if (containAllLayers && containAllExtensions)
            {
                m_physicalDevice = physicalDevice;
                break;
            }
        }

        if (m_physicalDevice == VK_NULL_HANDLE)
        {
            TL_LOG_ERROR("RHI Vulkan: No suitable physical device found.");
            return ResultCode::ErrorUnknown;
        }

        TL::Optional<uint32_t> graphicsQueueFamilyIndex;
        TL::Optional<uint32_t> transferQueueFamilyIndex;
        TL::Optional<uint32_t> computeQueueFamilyIndex;

        auto queueFamilyProperties = GetPhysicalDeviceQueueFamilyProperties(m_physicalDevice);
        for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyProperties.size(); queueFamilyIndex++)
        {
            auto queueFamilyProperty = queueFamilyProperties[queueFamilyIndex];
            if (graphicsQueueFamilyIndex.has_value() == false && (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                graphicsQueueFamilyIndex.emplace(queueFamilyIndex);
                continue;
            }
            if constexpr (EnableAsyncQueues)
            {
                if (computeQueueFamilyIndex.has_value() == false && (queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT))
                {
                    computeQueueFamilyIndex.emplace(queueFamilyIndex);
                }
                else if (transferQueueFamilyIndex.has_value() == false && (queueFamilyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT))
                {
                    transferQueueFamilyIndex.emplace(queueFamilyIndex);
                }
            }
        }

        float                               queuePriority    = 1.0f;
        TL::Vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};

        VkDeviceQueueCreateInfo queueCI{
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext            = nullptr,
            .flags            = 0,
            .queueCount       = 1,
            .pQueuePriorities = &queuePriority,
        };

        if (graphicsQueueFamilyIndex)
        {
            queueCI.queueFamilyIndex = graphicsQueueFamilyIndex.value();
            queueCreateInfos.push_back(queueCI);
        }
        if (computeQueueFamilyIndex)
        {
            queueCI.queueFamilyIndex = computeQueueFamilyIndex.value();
            queueCreateInfos.push_back(queueCI);
        }
        if (transferQueueFamilyIndex)
        {
            queueCI.queueFamilyIndex = transferQueueFamilyIndex.value();
            queueCreateInfos.push_back(queueCI);
        }

        VkPhysicalDeviceVulkan13Features features13{
            .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext            = nullptr,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
        };

        VkPhysicalDeviceVulkan12Features features12{
            .sType                                        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext                                        = &features13,
            .drawIndirectCount                            = VK_TRUE,
            .descriptorIndexing                           = VK_TRUE,
            // .shaderInputAttachmentArrayDynamicIndexing          = VK_TRUE,
            // .shaderUniformTexelBufferArrayDynamicIndexing       = VK_TRUE,
            // .shaderStorageTexelBufferArrayDynamicIndexing       = VK_TRUE,
            // .shaderUniformBufferArrayNonUniformIndexing         = VK_TRUE,
            .shaderSampledImageArrayNonUniformIndexing    = VK_TRUE,
            // .shaderStorageBufferArrayNonUniformIndexing         = VK_TRUE,
            // .shaderStorageImageArrayNonUniformIndexing          = VK_TRUE,
            // .shaderInputAttachmentArrayNonUniformIndexing       = VK_TRUE,
            // .shaderUniformTexelBufferArrayNonUniformIndexing    = VK_TRUE,
            // .shaderStorageTexelBufferArrayNonUniformIndexing    = VK_TRUE,
            // .descriptorBindingUniformBufferUpdateAfterBind      = VK_TRUE,
            .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
            // .descriptorBindingStorageImageUpdateAfterBind       = VK_TRUE,
            // .descriptorBindingStorageBufferUpdateAfterBind      = VK_TRUE,
            // .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE,
            // .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingUpdateUnusedWhilePending    = VK_TRUE,
            .descriptorBindingPartiallyBound              = VK_TRUE,
            .descriptorBindingVariableDescriptorCount     = VK_TRUE,
            .runtimeDescriptorArray                       = VK_TRUE,
            .timelineSemaphore                            = VK_TRUE,
            .bufferDeviceAddress                          = VK_TRUE,
            // .bufferDeviceAddressCaptureReplay             = VK_TRUE,
            // .bufferDeviceAddressMultiDevice               = VK_TRUE,
        };

        VkPhysicalDeviceVulkan11Features features11{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &features12,
        };

        VkPhysicalDeviceFeatures2 features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &features11,
            .features =
                {
                    .independentBlend  = VK_TRUE,
                    .samplerAnisotropy = VK_TRUE,
                },
        };

        VkDeviceCreateInfo deviceCI{
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext                   = &features,
            .flags                   = 0,
            .queueCreateInfoCount    = (uint32_t)queueCreateInfos.size(),
            .pQueueCreateInfos       = queueCreateInfos.data(),
            .enabledLayerCount       = (uint32_t)requiredDeviceLayers.size(),
            .ppEnabledLayerNames     = requiredDeviceLayers.data(),
            .enabledExtensionCount   = (uint32_t)requiredDeviceExtensions.size(),
            .ppEnabledExtensionNames = requiredDeviceExtensions.data(),
            .pEnabledFeatures        = nullptr,
        };

        result = vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device);
        if (!result)
        {
            Shutdown();
            return result;
        }

        VmaAllocatorCreateInfo vmaCI{
            .physicalDevice   = m_physicalDevice,
            .device           = m_device,
            .instance         = m_instance,
            .vulkanApiVersion = VK_API_VERSION_1_3,
        };
        result = vmaCreateAllocator(&vmaCI, &m_deviceAllocator);
        VkResultTry(result);

        if constexpr (DebugLayerEnabled)
        {
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
        }
        m_pfn.m_vkCmdBeginConditionalRenderingEXT = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdBeginConditionalRenderingEXT);
        m_pfn.m_vkCmdEndConditionalRenderingEXT   = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdEndConditionalRenderingEXT);

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
        m_limits                                  = TL::CreatePtr<DeviceLimits>();
        m_limits->minUniformBufferOffsetAlignment = uint32_t(properties.limits.minUniformBufferOffsetAlignment);
        m_limits->minStorageBufferOffsetAlignment = uint32_t(properties.limits.minStorageBufferOffsetAlignment);

        result = m_queue[(uint32_t)QueueType::Graphics].Init(this, "Graphics", graphicsQueueFamilyIndex.value(), 0);
        VkResultTry(result);

        if (computeQueueFamilyIndex)
        {
            result = m_queue[(uint32_t)QueueType::Compute].Init(this, "Compute", computeQueueFamilyIndex.value(), 0);
            VkResultTry(result);
        }

        if (transferQueueFamilyIndex)
        {
            result = m_queue[(uint32_t)QueueType::Transfer].Init(this, "Transfer", transferQueueFamilyIndex.value(), 0);
            VkResultTry(result);
        }

        result = m_bindGroupAllocator->Init(this);
        VkResultTry(result);

        m_framesInFlight.resize(2);
        for (auto& frame : m_framesInFlight)
        {
            frame           = TL::CreatePtr<IFrame>();
            auto resultCode = frame->Init(this);
            if (IsError(resultCode)) return resultCode;
        }

        {
            m_renderdoc            = TL::CreatePtr<Renderdoc>();
            TL_MAYBE_UNUSED auto _ = m_renderdoc->Init(this);
        }

        return result;
    }

    void IDevice::Shutdown()
    {
        ZoneScoped;

        WaitIdle();

        if (GetDebugRenderdoc())
        {
            m_renderdoc->Shutdown();
        }

        for (auto& frame : m_framesInFlight)
        {
            frame->Shutdown();
        }

        // Report live object stack tracecs
        {
            auto liveSwapchains = m_liveSwapchains;
            for (auto [ptr, stacktrace] : liveSwapchains)
            {
                TL_LOG_WARNNING("Leaked: RHI::Swapchain at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroySwapchain(ptr);
            }

            auto liveShaderModules = m_liveShaderModules;
            for (auto [ptr, stacktrace] : liveShaderModules)
            {
                TL_LOG_WARNNING("Leaked: RHI::ShaderModule at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyShaderModule(ptr);
            }

            auto liveImages = m_liveImages;
            for (auto [ptr, stacktrace] : liveImages)
            {
                TL_LOG_WARNNING("Leaked: RHI::Image at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyImage(ptr);
            }

            auto liveBuffers = m_liveBuffers;
            for (auto [ptr, stacktrace] : liveBuffers)
            {
                TL_LOG_WARNNING("Leaked: RHI::Buffer at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyBuffer(ptr);
            }

            auto liveBindGroupLayouts = m_liveBindGroupLayouts;
            for (auto [ptr, stacktrace] : liveBindGroupLayouts)
            {
                TL_LOG_WARNNING("Leaked: RHI::BindGroupLayout at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyBindGroupLayout(ptr);
            }

            auto liveBindGroups = m_liveBindGroups;
            for (auto [ptr, stacktrace] : liveBindGroups)
            {
                TL_LOG_WARNNING("Leaked: RHI::BindGroup at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyBindGroup(ptr);
            }

            auto livePipelineLayouts = m_livePipelineLayouts;
            for (auto [ptr, stacktrace] : livePipelineLayouts)
            {
                TL_LOG_WARNNING("Leaked: RHI::PipelineLayout at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyPipelineLayout(ptr);
            }

            auto liveGraphicsPipelines = m_liveGraphicsPipelines;
            for (auto [ptr, stacktrace] : liveGraphicsPipelines)
            {
                TL_LOG_WARNNING("Leaked: RHI::GraphicsPipeline at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyGraphicsPipeline(ptr);
            }

            auto liveComputePipelines = m_liveComputePipelines;
            for (auto [ptr, stacktrace] : liveComputePipelines)
            {
                TL_LOG_WARNNING("Leaked: RHI::ComputePipeline at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyComputePipeline(ptr);
            }

            auto liveSamplers = m_liveSamplers;
            for (auto [ptr, stacktrace] : liveSamplers)
            {
                TL_LOG_WARNNING("Leaked: RHI::Sampler at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroySampler(ptr);
            }
        }

        m_destroyQueue->shutdown(this);
        m_bindGroupAllocator->Shutdown();

        for (auto& queue : m_queue)
        {
            if (queue.GetHandle() != VK_NULL_HANDLE)
                queue.Shutdown();
        }

        vmaDestroyAllocator(m_deviceAllocator);

        vkDestroyDevice(m_device, nullptr);

        if (m_debugUtilsMessenger != VK_NULL_HANDLE)
        {
            m_pfn.m_vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugUtilsMessenger, nullptr);
        }

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

    void IDevice::WaitIdle()
    {
        ZoneScoped;
        vkDeviceWaitIdle(m_device);
    }

    uint64_t IDevice::GetNativeHandle(NativeHandleType type, uint64_t _handle)
    {
        switch (type)
        {
        case NativeHandleType::None: return 0;
        case NativeHandleType::Device:
            {
                auto handle = reinterpret_cast<IDevice*>(_handle);
                return (uint64_t)handle->m_device;
            }
        case NativeHandleType::CommandList:
            {
                auto handle = reinterpret_cast<ICommandList*>(_handle);
                return (uint64_t)handle->m_commandBuffer;
            }
        case NativeHandleType::Buffer:
        case NativeHandleType::Image:
        case NativeHandleType::ImageView:
        case NativeHandleType::Sampler:
        case NativeHandleType::ShaderModule:
        case NativeHandleType::Pipeline:
        case NativeHandleType::PipelineLayout:
        case NativeHandleType::BindGroupLayout:
        case NativeHandleType::BindGroup:
        case NativeHandleType::Swapchain:
        default:
            TL_UNREACHABLE_MSG("TODO! implement");
        }
        return 0;
    }

    void IDevice::UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo)
    {
        ZoneScoped;
        auto bindGroup = (IBindGroup*)(handle);
        bindGroup->Update(this, updateInfo);
    }

    ResultCode IDevice::SetFramesInFlightCount(uint32_t count)
    {
        auto previousSize = m_framesInFlight.size();
        m_framesInFlight.resize(count);
        for (size_t i = previousSize; i < m_framesInFlight.size(); i++)
        {
            auto result = m_framesInFlight[i]->Init(this);
            if (IsError(result))
                return result;
        }
        return ResultCode::Success;
    }

    Frame* IDevice::GetCurrentFrame()
    {
        return m_framesInFlight[m_currentFrameIndex].get();
    }

    TL::IAllocator& IDevice::GetTempAllocator()
    {
        return m_framesInFlight[m_currentFrameIndex]->GetAllocator();
    }

    Swapchain* IDevice::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        auto handle = TL ::construct<ISwapchain>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveSwapchains.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroySwapchain(Swapchain* _handle)
    {
        auto erased = m_liveSwapchains.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (ISwapchain*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    };

    ShaderModule* IDevice::CreateShaderModule(const ShaderModuleCreateInfo& createInfo)
    {
        auto handle = TL ::construct<IShaderModule>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveShaderModules.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyShaderModule(ShaderModule* _handle)
    {
        auto erased = m_liveShaderModules.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IShaderModule*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    };

    BindGroupLayout* IDevice::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        auto handle = TL ::construct<IBindGroupLayout>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveBindGroupLayouts.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyBindGroupLayout(BindGroupLayout* _handle)
    {
        auto erased = m_liveBindGroupLayouts.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IBindGroupLayout*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    };

    BindGroup* IDevice::CreateBindGroup(const BindGroupCreateInfo& createInfo)
    {
        auto handle = TL ::construct<IBindGroup>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveBindGroups.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyBindGroup(BindGroup* _handle)
    {
        auto erased = m_liveBindGroups.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IBindGroup*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    };

    PipelineLayout* IDevice::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        auto handle = TL ::construct<IPipelineLayout>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_livePipelineLayouts.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyPipelineLayout(PipelineLayout* _handle)
    {
        auto erased = m_livePipelineLayouts.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IPipelineLayout*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    };

    GraphicsPipeline* IDevice::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        auto handle = TL ::construct<IGraphicsPipeline>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveGraphicsPipelines.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyGraphicsPipeline(GraphicsPipeline* _handle)
    {
        auto erased = m_liveGraphicsPipelines.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IGraphicsPipeline*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    };

    ComputePipeline* IDevice::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        auto handle = TL ::construct<IComputePipeline>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveComputePipelines.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyComputePipeline(ComputePipeline* _handle)
    {
        auto erased = m_liveComputePipelines.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IComputePipeline*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    };

    Sampler* IDevice::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        auto handle = TL ::construct<ISampler>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveSamplers.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroySampler(Sampler* _handle)
    {
        auto erased = m_liveSamplers.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (ISampler*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    };

    Image* IDevice::CreateImage(const ImageCreateInfo& createInfo)
    {
        auto handle = TL ::construct<IImage>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveImages.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    Image* IDevice::CreateImageView(TL_MAYBE_UNUSED const ImageViewCreateInfo& createInfo)
    {
        TL_UNREACHABLE_MSG("TODO! Implement image views for Vulkan Backend!");
        return {};
    }

    void IDevice::DestroyImage(Image* _handle)
    {
        auto erased = m_liveImages.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IImage*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    };

    Buffer* IDevice::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        auto handle = TL ::construct<IBuffer>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveBuffers.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyBuffer(Buffer* _handle)
    {
        auto erased = m_liveBuffers.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IBuffer*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    };

    void DeleteQueue::shutdown(IDevice* device)
    {
        Flush(device, UINT64_MAX);
        TL_ASSERT(m_allocation.empty());
        TL_ASSERT(m_buffer.empty());
        TL_ASSERT(m_bufferView.empty());
        TL_ASSERT(m_image.empty());
        TL_ASSERT(m_imageView.empty());
        TL_ASSERT(m_sampler.empty());
        TL_ASSERT(m_pipeline.empty());
        TL_ASSERT(m_descriptorPool.empty());
        TL_ASSERT(m_swapchain.empty());
        TL_ASSERT(m_surface.empty());
        TL_ASSERT(m_semaphore.empty());
        TL_ASSERT(m_vmaBuffer.empty());
        TL_ASSERT(m_vmaImage.empty());
        TL_ASSERT(m_pending.empty());
    }

    void DeleteQueue::Flush(IDevice* device, uint64_t timeline)
    {
        // flush in an order that is safe: destroy child objects before parents
        FlushQueue(device, m_bufferView, timeline);
        FlushQueue(device, m_imageView, timeline);
        FlushQueue(device, m_descriptorPool, timeline);
        FlushQueue(device, m_pipeline, timeline);
        FlushQueue(device, m_sampler, timeline);
        FlushQueue(device, m_buffer, timeline);
        FlushQueue(device, m_image, timeline);
        FlushQueue(device, m_swapchain, timeline);
        FlushQueue(device, m_surface, timeline);
        FlushQueue(device, m_semaphore, timeline);
        FlushQueue(device, m_vmaBuffer, timeline);
        FlushQueue(device, m_vmaImage, timeline);
        FlushQueue(device, m_allocation, timeline);
    }

} // namespace RHI::Vulkan