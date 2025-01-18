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

#include <TL/Allocator/Allocator.hpp>
#include <TL/Assert.hpp>
#include <TL/Log.hpp>

#include <format>
#include <tracy/Tracy.hpp>

#include "CommandList.hpp"
#include "CommandPool.hpp"
#include "Common.hpp"
#include "Queue.hpp"
#include "RHI-Vulkan/Loader.hpp"
#include "RenderGraph.hpp"
#include "StagingBuffer.hpp"
#include "Swapchain.hpp"

#define VULKAN_DEVICE_FUNC_LOAD(device, proc) reinterpret_cast<PFN_##proc>(vkGetDeviceProcAddr(device, #proc));
#define VULKAN_INSTANCE_FUNC_LOAD(instance, proc) reinterpret_cast<PFN_##proc>(vkGetInstanceProcAddr(instance, #proc));

#define RHI_VK_TRY(expr) \
    if (auto _result_ = expr; _result_ != VK_SUCCESS) return ConvertResult(_result_);

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
    // Validation settings: to fine tune what is checked
    struct ValidationSettings
    {
        VkBool32                fine_grained_locking  = VK_TRUE;
        VkBool32                validate_core         = VK_TRUE;
        VkBool32                check_image_layout    = VK_TRUE;
        VkBool32                check_command_buffer  = VK_TRUE;
        VkBool32                check_object_in_use   = VK_TRUE;
        VkBool32                check_query           = VK_TRUE;
        VkBool32                check_shaders         = VK_TRUE;
        VkBool32                check_shaders_caching = VK_TRUE;
        VkBool32                unique_handles        = VK_TRUE;
        VkBool32                object_lifetime       = VK_TRUE;
        VkBool32                stateless_param       = VK_TRUE;
        TL::Vector<const char*> debug_action          = {"VK_DBG_LAYER_ACTION_LOG_MSG"}; // "VK_DBG_LAYER_ACTION_DEBUG_OUTPUT", "VK_DBG_LAYER_ACTION_BREAK"
        TL::Vector<const char*> report_flags          = {"error"};

        VkBaseInStructure* BuildPNextChain(void* pNext)
        {
            // clang-format off
            layerSettings = {
                {layerName, "fine_grained_locking",  VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,                             &fine_grained_locking },
                {layerName, "validate_core",         VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,                             &validate_core        },
                {layerName, "check_image_layout",    VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,                             &check_image_layout   },
                {layerName, "check_command_buffer",  VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,                             &check_command_buffer },
                {layerName, "check_object_in_use",   VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,                             &check_object_in_use  },
                {layerName, "check_query",           VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,                             &check_query          },
                {layerName, "check_shaders",         VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,                             &check_shaders        },
                {layerName, "check_shaders_caching", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,                             &check_shaders_caching},
                {layerName, "unique_handles",        VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,                             &unique_handles       },
                {layerName, "object_lifetime",       VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,                             &object_lifetime      },
                {layerName, "stateless_param",       VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,                             &stateless_param      },
                {layerName, "debug_action",          VK_LAYER_SETTING_TYPE_STRING_EXT, uint32_t(debug_action.size()), debug_action.data()   },
                {layerName, "report_flags",          VK_LAYER_SETTING_TYPE_STRING_EXT, uint32_t(report_flags.size()), report_flags.data()   },
            };
            // clang-format on

            layerSettingsCreateInfo = {
                .sType        = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
                .pNext        = pNext,
                .settingCount = uint32_t(layerSettings.size()),
                .pSettings    = layerSettings.data(),
            };

            return reinterpret_cast<VkBaseInStructure*>(&layerSettingsCreateInfo);
        }

        static constexpr const char*  layerName{"VK_LAYER_KHRONOS_validation"};
        TL::Vector<VkLayerSettingEXT> layerSettings{};
        VkLayerSettingsCreateInfoEXT  layerSettingsCreateInfo{};
    };

    /// @todo: add support for a custom sink, so vulkan errors are spereated
    VkBool32 DebugMessengerCallbacks(
        [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
        [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        [[maybe_unused]] void*                                       pUserData)
    {
        TL::String additionalInfo;

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
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            TL_LOG_INFO("{}\nMessage: {}", additionalInfo, pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            TL_LOG_WARNNING("{}\nMessage: {}", additionalInfo, pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            TL_LOG_ERROR("{}\nMessage: {}", additionalInfo, pCallbackData->pMessage);
            break;
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
    {
        m_destroyQueue       = TL::CreatePtr<DeleteQueue>();
        m_bindGroupAllocator = TL::CreatePtr<BindGroupAllocator>();
        m_commandsAllocator  = TL::CreatePtr<CommandAllocator>();
        m_stagingAllocator   = TL::CreatePtr<StagingBufferAllocator>();
    }

    IDevice::~IDevice() = default;

    ResultCode IDevice::Init(const ApplicationInfo& appInfo)
    {
        ZoneScoped;

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
            if (availableInstanceLayers.contains(ValidationSettings::layerName))
            {
                requiredInstanceLayers.push_back(ValidationSettings::layerName);
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
            .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
            .pfnUserCallback = DebugMessengerCallbacks,
            .pUserData       = this,
        };

        ValidationSettings   validationSettings{};
        VkInstanceCreateInfo instanceCI{
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext                   = DebugLayerEnabled ? validationSettings.BuildPNextChain(availableInstanceExtensions.contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) ? &debugUtilsCI : nullptr) : nullptr,
            .flags                   = {},
            .pApplicationInfo        = &applicationInfo,
            .enabledLayerCount       = static_cast<uint32_t>(requiredInstanceLayers.size()),
            .ppEnabledLayerNames     = requiredInstanceLayers.data(),
            .enabledExtensionCount   = static_cast<uint32_t>(requiredInstanceExtensions.size()),
            .ppEnabledExtensionNames = requiredInstanceExtensions.data(),
        };
        RHI_VK_TRY(vkCreateInstance(&instanceCI, nullptr, &m_instance));

        if constexpr (DebugLayerEnabled)
        {
            if (availableInstanceExtensions.contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
            {
                m_pfn.m_vkCreateDebugUtilsMessengerEXT  = VULKAN_INSTANCE_FUNC_LOAD(m_instance, vkCreateDebugUtilsMessengerEXT);
                m_pfn.m_vkDestroyDebugUtilsMessengerEXT = VULKAN_INSTANCE_FUNC_LOAD(m_instance, vkDestroyDebugUtilsMessengerEXT);
                RHI_VK_TRY(m_pfn.m_vkCreateDebugUtilsMessengerEXT(m_instance, &debugUtilsCI, nullptr, &m_debugUtilsMessenger));
            }
        }

        // Select the physical device

        TL::Vector<const char*> requiredDeviceLayers;
        TL::Vector<const char*> requiredDeviceExtensions{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_CALIBRATED_TIMESTAMPS_EXTENSION_NAME,
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
            bool containAllLayers = std::all_of(
                requiredDeviceLayers.begin(),
                requiredDeviceLayers.end(),
                [&](const char* layer)
                {
                    return availableDeviceExtensions.contains(layer);
                });
            bool containAllExtensions = std::all_of(
                requiredDeviceExtensions.begin(), requiredDeviceExtensions.end(), [&](const char* ext)
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
                graphicsQueueFamilyIndex = queueFamilyIndex;
                continue;
            }
            if constexpr (EnableAsyncQueues)
            {
                if (computeQueueFamilyIndex.has_value() == false && (queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT))
                {
                    computeQueueFamilyIndex = queueFamilyIndex;
                }
                else if (transferQueueFamilyIndex.has_value() == false && (queueFamilyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT))
                {
                    transferQueueFamilyIndex = queueFamilyIndex;
                }
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
            .sType                                              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext                                              = &features13,
            .descriptorIndexing                                 = VK_TRUE,
            .shaderInputAttachmentArrayDynamicIndexing          = VK_TRUE,
            .shaderUniformTexelBufferArrayDynamicIndexing       = VK_TRUE,
            .shaderStorageTexelBufferArrayDynamicIndexing       = VK_TRUE,
            .shaderUniformBufferArrayNonUniformIndexing         = VK_TRUE,
            .shaderSampledImageArrayNonUniformIndexing          = VK_TRUE,
            .shaderStorageBufferArrayNonUniformIndexing         = VK_TRUE,
            .shaderStorageImageArrayNonUniformIndexing          = VK_TRUE,
            .shaderInputAttachmentArrayNonUniformIndexing       = VK_TRUE,
            .shaderUniformTexelBufferArrayNonUniformIndexing    = VK_TRUE,
            .shaderStorageTexelBufferArrayNonUniformIndexing    = VK_TRUE,
            .descriptorBindingUniformBufferUpdateAfterBind      = VK_TRUE,
            .descriptorBindingSampledImageUpdateAfterBind       = VK_TRUE,
            .descriptorBindingStorageImageUpdateAfterBind       = VK_TRUE,
            .descriptorBindingStorageBufferUpdateAfterBind      = VK_TRUE,
            .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingUpdateUnusedWhilePending          = VK_TRUE,
            .descriptorBindingPartiallyBound                    = VK_TRUE,
            .descriptorBindingVariableDescriptorCount           = VK_TRUE,
            .runtimeDescriptorArray                             = VK_TRUE,
            .timelineSemaphore                                  = VK_TRUE,
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
            .enabledLayerCount       = (uint32_t)requiredDeviceLayers.size(),
            .ppEnabledLayerNames     = requiredDeviceLayers.data(),
            .enabledExtensionCount   = (uint32_t)requiredDeviceExtensions.size(),
            .ppEnabledExtensionNames = requiredDeviceExtensions.data(),
            .pEnabledFeatures        = nullptr,
        };

        RHI_VK_TRY(vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device));

        VmaAllocatorCreateInfo vmaCI{
            .physicalDevice   = m_physicalDevice,
            .device           = m_device,
            .instance         = m_instance,
            .vulkanApiVersion = VK_API_VERSION_1_3,
        };
        RHI_VK_TRY(vmaCreateAllocator(&vmaCI, &m_deviceAllocator));

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

        VkPhysicalDeviceProperties properties {};
        vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
        m_limits = TL::CreatePtr<DeviceLimits>();
        m_limits->minUniformBufferOffsetAlignment = properties.limits.minUniformBufferOffsetAlignment;

        ResultCode resultCode;

        resultCode = m_queue[(uint32_t)QueueType::Graphics].Init(this, "Graphics", graphicsQueueFamilyIndex.value(), 0);
        if (IsError(resultCode)) return resultCode;

        if (computeQueueFamilyIndex)
        {
            resultCode = m_queue[(uint32_t)QueueType::Compute].Init(this, "Compute", computeQueueFamilyIndex.value(), 0);
            if (IsError(resultCode)) return resultCode;
        }

        if (transferQueueFamilyIndex)
        {
            resultCode = m_queue[(uint32_t)QueueType::Transfer].Init(this, "Transfer", transferQueueFamilyIndex.value(), 0);
            if (IsError(resultCode)) return resultCode;
        }

        resultCode = m_stagingAllocator->Init(this);
        if (IsError(resultCode)) return resultCode;

        resultCode = m_commandsAllocator->Init(this);
        if (IsError(resultCode)) return resultCode;

        resultCode = m_bindGroupAllocator->Init(this);
        if (IsError(resultCode)) return resultCode;

        resultCode = m_destroyQueue->Init(this);
        if (IsError(resultCode)) return resultCode;

        return resultCode;
    }

    void IDevice::Shutdown()
    {
        ZoneScoped;

        vkDeviceWaitIdle(m_device);

        if (auto count = m_imageOwner.ReportLiveResourcesCount())
            TL_LOG_WARNNING("Detected {} Image leaked", count);
        if (auto count = m_bufferOwner.ReportLiveResourcesCount())
            TL_LOG_WARNNING("Detected {} Buffer leaked", count);
        if (auto count = m_bindGroupLayoutsOwner.ReportLiveResourcesCount())
            TL_LOG_WARNNING("Detected {} BindGroupLayout leaked", count);
        if (auto count = m_bindGroupOwner.ReportLiveResourcesCount())
            TL_LOG_WARNNING("Detected {} BindGroup leaked", count);
        if (auto count = m_pipelineLayoutOwner.ReportLiveResourcesCount())
            TL_LOG_WARNNING("Detected {} PipelineLayout leaked", count);
        if (auto count = m_graphicsPipelineOwner.ReportLiveResourcesCount())
            TL_LOG_WARNNING("Detected {} GraphicsPipeline leaked", count);
        if (auto count = m_computePipelineOwner.ReportLiveResourcesCount())
            TL_LOG_WARNNING("Detected {} ComputePipeline leaked", count);
        if (auto count = m_samplerOwner.ReportLiveResourcesCount())
            TL_LOG_WARNNING("Detected {} Sampler leaked", count);

        m_stagingAllocator->Shutdown();
        m_commandsAllocator->Shutdown();
        m_bindGroupAllocator->Shutdown();
        m_destroyQueue->Shutdown();
        for (auto& queue : m_queue)
        {
            if (queue)
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

    RenderGraph* IDevice::CreateRenderGraph()
    {
        ZoneScoped;

        auto renderGraph = new IRenderGraph();
        auto result      = renderGraph->Init(this);
        TL_ASSERT(IsSuccess(result));
        return renderGraph;
    }

    void IDevice::DestroyRenderGraph(RenderGraph* renderGraph)
    {
        ZoneScoped;
        delete renderGraph;
    }

    Swapchain* IDevice::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;
        auto swapchain = new ISwapchain();
        auto result    = swapchain->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        return swapchain;
    }

    void IDevice::DestroySwapchain(Swapchain* _swapchain)
    {
        ZoneScoped;
        auto swapchain = (ISwapchain*)_swapchain;
        swapchain->Shutdown();
        delete swapchain;
    }

    TL::Ptr<ShaderModule> IDevice::CreateShaderModule(const ShaderModuleCreateInfo& createInfo)
    {
        ZoneScoped;
        auto shaderModule = TL::CreatePtr<IShaderModule>();
        auto result       = shaderModule->Init(this, createInfo.code);
        TL_ASSERT(IsSuccess(result));
        return shaderModule;
    }

    CommandList* IDevice::CreateCommandList(const CommandListCreateInfo& createInfo)
    {
        ZoneScoped;
        return m_commandsAllocator->AllocateCommandList(createInfo.queueType, m_tempAllocator);
    }

    Handle<BindGroupLayout> IDevice::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        ZoneScoped;
        auto [handle, result] = m_bindGroupLayoutsOwner.Create(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        return handle;
    }

    void IDevice::DestroyBindGroupLayout(Handle<BindGroupLayout> handle)
    {
        ZoneScoped;
        m_destroyQueue->Push(
            m_frameIndex,
            [handle](IDevice* device)
            {
                device->m_bindGroupLayoutsOwner.Destroy(handle, device);
            });
    }

    Handle<BindGroup> IDevice::CreateBindGroup(Handle<BindGroupLayout> handle)
    {
        ZoneScoped;
        auto [bindGroupHandle, result] = m_bindGroupOwner.Create(this, handle);
        TL_ASSERT(IsSuccess(result));
        return bindGroupHandle;
    }

    void IDevice::DestroyBindGroup(Handle<BindGroup> handle)
    {
        ZoneScoped;
        m_destroyQueue->Push(
            m_frameIndex,
            [handle](IDevice* device)
            {
                device->m_bindGroupOwner.Destroy(handle, device);
            });
    }

    void IDevice::UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo)
    {
        ZoneScoped;
        auto bindGroup = m_bindGroupOwner.Get(handle);
        bindGroup->Write(this, updateInfo);
    }

    Handle<PipelineLayout> IDevice::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        ZoneScoped;
        auto [handle, result] = m_pipelineLayoutOwner.Create(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        return handle;
    }

    void IDevice::DestroyPipelineLayout(Handle<PipelineLayout> handle)
    {
        ZoneScoped;
        m_destroyQueue->Push(
            m_frameIndex,
            [handle](IDevice* device)
            {
                device->m_pipelineLayoutOwner.Destroy(handle, device);
            });
    }

    Handle<GraphicsPipeline> IDevice::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        ZoneScoped;
        auto [handle, result] = m_graphicsPipelineOwner.Create(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        return handle;
    }

    void IDevice::DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)
    {
        ZoneScoped;
        m_destroyQueue->Push(
            m_frameIndex,
            [handle](IDevice* device)
            {
                device->m_graphicsPipelineOwner.Destroy(handle, device);
            });
    }

    Handle<ComputePipeline> IDevice::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        ZoneScoped;
        auto [handle, result] = m_computePipelineOwner.Create(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        return handle;
    }

    void IDevice::DestroyComputePipeline(Handle<ComputePipeline> handle)
    {
        ZoneScoped;
        m_destroyQueue->Push(
            m_frameIndex,
            [handle](IDevice* device)
            {
                device->m_computePipelineOwner.Destroy(handle, device);
            });
    }

    Handle<Sampler> IDevice::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        ZoneScoped;
        auto [handle, result] = m_samplerOwner.Create(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        return handle;
    }

    void IDevice::DestroySampler(Handle<Sampler> handle)
    {
        ZoneScoped;
        m_destroyQueue->Push(
            m_frameIndex,
            [handle](IDevice* device)
            {
                device->m_samplerOwner.Destroy(handle, device);
            });
    }

    Result<Handle<Image>> IDevice::CreateImage(const ImageCreateInfo& createInfo)
    {
        ZoneScoped;
        auto [handle, result] = m_imageOwner.Create(this, createInfo);
        if (IsSuccess(result)) return (Handle<Image>)handle;
        return result;
    }

    void IDevice::DestroyImage(Handle<Image> handle)
    {
        ZoneScoped;
        m_destroyQueue->Push(
            m_frameIndex,
            [handle](IDevice* device)
            {
                device->m_imageOwner.Destroy(handle, device);
            });
    }

    Result<Handle<Buffer>> IDevice::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        ZoneScoped;
        auto [handle, result] = m_bufferOwner.Create(this, createInfo);
        if (IsSuccess(result)) return (Handle<Buffer>)handle;
        return result;
    }

    void IDevice::DestroyBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;
        m_destroyQueue->Push(
            m_frameIndex,
            [handle](IDevice* device)
            {
                device->m_bufferOwner.Destroy(handle, device);
            });
    }

    DeviceMemoryPtr IDevice::MapBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;
        auto            resource = m_bufferOwner.Get(handle);
        DeviceMemoryPtr memoryPtr;
        Validate(vmaMapMemory(m_deviceAllocator, resource->allocation, &memoryPtr));
        return memoryPtr;
    }

    void IDevice::UnmapBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;
        auto resource = m_bufferOwner.Get(handle)->allocation;
        vmaUnmapMemory(m_deviceAllocator, resource);
    }

    StagingBuffer IDevice::StagingAllocate(size_t size)
    {
        ZoneScoped;
        return m_stagingAllocator->Allocate(size);
    }

    uint64_t IDevice::UploadImage(const ImageUploadInfo& uploadInfo)
    {
        ZoneScoped;
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
        commandList->AddPipelineBarriers({
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
                    // TODO:
                    // .imageAspects = ConvertImageAspect(GetFormatAspects(image.format)),
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
        commandList->AddPipelineBarriers({
            .imageBarriers = {{
                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext               = nullptr,
                .srcStageMask        = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .srcAccessMask       = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .dstStageMask        = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
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

        QueueSubmitInfo submitInfo(*this);
        submitInfo.AddCommandList(commandList->GetHandle());
        submitInfo.signalStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        auto res = m_queue[(int)QueueType::Transfer].Submit(submitInfo);
        vkDeviceWaitIdle(m_device);
        return res;
    }

    void IDevice::CollectResources()
    {
        ZoneScoped;
        m_tempAllocator.Collect();
        m_destroyQueue->DestroyObjects(false);
        m_stagingAllocator->ReleaseAll();
        m_frameIndex++;
    }

    bool IDevice::WaitForQueueTimelineValue(QueueType queueType, uint64_t value, uint64_t waitDuration)
    {
        return GetDeviceQueue(queueType).WaitTimeline(value, waitDuration);
    }

} // namespace RHI::Vulkan