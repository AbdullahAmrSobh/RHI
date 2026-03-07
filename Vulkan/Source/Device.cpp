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

#include "RHI-Vulkan/Loader.hpp"

#include "Device.hpp"
#include "Common.hpp"

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
    inline static VkBool32
    DebugMessengerCallbacks(
        VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void*                                       pUserData)
    {
        TL::String message = std::format("Vulkan Validation: {}\n", pCallbackData->pMessage);

        if (pCallbackData->objectCount > 0)
        {
            message += "Objects:\n";
            for (uint32_t i = 0; i < pCallbackData->objectCount; ++i)
            {
                const char* objectName = pCallbackData->pObjects[i].pObjectName;

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
                const char* labelName = pCallbackData->pCmdBufLabels[i].pLabelName;
                auto [r, g, b, a]     = pCallbackData->pCmdBufLabels[i].color;
                message += std::format("  [{}] {} (color: [{:.2f}, {:.2f}, {:.2f}, {:.2f}])\n", i, labelName, r, g, b, a);
            }
        }

        if (pCallbackData->queueLabelCount > 0)
        {
            message += "Queue Labels:\n";
            for (uint32_t i = 0; i < pCallbackData->queueLabelCount; ++i)
            {
                const char* labelName = pCallbackData->pQueueLabels[i].pLabelName;
                auto [r, g, b, a]     = pCallbackData->pQueueLabels[i].color;
                message += std::format("  [{}] {} (color: [{:.2f}, {:.2f}, {:.2f}, {:.2f}])\n", i, labelName, r, g, b, a);
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

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IQueue
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    VkResult IQueue::Init(IDevice* device, const char* debugName, uint32_t familyIndex, uint32_t queueIndex)
    {
        m_device      = device;
        m_familyIndex = familyIndex;

        vkGetDeviceQueue(device->m_device, familyIndex, queueIndex, &m_queue);
        if (debugName)
            m_device->SetDebugName(m_queue, debugName);

        return VK_SUCCESS;
    }

    void IQueue::Shutdown()
    {
        vkQueueWaitIdle(m_queue);
    }

    void IQueue::BeginAnnotation(const char* name, uint32_t bgra)
    {
        if (m_device->m_pfn.vkQueueBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT label{
                .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pNext      = nullptr,
                .pLabelName = name,
                .color      = {},
            };
            m_device->m_pfn.vkQueueBeginDebugUtilsLabelEXT(m_queue, &label);
        }
    }

    void IQueue::EndAnnotation()
    {
        if (m_device->m_pfn.vkQueueEndDebugUtilsLabelEXT)
        {
            m_device->m_pfn.vkQueueEndDebugUtilsLabelEXT(m_queue);
        }
    }

    void IQueue::InsertAnnotation(const char* name, uint32_t bgra)
    {
        if (m_device->m_pfn.vkQueueBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT label{
                .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pNext      = nullptr,
                .pLabelName = name,
                .color      = {},
            };
            m_device->m_pfn.vkQueueInsertDebugUtilsLabelEXT(m_queue, &label);
        }
    }

    void IQueue::Submit(const QueueSubmitInfo& submitInfo)
    {
        TL::Vector<VkSemaphoreSubmitInfo>     waitSemaphores{m_device->m_arena};
        TL::Vector<VkCommandBufferSubmitInfo> commandBufferSubmitInfos{m_device->m_arena};
        TL::Vector<VkSemaphoreSubmitInfo>     signalSemaphores{m_device->m_arena};

        for (auto _fence : submitInfo.waitFences)
        {
            auto fence = (IFence*)_fence.fence;
            waitSemaphores.push_back({
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = fence->semaphore,
                .value     = _fence.value,
                .stageMask = ConvertPipelineStageFlags(_fence.stage)
                // .deviceMask    = 1,
            });
        }

        for (auto _fence : submitInfo.signalFences)
        {
            auto fence = (IFence*)_fence.fence;
            signalSemaphores.push_back({
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = fence->semaphore,
                .value     = _fence.value,
                .stageMask = ConvertPipelineStageFlags(_fence.stage),
            });
        }

        for (auto cmd : submitInfo.commandLists)
        {
            auto commandList = (ICommandList*)cmd;
            commandBufferSubmitInfos.push_back({
                .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = commandList->m_commandBuffer,
                // .deviceMask    = 1,
            });
        }

        for (auto _swapchain : submitInfo.acquireSwapchains)
        {
            ISwapchain* swapchain = (ISwapchain*)_swapchain;

            VkSemaphore acquireSemaphore       = swapchain->m_acquireSemaphore[swapchain->m_acquireSemaphoreIndex];
            VkSemaphore presentSemaphore       = swapchain->m_presentSemaphore[swapchain->m_presentSemaphoreIndex];
            swapchain->m_acquireSemaphoreIndex = (swapchain->m_acquireSemaphoreIndex + 1) % ISwapchain::MaxImageCount;

            swapchain->m_acquireSemaphoreIndex += 1;
            swapchain->m_acquireSemaphoreIndex %= ISwapchain::MaxImageCount;

            // increment down during vkQueuePresent
            // swapchain->m_presentSemaphoreIndex += 1;
            // swapchain->m_presentSemaphoreIndex %= ISwapchain::MaxImageCount;

            waitSemaphores.push_back({
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = acquireSemaphore,
            });
            signalSemaphores.push_back({
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = presentSemaphore,
            });
        }

        // m_lastSubmitValue++;

        VkSubmitInfo2 vksubmitInfo = {
            .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext                    = nullptr,
            .flags                    = {},
            .waitSemaphoreInfoCount   = (uint32_t)waitSemaphores.size(),
            .pWaitSemaphoreInfos      = waitSemaphores.data(),
            .commandBufferInfoCount   = (uint32_t)commandBufferSubmitInfos.size(),
            .pCommandBufferInfos      = commandBufferSubmitInfos.data(),
            .signalSemaphoreInfoCount = (uint32_t)signalSemaphores.size(),
            .pSignalSemaphoreInfos    = signalSemaphores.data(),
        };
        VulkanResult result = vkQueueSubmit2(m_queue, 1, &vksubmitInfo, VK_NULL_HANDLE);
        TL_ASSERT(result.IsSuccess());

        vkQueueWaitIdle(m_queue);

        // assert queue is graphics
        if (submitInfo.presentSwapchains.empty() == false)
        {
            TL::Vector<VkSwapchainKHR> swapchains{m_device->m_arena};
            TL::Vector<uint32_t>       imageIndcies{m_device->m_arena};
            TL::Vector<VkSemaphore>    presentWaitSemaphores{m_device->m_arena};

            for (auto _swapchain : submitInfo.presentSwapchains)
            {
                ISwapchain* swapchain = (ISwapchain*)_swapchain;
                VkSemaphore semaphore = swapchain->m_presentSemaphore[swapchain->m_presentSemaphoreIndex];
                swapchain->m_presentSemaphoreIndex += 1;
                swapchain->m_presentSemaphoreIndex %= ISwapchain::MaxImageCount;
                presentWaitSemaphores.push_back(semaphore);
                imageIndcies.push_back(swapchain->m_imageIndex);
                swapchains.push_back(swapchain->m_swapchain);
            }

            VkPresentInfoKHR presentInfos{
                .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .pNext              = 0,
                .waitSemaphoreCount = (uint32_t)presentWaitSemaphores.size(),
                .pWaitSemaphores    = presentWaitSemaphores.data(),
                .swapchainCount     = (uint32_t)swapchains.size(),
                .pSwapchains        = swapchains.data(),
                .pImageIndices      = imageIndcies.data(),
                .pResults           = nullptr,
            };
            vkQueuePresentKHR(m_queue, &presentInfos);
        }

        vkQueueWaitIdle(m_queue);

        for (auto _swapchain : submitInfo.presentSwapchains)
        {
            ISwapchain* swapchain = (ISwapchain*)_swapchain;
            swapchain->AcquireNextImage();
        }
    }

    void IQueue::WaitIdle()
    {
        vkQueueWaitIdle(m_queue);
    }

    void IQueue::WaitFence(Fence* _fence, uint64_t value)
    {
        VkSemaphoreWaitInfo waitInfo{
            .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .pNext          = nullptr,
            .flags          = 0,
            .semaphoreCount = {},
            .pSemaphores    = {},
            .pValues        = &value,
        };
        VulkanResult result = vkWaitSemaphores(m_device->m_device, &waitInfo, UINT32_MAX);
        TL_ASSERT(result.IsSuccess());
    }

    ///

    IDevice::IDevice()
    {
        m_destroyQueue = TL::CreatePtr<DeleteQueue>();
    }

    IDevice::~IDevice() = default;

    ResultCode IDevice::Init(const ApplicationInfo& appInfo)
    {
        ZoneScoped;

        m_backend = BackendType::Vulkan1_3;

        VulkanResult result;
        TL_ASSERT(result.IsSuccess());

        // result = volkInitialize();

        constexpr bool DebugLayerEnabled = RHI_DEBUG;
        constexpr bool EnableAsyncQueues = true;

        TL::Map<TL::String, VkLayerProperties>     availableInstanceLayers;
        TL::Map<TL::String, VkExtensionProperties> availableInstanceExtensions;

        uint32_t instanceLayerCount;
        result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        TL_ASSERT(result);
        TL::Vector<VkLayerProperties> instanceLayers;
        instanceLayers.resize(instanceLayerCount);
        result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayers.data());
        TL_ASSERT(result);

        for (VkLayerProperties layer : instanceLayers)
            availableInstanceLayers[layer.layerName] = layer;

        {
            VulkanResult result;
            uint32_t     instanceExtensionsCount;
            result = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, nullptr);
            TL_ASSERT(result);
            TL::Vector<VkExtensionProperties> extensions;
            extensions.resize(instanceExtensionsCount);
            result = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, extensions.data());
            TL_ASSERT(result);
            for (VkExtensionProperties extension : extensions)
                availableInstanceExtensions[extension.extensionName] = extension;
        }

        TL::Vector<const char*> requiredInstanceLayers;
        TL::Vector<const char*> requiredInstanceExtensions{
            VK_KHR_SURFACE_EXTENSION_NAME,
            VULKAN_SURFACE_OS_EXTENSION_NAME,
        };

        if constexpr (DebugLayerEnabled)
        {
            requiredInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
            requiredInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
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
        VkInstanceCreateInfo instanceCI{
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext                   = DebugLayerEnabled ? &debugUtilsCI : nullptr,
            .flags                   = {},
            .pApplicationInfo        = &applicationInfo,
            .enabledLayerCount       = (uint32_t)requiredInstanceLayers.size(),
            .ppEnabledLayerNames     = requiredInstanceLayers.data(),
            .enabledExtensionCount   = (uint32_t)requiredInstanceExtensions.size(),
            .ppEnabledExtensionNames = requiredInstanceExtensions.data(),
        };

        TL_ASSERT(result.IsSuccess());
        result = vkCreateInstance(&instanceCI, nullptr, &m_instance);
        // volkLoadInstanceOnly(m_instance);

        if (!result)
        {
            Shutdown();
            return result;
        }

        if constexpr (DebugLayerEnabled)
        {
            if (availableInstanceExtensions.contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
            {
                m_pfn.vkCreateDebugUtilsMessengerEXT  = VULKAN_INSTANCE_FUNC_LOAD(m_instance, vkCreateDebugUtilsMessengerEXT);
                m_pfn.vkDestroyDebugUtilsMessengerEXT = VULKAN_INSTANCE_FUNC_LOAD(m_instance, vkDestroyDebugUtilsMessengerEXT);
                result                                = m_pfn.vkCreateDebugUtilsMessengerEXT(m_instance, &debugUtilsCI, nullptr, &m_debugUtilsMessenger);
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
            VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
            VK_EXT_MESH_SHADER_EXTENSION_NAME,
            // VK_KHR_CALIBRATED_TIMESTAMPS_EXTENSION_NAME,
        };

        bool enablePushDescriptors         = true;
        bool enableMeshShaders             = true;
        bool enableRayTracing              = true;
        bool enableDescriptorIndexing      = true;
        bool enableDeviceGeneratedCommands = true;

        if (enablePushDescriptors)
        {
            requiredDeviceExtensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
        }

        if (enableMeshShaders)
        {
            requiredDeviceExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
        }

        if (enableRayTracing)
        {
            requiredDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
            requiredDeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            requiredDeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
            requiredDeviceExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
            requiredDeviceExtensions.push_back(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME);
        }

        {
            VulkanResult result;
            uint32_t     physicalDeviceCount;
            result = vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
            TL::Vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
            result = vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());
            for (VkPhysicalDevice physicalDevice : physicalDevices)
            {
                TL::Map<TL::String, VkLayerProperties> availableDeviceLayers;
                {
                    VulkanResult result;
                    uint32_t     instanceLayerCount;
                    result = vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, nullptr);
                    TL_ASSERT(result);
                    TL::Vector<VkLayerProperties> layers;
                    layers.resize(instanceLayerCount);
                    result = vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, layers.data());
                    TL_ASSERT(result);
                    for (VkLayerProperties layer : layers)
                        availableDeviceLayers[layer.layerName] = layer;
                }

                TL::Map<TL::String, VkExtensionProperties> availableDeviceExtensions;
                {
                    VulkanResult result;
                    uint32_t     extensionsCount;
                    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr);
                    TL_ASSERT(result);
                    TL::Vector<VkExtensionProperties> extnesions;
                    extnesions.resize(extensionsCount);
                    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, extnesions.data());
                    TL_ASSERT(result);
                    for (VkExtensionProperties extension : extnesions)
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
        }

        uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t transferQueueFamilyIndex = UINT32_MAX;
        uint32_t computeQueueFamilyIndex  = UINT32_MAX;

        uint32_t queueFamilyPropertiesCount;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyPropertiesCount, nullptr);
        TL::Vector<VkQueueFamilyProperties> queueFamilyProperties{};
        queueFamilyProperties.resize(queueFamilyPropertiesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());

        for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < uint32_t(queueFamilyProperties.size()); ++queueFamilyIndex)
        {
            const auto& queueFamilyProperty = queueFamilyProperties[queueFamilyIndex];

            if (graphicsQueueFamilyIndex == UINT32_MAX &&
                (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                graphicsQueueFamilyIndex = queueFamilyIndex;
                continue;
            }

            if constexpr (EnableAsyncQueues)
            {
                if (computeQueueFamilyIndex == UINT32_MAX &&
                    (queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT))
                {
                    computeQueueFamilyIndex = queueFamilyIndex;
                }
                else if (transferQueueFamilyIndex == UINT32_MAX &&
                         (queueFamilyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT))
                {
                    transferQueueFamilyIndex = queueFamilyIndex;
                }
            }
        }

        float queuePriority = 1.0f;

        TL::Vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};

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
        if (computeQueueFamilyIndex != UINT32_MAX)
        {
            queueCI.queueFamilyIndex = computeQueueFamilyIndex;
            queueCreateInfos.push_back(queueCI);
        }
        if (transferQueueFamilyIndex != UINT32_MAX)
        {
            queueCI.queueFamilyIndex = transferQueueFamilyIndex;
            queueCreateInfos.push_back(queueCI);
        }

        void* pNext = nullptr;

        VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT deviceGeneratedCommandsFeatures{
            .sType                          = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT,
            .pNext                          = pNext,
            .deviceGeneratedCommands        = VK_TRUE,
            .dynamicGeneratedPipelineLayout = VK_TRUE,
        };
        if (enableDeviceGeneratedCommands) pNext = &deviceGeneratedCommandsFeatures;

        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{
            .sType                                  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
            .pNext                                  = pNext,
            .taskShader                             = VK_TRUE,
            .meshShader                             = VK_TRUE,
            .multiviewMeshShader                    = VK_TRUE,
            .primitiveFragmentShadingRateMeshShader = VK_TRUE,
            .meshShaderQueries                      = VK_TRUE,
        };
        if (enableMeshShaders) pNext = &meshShaderFeatures;

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{
            .sType                                                 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
            .pNext                                                 = pNext,
            .rayTracingPipeline                                    = VK_TRUE,
            .rayTracingPipelineShaderGroupHandleCaptureReplay      = VK_TRUE,
            .rayTracingPipelineShaderGroupHandleCaptureReplayMixed = VK_FALSE,
            .rayTracingPipelineTraceRaysIndirect                   = VK_TRUE,
            .rayTraversalPrimitiveCulling                          = VK_TRUE,
        };
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{
            .sType                                                 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
            .pNext                                                 = &rayTracingPipelineFeatures,
            .accelerationStructure                                 = VK_TRUE,
            .accelerationStructureCaptureReplay                    = VK_TRUE,
            .accelerationStructureIndirectBuild                    = VK_FALSE, // TODO: test
            .accelerationStructureHostCommands                     = VK_FALSE, // TODO: test
            .descriptorBindingAccelerationStructureUpdateAfterBind = VK_TRUE,
        };
        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{
            .sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR,
            .pNext    = &accelerationStructureFeatures,
            .rayQuery = VK_TRUE,
        };
        VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR rayTracingPositionFetchFeaturesKHR{

            .sType                   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR,
            .pNext                   = &rayQueryFeatures,
            .rayTracingPositionFetch = VK_TRUE,
        };

        if (enableRayTracing) pNext = &rayTracingPositionFetchFeaturesKHR;
        VkPhysicalDeviceVulkan13Features features13{
            .sType                                              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext                                              = pNext,
            .robustImageAccess                                  = VK_FALSE,
            .inlineUniformBlock                                 = VK_FALSE,
            .descriptorBindingInlineUniformBlockUpdateAfterBind = VK_FALSE,
            .pipelineCreationCacheControl                       = VK_FALSE,
            .privateData                                        = VK_FALSE,
            .shaderDemoteToHelperInvocation                     = VK_FALSE,
            .shaderTerminateInvocation                          = VK_FALSE,
            .subgroupSizeControl                                = VK_FALSE,
            .computeFullSubgroups                               = VK_FALSE,
            .synchronization2                                   = VK_TRUE,
            .textureCompressionASTC_HDR                         = VK_FALSE,
            .shaderZeroInitializeWorkgroupMemory                = VK_FALSE,
            .dynamicRendering                                   = VK_TRUE,
            .shaderIntegerDotProduct                            = VK_FALSE,
            .maintenance4                                       = VK_FALSE,
        };
        VkPhysicalDeviceVulkan12Features features12{
            .sType                                              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext                                              = &features13,
            .samplerMirrorClampToEdge                           = VK_FALSE,
            .drawIndirectCount                                  = VK_TRUE,
            .storageBuffer8BitAccess                            = VK_FALSE,
            .uniformAndStorageBuffer8BitAccess                  = VK_FALSE,
            .storagePushConstant8                               = VK_FALSE,
            .shaderBufferInt64Atomics                           = VK_FALSE,
            .shaderSharedInt64Atomics                           = VK_FALSE,
            .shaderFloat16                                      = VK_FALSE,
            .shaderInt8                                         = VK_FALSE,
            .descriptorIndexing                                 = VK_TRUE,
            .shaderInputAttachmentArrayDynamicIndexing          = VK_FALSE,
            .shaderUniformTexelBufferArrayDynamicIndexing       = VK_FALSE,
            .shaderStorageTexelBufferArrayDynamicIndexing       = VK_FALSE,
            .shaderUniformBufferArrayNonUniformIndexing         = VK_FALSE,
            .shaderSampledImageArrayNonUniformIndexing          = VK_TRUE,
            .shaderStorageBufferArrayNonUniformIndexing         = VK_FALSE,
            .shaderStorageImageArrayNonUniformIndexing          = VK_TRUE,
            .shaderInputAttachmentArrayNonUniformIndexing       = VK_TRUE,
            .shaderUniformTexelBufferArrayNonUniformIndexing    = VK_FALSE,
            .shaderStorageTexelBufferArrayNonUniformIndexing    = VK_FALSE,
            .descriptorBindingUniformBufferUpdateAfterBind      = VK_TRUE,
            .descriptorBindingSampledImageUpdateAfterBind       = VK_TRUE,
            .descriptorBindingStorageImageUpdateAfterBind       = VK_TRUE,
            .descriptorBindingStorageBufferUpdateAfterBind      = VK_TRUE,
            .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingUpdateUnusedWhilePending          = VK_TRUE,
            .descriptorBindingPartiallyBound                    = VK_TRUE,
            .descriptorBindingVariableDescriptorCount           = VK_FALSE,
            .runtimeDescriptorArray                             = VK_TRUE,
            .samplerFilterMinmax                                = VK_FALSE,
            .scalarBlockLayout                                  = VK_FALSE,
            .imagelessFramebuffer                               = VK_FALSE,
            .uniformBufferStandardLayout                        = VK_FALSE,
            .shaderSubgroupExtendedTypes                        = VK_FALSE,
            .separateDepthStencilLayouts                        = VK_FALSE,
            .hostQueryReset                                     = VK_FALSE,
            .timelineSemaphore                                  = VK_TRUE,
            .bufferDeviceAddress                                = VK_TRUE,
            .bufferDeviceAddressCaptureReplay                   = DebugLayerEnabled ? VK_TRUE : VK_FALSE,
            .bufferDeviceAddressMultiDevice                     = VK_FALSE,
            .vulkanMemoryModel                                  = VK_FALSE,
            .vulkanMemoryModelDeviceScope                       = VK_FALSE,
            .vulkanMemoryModelAvailabilityVisibilityChains      = VK_FALSE,
            .shaderOutputViewportIndex                          = VK_FALSE,
            .shaderOutputLayer                                  = VK_FALSE,
            .subgroupBroadcastDynamicId                         = VK_FALSE,
        };
        VkPhysicalDeviceVulkan11Features features11{
            .sType                              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext                              = &features12,
            .storageBuffer16BitAccess           = VK_FALSE,
            .uniformAndStorageBuffer16BitAccess = VK_FALSE,
            .storagePushConstant16              = VK_FALSE,
            .storageInputOutput16               = VK_FALSE,
            .multiview                          = VK_FALSE,
            .multiviewGeometryShader            = VK_FALSE,
            .multiviewTessellationShader        = VK_FALSE,
            .variablePointersStorageBuffer      = VK_FALSE,
            .variablePointers                   = VK_FALSE,
            .protectedMemory                    = VK_FALSE,
            .samplerYcbcrConversion             = VK_FALSE,
            .shaderDrawParameters               = VK_FALSE,
        };
        VkPhysicalDeviceFeatures2 features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &features11,
        };
        features.features = {
            .robustBufferAccess                      = VK_FALSE,
            .fullDrawIndexUint32                     = VK_FALSE,
            .imageCubeArray                          = VK_FALSE,
            .independentBlend                        = VK_TRUE,
            .geometryShader                          = VK_FALSE,
            .tessellationShader                      = VK_FALSE,
            .sampleRateShading                       = VK_FALSE,
            .dualSrcBlend                            = VK_FALSE,
            .logicOp                                 = VK_FALSE,
            .multiDrawIndirect                       = VK_FALSE,
            .drawIndirectFirstInstance               = VK_FALSE,
            .depthClamp                              = VK_FALSE,
            .depthBiasClamp                          = VK_FALSE,
            .fillModeNonSolid                        = VK_FALSE,
            .depthBounds                             = VK_FALSE,
            .wideLines                               = VK_FALSE,
            .largePoints                             = VK_FALSE,
            .alphaToOne                              = VK_FALSE,
            .multiViewport                           = VK_FALSE,
            .samplerAnisotropy                       = VK_TRUE,
            .textureCompressionETC2                  = VK_FALSE,
            .textureCompressionASTC_LDR              = VK_FALSE,
            .textureCompressionBC                    = VK_FALSE,
            .occlusionQueryPrecise                   = VK_FALSE,
            .pipelineStatisticsQuery                 = VK_FALSE,
            .vertexPipelineStoresAndAtomics          = VK_FALSE,
            .fragmentStoresAndAtomics                = VK_FALSE,
            .shaderTessellationAndGeometryPointSize  = VK_FALSE,
            .shaderImageGatherExtended               = VK_FALSE,
            .shaderStorageImageExtendedFormats       = VK_FALSE,
            .shaderStorageImageMultisample           = VK_FALSE,
            .shaderStorageImageReadWithoutFormat     = VK_FALSE,
            .shaderStorageImageWriteWithoutFormat    = VK_FALSE,
            .shaderUniformBufferArrayDynamicIndexing = VK_FALSE,
            .shaderSampledImageArrayDynamicIndexing  = VK_FALSE,
            .shaderStorageBufferArrayDynamicIndexing = VK_FALSE,
            .shaderStorageImageArrayDynamicIndexing  = VK_FALSE,
            .shaderClipDistance                      = VK_FALSE,
            .shaderCullDistance                      = VK_FALSE,
            .shaderFloat64                           = VK_FALSE,
            .shaderInt64                             = VK_FALSE,
            .shaderInt16                             = VK_FALSE,
            .shaderResourceResidency                 = VK_FALSE,
            .shaderResourceMinLod                    = VK_FALSE,
            .sparseBinding                           = VK_FALSE,
            .sparseResidencyBuffer                   = VK_FALSE,
            .sparseResidencyImage2D                  = VK_FALSE,
            .sparseResidencyImage3D                  = VK_FALSE,
            .sparseResidency2Samples                 = VK_FALSE,
            .sparseResidency4Samples                 = VK_FALSE,
            .sparseResidency8Samples                 = VK_FALSE,
            .sparseResidency16Samples                = VK_FALSE,
            .sparseResidencyAliased                  = VK_FALSE,
            .variableMultisampleRate                 = VK_FALSE,
            .inheritedQueries                        = VK_FALSE,
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

        m_pfn.vkSubmitDebugUtilsMessageEXT        = VULKAN_INSTANCE_FUNC_LOAD(m_instance, vkSubmitDebugUtilsMessageEXT);
        m_pfn.vkCmdBeginDebugUtilsLabelEXT        = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdBeginDebugUtilsLabelEXT);
        m_pfn.vkCmdEndDebugUtilsLabelEXT          = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdEndDebugUtilsLabelEXT);
        m_pfn.vkCmdInsertDebugUtilsLabelEXT       = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdInsertDebugUtilsLabelEXT);
        m_pfn.vkQueueBeginDebugUtilsLabelEXT      = VULKAN_DEVICE_FUNC_LOAD(m_device, vkQueueBeginDebugUtilsLabelEXT);
        m_pfn.vkQueueEndDebugUtilsLabelEXT        = VULKAN_DEVICE_FUNC_LOAD(m_device, vkQueueEndDebugUtilsLabelEXT);
        m_pfn.vkQueueInsertDebugUtilsLabelEXT     = VULKAN_DEVICE_FUNC_LOAD(m_device, vkQueueInsertDebugUtilsLabelEXT);
        m_pfn.vkSetDebugUtilsObjectNameEXT        = VULKAN_DEVICE_FUNC_LOAD(m_device, vkSetDebugUtilsObjectNameEXT);
        m_pfn.vkSetDebugUtilsObjectTagEXT         = VULKAN_DEVICE_FUNC_LOAD(m_device, vkSetDebugUtilsObjectTagEXT);
        m_pfn.vkCreateRayTracingPipelinesKHR      = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCreateRayTracingPipelinesKHR);
        m_pfn.vkCmdTraceRaysIndirect2KHR          = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdTraceRaysIndirect2KHR);
        m_pfn.vkCmdPushDescriptorSet2KHR          = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdPushDescriptorSet2KHR);
        m_pfn.vkCmdTraceRaysKHR                   = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdTraceRaysKHR);
        m_pfn.vkCmdDrawMeshTasksEXT               = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdDrawMeshTasksEXT);
        m_pfn.vkCmdDrawMeshTasksIndirectEXT       = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdDrawMeshTasksIndirectEXT);
        m_pfn.vkCmdDrawMeshTasksIndirectCountEXT  = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdDrawMeshTasksIndirectCountEXT);
        m_pfn.vkCmdBeginConditionalRenderingEXT   = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdBeginConditionalRenderingEXT);
        m_pfn.vkCmdEndConditionalRenderingEXT     = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdEndConditionalRenderingEXT);
        m_pfn.vkCmdBuildAccelerationStructuresKHR = VULKAN_DEVICE_FUNC_LOAD(m_device, vkCmdBuildAccelerationStructuresKHR);

        // VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProperties = {};
        // VkPhysicalDeviceRayQueryFeaturesKHR             rayQueryFeatures     = {};

        // clang-format off
        VkPhysicalDeviceMeshShaderPropertiesEXT  meshShadersFeatures      = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT,     .pNext = nullptr                   };
        VkPhysicalDevicePushDescriptorProperties pushDescriptorProperties = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR, .pNext = &meshShadersFeatures      };
        VkPhysicalDeviceVulkan13Properties       deviceProperties13       = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES,          .pNext = &pushDescriptorProperties };
        VkPhysicalDeviceVulkan12Properties       deviceProperties12       = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES,          .pNext = &deviceProperties13       };
        VkPhysicalDeviceVulkan11Properties       deviceProperties11       = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES,          .pNext = &deviceProperties12       };
        VkPhysicalDeviceProperties2              deviceProperties         = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,                   .pNext = &deviceProperties11       };
        // clang-format on
        vkGetPhysicalDeviceProperties2(m_physicalDevice, &deviceProperties);

        // Fill DeviceLimits
        {
            m_limits.minUniformBufferOffsetAlignment = uint32_t(deviceProperties.properties.limits.minUniformBufferOffsetAlignment);
            m_limits.minStorageBufferOffsetAlignment = uint32_t(deviceProperties.properties.limits.minStorageBufferOffsetAlignment);
        }

        result = m_queue[(uint32_t)QueueType::Graphics].Init(this, "Graphics", graphicsQueueFamilyIndex, 0);
        VkResultTry(result);

        if (computeQueueFamilyIndex)
        {
            result = m_queue[(uint32_t)QueueType::Compute].Init(this, "Compute", computeQueueFamilyIndex, 0);
            VkResultTry(result);
        }

        if (transferQueueFamilyIndex)
        {
            result = m_queue[(uint32_t)QueueType::Transfer].Init(this, "Transfer", transferQueueFamilyIndex, 0);
            VkResultTry(result);
        }

        result = m_bindGroupAllocator.Init(this);
        VkResultTry(result);
        return result;
    }

    void IDevice::Shutdown()
    {
        ZoneScoped;

        // Report live object stack tracecs
        {
            for (auto [ptr, stacktrace] : m_liveCommandPools)
            {
                TL_LOG_WARNNING("Leaked: RHI::CommandPool at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyCommandPool(ptr);
            }

            for (auto [ptr, stacktrace] : m_liveFences)
            {
                TL_LOG_WARNNING("Leaked: RHI::Fence at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyFence(ptr);
            }

            for (auto [ptr, stacktrace] : m_liveSwapchains)
            {
                TL_LOG_WARNNING("Leaked: RHI::Swapchain at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroySwapchain(ptr);
            }

            for (auto [ptr, stacktrace] : m_liveShaderModules)
            {
                TL_LOG_WARNNING("Leaked: RHI::ShaderModule at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyShaderModule(ptr);
            }

            for (auto [ptr, stacktrace] : m_liveImages)
            {
                TL_LOG_WARNNING("Leaked: RHI::Image at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyImage(ptr);
            }

            for (auto [ptr, stacktrace] : m_liveBuffers)
            {
                TL_LOG_WARNNING("Leaked: RHI::Buffer at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyBuffer(ptr);
            }

            for (auto [ptr, stacktrace] : m_liveBindGroupLayouts)
            {
                TL_LOG_WARNNING("Leaked: RHI::BindGroupLayout at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyBindGroupLayout(ptr);
            }

            for (auto [ptr, stacktrace] : m_liveBindGroups)
            {
                TL_LOG_WARNNING("Leaked: RHI::BindGroup at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyBindGroup(ptr);
            }

            for (auto [ptr, stacktrace] : m_livePipelineLayouts)
            {
                TL_LOG_WARNNING("Leaked: RHI::PipelineLayout at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyPipelineLayout(ptr);
            }

            for (auto [ptr, stacktrace] : m_liveGraphicsPipelines)
            {
                TL_LOG_WARNNING("Leaked: RHI::GraphicsPipeline at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyGraphicsPipeline(ptr);
            }

            for (auto [ptr, stacktrace] : m_liveComputePipelines)
            {
                TL_LOG_WARNNING("Leaked: RHI::ComputePipeline at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyComputePipeline(ptr);
            }

            for (auto [ptr, stacktrace] : m_liveSamplers)
            {
                TL_LOG_WARNNING("Leaked: RHI::Sampler at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroySampler(ptr);
            }
        }

        m_destroyQueue->shutdown(this);
        m_bindGroupAllocator.Shutdown();

        m_queue[(int)QueueType::Transfer].Shutdown();
        m_queue[(int)QueueType::Compute].Shutdown();
        m_queue[(int)QueueType::Graphics].Shutdown();

        vmaDestroyAllocator(m_deviceAllocator);
        vkDestroyDevice(m_device, nullptr);
        if (m_debugUtilsMessenger != VK_NULL_HANDLE)
        {
            m_pfn.vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugUtilsMessenger, nullptr);
        }
        vkDestroyInstance(m_instance, nullptr);
    }

    void IDevice::SetDebugName(VkObjectType type, uint64_t handle, const char* name) const
    {
        if (handle == 0 /* VK_NULL_HANDLE */) return;

        if (auto fn = m_pfn.vkSetDebugUtilsObjectNameEXT; fn && name)
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

    uint64_t IDevice::GetNativeHandle(NativeHandleType type, uint64_t _resource)
    {
        switch (type)
        {
        case NativeHandleType::None: return 0;
        case NativeHandleType::Device:
            {
                auto resource = (IDevice*)_resource;
                return (uint64_t)resource->m_device;
            }
        case NativeHandleType::CommandList:
            {
                auto resource = (ICommandList*)_resource;
                return (uint64_t)resource->m_commandBuffer;
            }
        case NativeHandleType::Buffer:
            {
                auto resource = (IBuffer*)_resource;
                return (uint64_t)resource->handle;
            }
        case NativeHandleType::Image:
            {
                auto resource = (IImage*)_resource;
                return (uint64_t)resource->handle;
            }
        case NativeHandleType::ImageView:
            {
                auto resource = (IImage*)_resource;
                return (uint64_t)resource->viewHandle;
            }
        case NativeHandleType::Sampler:
            {
                auto resource = (ISampler*)_resource;
                return (uint64_t)resource->handle;
            }
        case NativeHandleType::ShaderModule:
            {
                auto resource = (IShaderModule*)_resource;
                return (uint64_t)resource->m_shaderModule;
            }
        case NativeHandleType::Pipeline:
            {
                auto resource = (IGraphicsPipeline*)_resource;
                return (uint64_t)resource->handle;
            }
        case NativeHandleType::PipelineLayout:
            {
                auto resource = (IPipelineLayout*)_resource;
                return (uint64_t)resource->handle;
            }
        case NativeHandleType::BindGroupLayout:
            {
                auto resource = (IBindGroupLayout*)_resource;
                return (uint64_t)resource->handle;
            }
        case NativeHandleType::BindGroup:
            {
                auto resource = (IBindGroup*)_resource;
                return (uint64_t)resource->descriptorSet;
            }
        case NativeHandleType::Swapchain:
            {
                auto resource = (ISwapchain*)_resource;
                return (uint64_t)resource->m_swapchain;
            }
        default:
            TL_UNREACHABLE_MSG("Unknown NativeHandleType");
        }
        return 0;
    }

    void IDevice::UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo)
    {
        ZoneScoped;
        auto bindGroup = (IBindGroup*)(handle);
        bindGroup->Update(this, updateInfo);
    }

    Queue* IDevice::GetQueue(QueueType queueType)
    {
        return &m_queue[(int)queueType];
    }

    uint64_t IDevice::GetFenceValue(Fence* _fence)
    {
        IFence* fence = (IFence*)_fence;

        uint64_t value;
        vkGetSemaphoreCounterValue(m_device, fence->semaphore, &value);
        return value;
    }

    uint64_t IDevice::GarbageCollect(uint64_t graphicsTimeline)
    {
        m_arena.reset();
        m_destroyQueue->Flush(this, graphicsTimeline);
        return graphicsTimeline;
    }

    uint64_t IDevice::GetBufferDeviceAddress(Buffer* _buffer)
    {
        IBuffer* buffer = (IBuffer*)_buffer;
        TL_ASSERT(buffer->address != 0, "Buffer is not shader addressable");
        return buffer->address;
    }

    template<typename Resource, typename... Args>
    inline Resource* createImpl(IDevice* device, TL::Map<Resource*, TL::Stacktrace>& liveResources, Args... args)
    {
        Resource*  resource = TL ::construct<Resource>();
        ResultCode result   = resource->Init(device, args...);
        if (IsSuccess(result))
        {
            liveResources.emplace(resource, TL::CaptureStacktrace());
            return resource;
        }
        return nullptr;
    }

    template<typename Resource>
    inline void destroyImpl(IDevice* device, Resource* resource, TL::Map<Resource*, TL::Stacktrace>& liveResources)
    {
        auto erased = liveResources.erase(resource);
        TL_ASSERT(erased);
        resource->Shutdown(device);
    }

    // clang-format off
    // interface implementation

    CommandPool*        IDevice::CreateCommandPool(const CommandPoolCreateInfo& createInfo)                { return createImpl<ICommandPool>(this, this->m_liveCommandPools, createInfo);                             }
    void                IDevice::DestroyCommandPool(CommandPool* resource)                                 { destroyImpl<ICommandPool>(this, (ICommandPool*)resource, this->m_liveCommandPools);                      }
    Fence*              IDevice::CreateFence(const FenceCreateInfo& createInfo)                            { return createImpl<IFence>(this, this->m_liveFences, createInfo);                                         }
    void                IDevice::DestroyFence(Fence* resource)                                             { destroyImpl<IFence>(this, (IFence*)resource, this->m_liveFences);                                        }
    Swapchain*          IDevice::CreateSwapchain(const SwapchainCreateInfo& createInfo)                    { return createImpl<ISwapchain>(this, this->m_liveSwapchains, createInfo);                                 }
    void                IDevice::DestroySwapchain(Swapchain* resource)                                     { destroyImpl<ISwapchain>(this, (ISwapchain*)resource, this->m_liveSwapchains);                            }
    ShaderModule*       IDevice::CreateShaderModule(const ShaderModuleCreateInfo& createInfo)              { return createImpl<IShaderModule>(this, this->m_liveShaderModules, createInfo);                           }
    void                IDevice::DestroyShaderModule(ShaderModule* resource)                               { destroyImpl<IShaderModule>(this, (IShaderModule*)resource, this->m_liveShaderModules);                   }
    BindGroupLayout*    IDevice::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)        { return createImpl<IBindGroupLayout>(this, this->m_liveBindGroupLayouts, createInfo);                     }
    void                IDevice::DestroyBindGroupLayout(BindGroupLayout* resource)                         { destroyImpl<IBindGroupLayout>(this, (IBindGroupLayout*)resource, this->m_liveBindGroupLayouts);          }
    BindGroup*          IDevice::CreateBindGroup(const BindGroupCreateInfo& createInfo)                    { return createImpl<IBindGroup>(this, this->m_liveBindGroups, createInfo);                                 }
    void                IDevice::DestroyBindGroup(BindGroup* resource)                                     { destroyImpl<IBindGroup>(this, (IBindGroup*)resource, this->m_liveBindGroups);                            }
    QueryPool*          IDevice::CreateQueryPool(const QueryPoolCreateInfo& createInfo)                    { return createImpl<IQueryPool>(this, this->m_liveQueryPools, createInfo);                                 }
    void                IDevice::DestroyQueryPool(QueryPool* resource)                                     { destroyImpl<IQueryPool>(this, (IQueryPool*)resource, this->m_liveQueryPools);                            }
    Buffer*             IDevice::CreateBuffer(const BufferCreateInfo& createInfo)                          { return createImpl<IBuffer>(this, this->m_liveBuffers, createInfo);                                       }
    void                IDevice::DestroyBuffer(Buffer* resource)                                           { destroyImpl<IBuffer>(this, (IBuffer*)resource, this->m_liveBuffers);                                     }
    Image*              IDevice::CreateImage(const ImageCreateInfo& createInfo)                            { return createImpl<IImage>(this, this->m_liveImages, createInfo);                                         }
    Image*              IDevice::CreateImageView(const ImageViewCreateInfo& createInfo)                    { return createImpl<IImage>(this, this->m_liveImages, createInfo);                                         }
    void                IDevice::DestroyImage(Image* resource)                                             { destroyImpl<IImage>(this, (IImage*)resource, this->m_liveImages);                                        }
    Sampler*            IDevice::CreateSampler(const SamplerCreateInfo& createInfo)                        { return createImpl<ISampler>(this, this->m_liveSamplers, createInfo);                                     }
    void                IDevice::DestroySampler(Sampler* resource)                                         { destroyImpl<ISampler>(this, (ISampler*)resource, this->m_liveSamplers);                                  }
    PipelineLayout*     IDevice::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)          { return createImpl<IPipelineLayout>(this, this->m_livePipelineLayouts, createInfo);                       }
    void                IDevice::DestroyPipelineLayout(PipelineLayout* resource)                           { destroyImpl<IPipelineLayout>(this, (IPipelineLayout*)resource, this->m_livePipelineLayouts);             }
    GraphicsPipeline*   IDevice::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)      { return createImpl<IGraphicsPipeline>(this, this->m_liveGraphicsPipelines, createInfo);                   }
    void                IDevice::DestroyGraphicsPipeline(GraphicsPipeline* resource)                       { destroyImpl<IGraphicsPipeline>(this, (IGraphicsPipeline*)resource, this->m_liveGraphicsPipelines);       }
    RayTracingPipeline* IDevice::CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo)  { return createImpl<IRayTracingPipeline>(this, this->m_liveRayTracingPipelines, createInfo);               }
    void                IDevice::DestroyRayTracingPipeline(RayTracingPipeline* resource)                   { destroyImpl<IRayTracingPipeline>(this, (IRayTracingPipeline*)resource, this->m_liveRayTracingPipelines); }
    ComputePipeline*    IDevice::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)        { return createImpl<IComputePipeline>(this, this->m_liveComputePipelines, createInfo);                     }
    void                IDevice::DestroyComputePipeline(ComputePipeline* resource)                         { destroyImpl<IComputePipeline>(this, (IComputePipeline*)resource, this->m_liveComputePipelines);          }

    // clang-format on

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

    template<typename ResourceType>
    inline static void destroyVkResource(IDevice* device, ResourceType handle)
    {
        if constexpr (std::is_same_v<VmaAllocation, ResourceType>) vmaFreeMemory(device->m_deviceAllocator, handle);
        else if constexpr (std::is_same_v<VkBuffer, ResourceType>) vkDestroyBuffer(device->m_device, handle, nullptr);
        else if constexpr (std::is_same_v<VkBufferView, ResourceType>) vkDestroyBufferView(device->m_device, handle, nullptr);
        else if constexpr (std::is_same_v<VkImage, ResourceType>) vkDestroyImage(device->m_device, handle, nullptr);
        else if constexpr (std::is_same_v<VkImageView, ResourceType>) vkDestroyImageView(device->m_device, handle, nullptr);
        else if constexpr (std::is_same_v<VkSampler, ResourceType>) vkDestroySampler(device->m_device, handle, nullptr);
        else if constexpr (std::is_same_v<VkPipeline, ResourceType>) vkDestroyPipeline(device->m_device, handle, nullptr);
        else if constexpr (std::is_same_v<VkDescriptorPool, ResourceType>) vkDestroyDescriptorPool(device->m_device, handle, nullptr);
        else if constexpr (std::is_same_v<VkQueryPool, ResourceType>) vkDestroyQueryPool(device->m_device, handle, nullptr);
        else if constexpr (std::is_same_v<VkSemaphore, ResourceType>) vkDestroySemaphore(device->m_device, handle, nullptr);
        else if constexpr (std::is_same_v<VkSwapchainKHR, ResourceType>) vkDestroySwapchainKHR(device->m_device, handle, nullptr);
        else if constexpr (std::is_same_v<VkSurfaceKHR, ResourceType>) vkDestroySurfaceKHR(device->m_instance, handle, nullptr);
        else if constexpr (std::is_same_v<VmaBufferAllocation, ResourceType>) vmaDestroyBuffer(device->m_deviceAllocator, handle.first, handle.second);
        else if constexpr (std::is_same_v<VmaImageAllocation, ResourceType>) vmaDestroyImage(device->m_deviceAllocator, handle.first, handle.second);
        else
        {
            static_assert(false, "Invalid ResourceType");
        }
    }

    // FlushQueue that works on ResourceDeleteQueueEntry<ResourceType>
    template<typename ResourceType>
    void DeleteQueue::FlushQueue(IDevice* device, TL::Vector<ResourceDeleteQueueEntry<ResourceType>>& queue, uint64_t timeline)
    {
        uint32_t deleteCount = 0;
        for (const auto& entry : queue)
        {
            if (entry.timeline > timeline)
                break;

            destroyVkResource(device, entry.resource);

            uint64_t key = TL::hashBytes(TL::Block::create(entry.resource));
            TL_ASSERT(m_pending.erase(key));
            deleteCount++;
        }
        queue.erase(queue.begin(), queue.begin() + deleteCount);
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