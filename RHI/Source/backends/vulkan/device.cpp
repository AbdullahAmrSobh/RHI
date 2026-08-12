#define VOLK_IMPLEMENTATION

#include <TL/Assert.hpp>
#define VMA_ASSERT(expr) TL_ASSERT(expr)
#define VMA_IMPLEMENTATION

#include "device.h"
#include "common.h"

#include <TL/Log.hpp>
#include <TL/Containers/Optional.hpp>
#include <TL/Allocator/Allocator.hpp>
#include <TL/Containers/Map.hpp>

#include <algorithm>
#include <format>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    Device* createDevice(const ApplicationInfo& appInfo)
    {
        ZoneScoped;
        auto device = TL::constructFrom<IDevice>(TL::Context::getDefaultAllocator());
        auto result = device->Init(appInfo);
        TL_ASSERT(IsSuccess(result));
        return device;
    }

    void destroyDevice(Device* _device)
    {
        auto device = (IDevice*)_device;
        device->Shutdown();
        TL::destructFrom(TL::Context::getDefaultAllocator(), device);
    }

    inline static const char* ObjectTypeToName(VkObjectType type)
    {
        switch (type)
        {
        case VK_OBJECT_TYPE_UNKNOWN: return "UNKNOWN";
        case VK_OBJECT_TYPE_INSTANCE: return "VkInstance";
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE: return "VkPhysicalDevice";
        case VK_OBJECT_TYPE_DEVICE: return "VkDevice";
        case VK_OBJECT_TYPE_QUEUE: return "VkQueue";
        case VK_OBJECT_TYPE_SEMAPHORE: return "VkSemaphore";
        case VK_OBJECT_TYPE_COMMAND_BUFFER: return "VkCommandBuffer";
        case VK_OBJECT_TYPE_FENCE: return "VkFence";
        case VK_OBJECT_TYPE_DEVICE_MEMORY: return "VkDeviceMemory";
        case VK_OBJECT_TYPE_BUFFER: return "VkBuffer";
        case VK_OBJECT_TYPE_IMAGE: return "VkImage";
        case VK_OBJECT_TYPE_EVENT: return "VkEvent";
        case VK_OBJECT_TYPE_QUERY_POOL: return "VkQueryPool";
        case VK_OBJECT_TYPE_BUFFER_VIEW: return "VkBufferView";
        case VK_OBJECT_TYPE_IMAGE_VIEW: return "VkImageView";
        case VK_OBJECT_TYPE_SHADER_MODULE: return "VkShaderModule";
        case VK_OBJECT_TYPE_PIPELINE_CACHE: return "VkPipelineCache";
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT: return "VkPipelineLayout";
        case VK_OBJECT_TYPE_RENDER_PASS: return "VkRenderPass";
        case VK_OBJECT_TYPE_PIPELINE: return "VkPipeline";
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: return "VkDescriptorSetLayout";
        case VK_OBJECT_TYPE_SAMPLER: return "VkSampler";
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL: return "VkDescriptorPool";
        case VK_OBJECT_TYPE_DESCRIPTOR_SET: return "VkDescriptorSet";
        case VK_OBJECT_TYPE_FRAMEBUFFER: return "VkFramebuffer";
        case VK_OBJECT_TYPE_COMMAND_POOL: return "VkCommandPool";
        case VK_OBJECT_TYPE_SURFACE_KHR: return "VkSurfaceKHR";
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR: return "VkSwapchainKHR";
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT: return "VkDebugReportCallbackEXT";
        case VK_OBJECT_TYPE_DISPLAY_KHR: return "VkDisplayKHR";
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: return "VkDisplayModeKHR";
        case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT: return "VkValidationCacheEXT";
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION: return "VkSamplerYcbcrConversion";
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE: return "VkDescriptorUpdateTemplate";
        case VK_OBJECT_TYPE_CU_MODULE_NVX: return "VkCuModuleNVX";
        case VK_OBJECT_TYPE_CU_FUNCTION_NVX: return "VkCuFunctionNVX";
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR: return "VkAccelerationStructureKHR";
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV: return "VkAccelerationStructureNV";
        default: return "UNKNOWN";
        };
    }

    inline static VkBool32 DebugMessengerCallbacks(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
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
                auto [r, g, b, a] = pCallbackData->pCmdBufLabels[i].color;
                message += std::format("  [{}] {} (color: [{:.2f}, {:.2f}, {:.2f}, {:.2f}])\n", i, labelName, r, g, b, a);
            }
        }

        if (pCallbackData->queueLabelCount > 0)
        {
            message += "Queue Labels:\n";
            for (uint32_t i = 0; i < pCallbackData->queueLabelCount; ++i)
            {
                const char* labelName = pCallbackData->pQueueLabels[i].pLabelName;
                auto [r, g, b, a] = pCallbackData->pQueueLabels[i].color;
                message += std::format("  [{}] {} (color: [{:.2f}, {:.2f}, {:.2f}, {:.2f}])\n", i, labelName, r, g, b, a);
            }
        }

        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: TL::LogInfo("{}", message); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: TL::LogInfo("{}", message); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: TL::LogWarn("{}", message); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: TL::LogError("{}", message); break;
        default: TL_UNREACHABLE();
        }

        return VK_FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IQueue
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    VkResult IQueue::Init(IDevice* device, const char* debugName, uint32_t familyIndex, uint32_t queueIndex)
    {
        m_device = device;
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

    void queueBeginAnnotation(IQueue* self, const char* name, uint32_t bgra)
    {
        if (vkQueueBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT label = MakeDebugLabel(name, bgra);
            vkQueueBeginDebugUtilsLabelEXT(self->m_queue, &label);
        }
    }

    void queueEndAnnotation(IQueue* self)
    {
        if (vkQueueEndDebugUtilsLabelEXT)
        {
            vkQueueEndDebugUtilsLabelEXT(self->m_queue);
        }
    }

    void queueInsertAnnotation(IQueue* self, const char* name, uint32_t bgra)
    {
        if (vkQueueInsertDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT label = MakeDebugLabel(name, bgra);
            vkQueueInsertDebugUtilsLabelEXT(self->m_queue, &label);
        }
    }

    void queueSubmit(IQueue* self, const QueueSubmitInfo& submitInfo)
    {
        TL::Vector<VkSemaphoreSubmitInfo> waitSemaphores{self->m_device->m_arena};
        TL::Vector<VkCommandBufferSubmitInfo> commandBufferSubmitInfos{self->m_device->m_arena};
        TL::Vector<VkSemaphoreSubmitInfo> signalSemaphores{self->m_device->m_arena};

        for (auto _fence : submitInfo.waitFences)
        {
            auto fence = (IFence*)_fence.fence;
            waitSemaphores.push_back({
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = fence->semaphore,
                .value = _fence.value,
                .stageMask = ConvertPipelineStageFlags(_fence.stage),
            });
        }

        for (auto _fence : submitInfo.signalFences)
        {
            auto fence = (IFence*)_fence.fence;
            signalSemaphores.push_back({
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = fence->semaphore,
                .value = _fence.value,
                .stageMask = ConvertPipelineStageFlags(_fence.stage),
            });
        }

        for (auto cmd : submitInfo.commandLists)
        {
            auto commandList = (ICommandList*)cmd;
            commandBufferSubmitInfos.push_back({
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = commandList->commandBuffer,
            });
        }

        for (auto _swapchain : submitInfo.presentSwapchains)
        {
            ISwapchain* swapchain = (ISwapchain*)_swapchain;

            VkSemaphore presentSemaphore = swapchain->m_presentSemaphore[swapchain->m_presentSemaphoreIndex];
            signalSemaphores.push_back({
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = presentSemaphore,
            });
        }

        // TODO(cleanup): m_lastSubmitValue is never incremented, so the timeline-based DeleteQueue
        // deferral is effectively inert. Left for a dedicated GPU-sync pass (needs runtime testing).

        VkSubmitInfo2 vksubmitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext = nullptr,
            .flags = {},
            .waitSemaphoreInfoCount = (uint32_t)waitSemaphores.size(),
            .pWaitSemaphoreInfos = waitSemaphores.data(),
            .commandBufferInfoCount = (uint32_t)commandBufferSubmitInfos.size(),
            .pCommandBufferInfos = commandBufferSubmitInfos.data(),
            .signalSemaphoreInfoCount = (uint32_t)signalSemaphores.size(),
            .pSignalSemaphoreInfos = signalSemaphores.data(),
        };
        VK_CHECK(vkQueueSubmit2(self->m_queue, 1, &vksubmitInfo, VK_NULL_HANDLE));

        // TODO(cleanup): these two vkQueueWaitIdle calls fully serialize the GPU every submit,
        // defeating the timeline-semaphore design. Left for a dedicated GPU-sync pass.
        vkQueueWaitIdle(self->m_queue);

        if (submitInfo.presentSwapchains.empty() == false)
        {
            TL::Vector<VkSwapchainKHR> swapchains{self->m_device->m_arena};
            TL::Vector<uint32_t> imageIndices{self->m_device->m_arena};
            TL::Vector<VkSemaphore> presentWaitSemaphores{self->m_device->m_arena};

            for (auto _swapchain : submitInfo.presentSwapchains)
            {
                ISwapchain* swapchain = (ISwapchain*)_swapchain;
                VkSemaphore semaphore = swapchain->m_presentSemaphore[swapchain->m_presentSemaphoreIndex];
                swapchain->m_presentSemaphoreIndex += 1;
                swapchain->m_presentSemaphoreIndex %= ISwapchain::MaxImageCount;
                presentWaitSemaphores.push_back(semaphore);
                imageIndices.push_back(swapchain->m_imageIndex);
                swapchains.push_back(swapchain->m_swapchain);
            }

            VkPresentInfoKHR presentInfos{
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .pNext = 0,
                .waitSemaphoreCount = (uint32_t)presentWaitSemaphores.size(),
                .pWaitSemaphores = presentWaitSemaphores.data(),
                .swapchainCount = (uint32_t)swapchains.size(),
                .pSwapchains = swapchains.data(),
                .pImageIndices = imageIndices.data(),
                .pResults = nullptr,
            };
            vkQueuePresentKHR(self->m_queue, &presentInfos);
        }

        vkQueueWaitIdle(self->m_queue);

        for (auto _swapchain : submitInfo.presentSwapchains)
        {
            ISwapchain* swapchain = (ISwapchain*)_swapchain;
            swapchain->AcquireNextImage(self->m_device);
        }
    }

    void queueWaitIdle(IQueue* self)
    {
        vkQueueWaitIdle(self->m_queue);
    }

    void queueWaitFence(IQueue* self, Fence* _fence, uint64_t value)
    {
        IFence* fence = (IFence*)_fence;
        VkSemaphoreWaitInfo waitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .pNext = nullptr,
            .flags = 0,
            .semaphoreCount = 1,
            .pSemaphores = &fence->semaphore,
            .pValues = &value,
        };
        VK_CHECK(vkWaitSemaphores(self->m_device->m_device, &waitInfo, UINT64_MAX));
    }

    ///

    IDevice::IDevice()
    {
        m_objectAllocator = TL::Context::getDefaultAllocator();
        m_destroyQueue    = TL::CreatePtr<DeleteQueue>();
    }

    IDevice::~IDevice() = default;

    ResultCode IDevice::Init(const ApplicationInfo& appInfo)
    {
        ZoneScoped;

        m_backend = BackendType::Vulkan1_3;

        VulkanResult result;

        VK_CHECK(volkInitialize());

        constexpr bool EnableAsyncQueues = true;

        TL::Map<TL::String, VkLayerProperties> availableInstanceLayers;
        TL::Map<TL::String, VkExtensionProperties> availableInstanceExtensions;

        uint32_t instanceLayerCount;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
        TL::Vector<VkLayerProperties> instanceLayers;
        instanceLayers.resize(instanceLayerCount);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayers.data()));

        for (VkLayerProperties layer : instanceLayers)
            availableInstanceLayers[layer.layerName] = layer;

        {
            uint32_t instanceExtensionsCount;
            VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, nullptr));
            TL::Vector<VkExtensionProperties> extensions;
            extensions.resize(instanceExtensionsCount);
            VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, extensions.data()));
            for (VkExtensionProperties extension : extensions)
                availableInstanceExtensions[extension.extensionName] = extension;
        }

        TL::Vector<const char*> requiredInstanceLayers;
        TL::Vector<const char*> requiredInstanceExtensions{
            VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
        };

        if constexpr (RHI_DEBUG)
        {
            requiredInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
            requiredInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

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
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = DebugMessengerCallbacks,
            .pUserData = this,
        };
        VkInstanceCreateInfo instanceCI{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = RHI_DEBUG ? &debugUtilsCI : nullptr,
            .flags = {},
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = (uint32_t)requiredInstanceLayers.size(),
            .ppEnabledLayerNames = requiredInstanceLayers.data(),
            .enabledExtensionCount = (uint32_t)requiredInstanceExtensions.size(),
            .ppEnabledExtensionNames = requiredInstanceExtensions.data(),
        };

        result = vkCreateInstance(&instanceCI, nullptr, &m_instance);
        VkResultTry(result);

        volkLoadInstanceOnly(m_instance);

        // Select the physical device

        TL::Vector<const char*> requiredDeviceLayers;
        TL::Vector<const char*> requiredDeviceExtensions{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            // VK_KHR_CALIBRATED_TIMESTAMPS_EXTENSION_NAME,
        };

        bool enablePushDescriptors = true;
        bool enableMeshShaders = true;
        bool enableRayTracing = true;
        bool enableDeviceGeneratedCommands = true;
        bool enableRobustness = appInfo.enableVulkanRobustness;

        if (enablePushDescriptors)
        {
            requiredDeviceExtensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
            requiredDeviceExtensions.push_back(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);
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

        if (enableRobustness)
        {
            requiredDeviceExtensions.push_back(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
            requiredDeviceExtensions.push_back(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);
        }

        {
            uint32_t physicalDeviceCount;
            VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr));
            TL::Vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
            VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data()));
            for (VkPhysicalDevice physicalDevice : physicalDevices)
            {
                TL::Map<TL::String, VkLayerProperties> availableDeviceLayers;
                {
                    uint32_t deviceLayerCount;
                    VK_CHECK(vkEnumerateDeviceLayerProperties(physicalDevice, &deviceLayerCount, nullptr));
                    TL::Vector<VkLayerProperties> layers;
                    layers.resize(deviceLayerCount);
                    VK_CHECK(vkEnumerateDeviceLayerProperties(physicalDevice, &deviceLayerCount, layers.data()));
                    for (VkLayerProperties layer : layers)
                        availableDeviceLayers[layer.layerName] = layer;
                }

                TL::Map<TL::String, VkExtensionProperties> availableDeviceExtensions;
                {
                    uint32_t extensionsCount;
                    VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr));
                    TL::Vector<VkExtensionProperties> extensions;
                    extensions.resize(extensionsCount);
                    VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, extensions.data()));
                    for (VkExtensionProperties extension : extensions)
                        availableDeviceExtensions[extension.extensionName] = extension;
                }

                // search for a suitable physical device if it contains the required extensions
                bool containAllLayers = std::all_of(requiredDeviceLayers.begin(), requiredDeviceLayers.end(), [&](const char* layer)
                    {
                        return availableDeviceLayers.contains(layer);
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
                TL::LogError("RHI Vulkan: No suitable physical device found.");
                return ResultCode::ErrorUnknown;
            }
        }

        uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t transferQueueFamilyIndex = UINT32_MAX;
        uint32_t computeQueueFamilyIndex = UINT32_MAX;

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
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCount = 1,
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
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT,
            .pNext = pNext,
            .deviceGeneratedCommands = VK_TRUE,
            .dynamicGeneratedPipelineLayout = VK_TRUE,
        };
        if (enableDeviceGeneratedCommands) pNext = &deviceGeneratedCommandsFeatures;

        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
            .pNext = pNext,
            .taskShader = VK_TRUE,
            .meshShader = VK_TRUE,
            .multiviewMeshShader = VK_FALSE,
            .primitiveFragmentShadingRateMeshShader = VK_FALSE,
            .meshShaderQueries = VK_TRUE,
        };
        if (enableMeshShaders) pNext = &meshShaderFeatures;

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
            .pNext = pNext,
            .rayTracingPipeline = VK_TRUE,
            .rayTracingPipelineShaderGroupHandleCaptureReplay = VK_TRUE,
            .rayTracingPipelineShaderGroupHandleCaptureReplayMixed = VK_FALSE,
            .rayTracingPipelineTraceRaysIndirect = VK_TRUE,
            .rayTraversalPrimitiveCulling = VK_TRUE,
        };
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
            .pNext = &rayTracingPipelineFeatures,
            .accelerationStructure = VK_TRUE,
            .accelerationStructureCaptureReplay = VK_TRUE,
            .accelerationStructureIndirectBuild = VK_FALSE, // TODO: test
            .accelerationStructureHostCommands = VK_FALSE,  // TODO: test
            .descriptorBindingAccelerationStructureUpdateAfterBind = VK_TRUE,
        };
        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR,
            .pNext = &accelerationStructureFeatures,
            .rayQuery = VK_TRUE,
        };
        VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR rayTracingPositionFetchFeaturesKHR{

            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR,
            .pNext = &rayQueryFeatures,
            .rayTracingPositionFetch = VK_TRUE,
        };

        if (enableRayTracing) pNext = &rayTracingPositionFetchFeaturesKHR;

        VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
            .pNext = pNext,
            .robustBufferAccess2 = VK_TRUE,
            .robustImageAccess2  = VK_TRUE,
            .nullDescriptor      = VK_TRUE,
        };
        VkPhysicalDevicePipelineRobustnessFeaturesEXT pipelineRobustnessFeatures{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES_EXT,
            .pNext = &robustness2Features,
            .pipelineRobustness = VK_TRUE,
        };
        if (enableRobustness) pNext = &pipelineRobustnessFeatures;

        VkPhysicalDeviceVulkan13Features features13{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = pNext,
            .robustImageAccess = enableRobustness ? VK_TRUE : VK_FALSE,
            .inlineUniformBlock = VK_FALSE,
            .descriptorBindingInlineUniformBlockUpdateAfterBind = VK_FALSE,
            .pipelineCreationCacheControl = VK_FALSE,
            .privateData = VK_FALSE,
            .shaderDemoteToHelperInvocation = VK_FALSE,
            .shaderTerminateInvocation = VK_FALSE,
            .subgroupSizeControl = VK_FALSE,
            .computeFullSubgroups = VK_FALSE,
            .synchronization2 = VK_TRUE,
            .textureCompressionASTC_HDR = VK_FALSE,
            .shaderZeroInitializeWorkgroupMemory = VK_FALSE,
            .dynamicRendering = VK_TRUE,
            .shaderIntegerDotProduct = VK_FALSE,
            .maintenance4 = VK_FALSE,
        };
        VkPhysicalDeviceVulkan12Features features12{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &features13,
            .samplerMirrorClampToEdge = VK_FALSE,
            .drawIndirectCount = VK_TRUE,
            .storageBuffer8BitAccess = VK_FALSE,
            .uniformAndStorageBuffer8BitAccess = VK_TRUE,
            .storagePushConstant8 = VK_TRUE,
            .shaderBufferInt64Atomics = VK_FALSE,
            .shaderSharedInt64Atomics = VK_FALSE,
            .shaderFloat16 = VK_FALSE,
            .shaderInt8 = VK_TRUE,
            .descriptorIndexing = VK_TRUE,
            .shaderInputAttachmentArrayDynamicIndexing = VK_FALSE,
            .shaderUniformTexelBufferArrayDynamicIndexing = VK_FALSE,
            .shaderStorageTexelBufferArrayDynamicIndexing = VK_FALSE,
            .shaderUniformBufferArrayNonUniformIndexing = VK_FALSE,
            .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
            .shaderStorageBufferArrayNonUniformIndexing = VK_FALSE,
            .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
            .shaderInputAttachmentArrayNonUniformIndexing = VK_TRUE,
            .shaderUniformTexelBufferArrayNonUniformIndexing = VK_FALSE,
            .shaderStorageTexelBufferArrayNonUniformIndexing = VK_FALSE,
            .descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
            .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
            .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
            .descriptorBindingPartiallyBound = VK_TRUE,
            .descriptorBindingVariableDescriptorCount = VK_TRUE,
            .runtimeDescriptorArray = VK_TRUE,
            .samplerFilterMinmax = VK_FALSE,
            .scalarBlockLayout = VK_FALSE,
            .imagelessFramebuffer = VK_FALSE,
            .uniformBufferStandardLayout = VK_FALSE,
            .shaderSubgroupExtendedTypes = VK_FALSE,
            .separateDepthStencilLayouts = VK_FALSE,
            .hostQueryReset = VK_FALSE,
            .timelineSemaphore = VK_TRUE,
            .bufferDeviceAddress = VK_TRUE,
            .bufferDeviceAddressCaptureReplay = RHI_DEBUG ? VK_TRUE : VK_FALSE,
            .bufferDeviceAddressMultiDevice = VK_FALSE,
            .vulkanMemoryModel = VK_FALSE,
            .vulkanMemoryModelDeviceScope = VK_FALSE,
            .vulkanMemoryModelAvailabilityVisibilityChains = VK_FALSE,
            .shaderOutputViewportIndex = VK_FALSE,
            .shaderOutputLayer = VK_FALSE,
            .subgroupBroadcastDynamicId = VK_FALSE,
        };
        VkPhysicalDeviceVulkan11Features features11{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &features12,
            .storageBuffer16BitAccess = VK_FALSE,
            .uniformAndStorageBuffer16BitAccess = VK_FALSE,
            .storagePushConstant16 = VK_TRUE,
            .storageInputOutput16 = VK_FALSE,
            .multiview = VK_FALSE,
            .multiviewGeometryShader = VK_FALSE,
            .multiviewTessellationShader = VK_FALSE,
            .variablePointersStorageBuffer = VK_FALSE,
            .variablePointers = VK_FALSE,
            .protectedMemory = VK_FALSE,
            .samplerYcbcrConversion = VK_FALSE,
            .shaderDrawParameters = VK_TRUE,
        };
        VkPhysicalDeviceFeatures2 features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &features11,
            .features = {
                .robustBufferAccess = enableRobustness ? VK_TRUE : VK_FALSE,
                .fullDrawIndexUint32 = VK_FALSE,
                .imageCubeArray = VK_FALSE,
                .independentBlend = VK_TRUE,
                .geometryShader = VK_FALSE,
                .tessellationShader = VK_FALSE,
                .sampleRateShading = VK_FALSE,
                .dualSrcBlend = VK_FALSE,
                .logicOp = VK_FALSE,
                .multiDrawIndirect = VK_TRUE,
                .drawIndirectFirstInstance = VK_TRUE,
                .depthClamp = VK_FALSE,
                .depthBiasClamp = VK_FALSE,
                .fillModeNonSolid = VK_FALSE,
                .depthBounds = VK_FALSE,
                .wideLines = VK_FALSE,
                .largePoints = VK_FALSE,
                .alphaToOne = VK_FALSE,
                .multiViewport = VK_FALSE,
                .samplerAnisotropy = VK_TRUE,
                .textureCompressionETC2 = VK_FALSE,
                .textureCompressionASTC_LDR = VK_FALSE,
                .textureCompressionBC = VK_FALSE,
                .occlusionQueryPrecise = VK_FALSE,
                .pipelineStatisticsQuery = VK_FALSE,
                .vertexPipelineStoresAndAtomics = VK_FALSE,
                .fragmentStoresAndAtomics = VK_FALSE,
                .shaderTessellationAndGeometryPointSize = VK_FALSE,
                .shaderImageGatherExtended = VK_FALSE,
                .shaderStorageImageExtendedFormats = VK_FALSE,
                .shaderStorageImageMultisample = VK_FALSE,
                .shaderStorageImageReadWithoutFormat = VK_FALSE,
                .shaderStorageImageWriteWithoutFormat = VK_FALSE,
                .shaderUniformBufferArrayDynamicIndexing = VK_FALSE,
                .shaderSampledImageArrayDynamicIndexing = VK_FALSE,
                .shaderStorageBufferArrayDynamicIndexing = VK_FALSE,
                .shaderStorageImageArrayDynamicIndexing = VK_FALSE,
                .shaderClipDistance = VK_FALSE,
                .shaderCullDistance = VK_FALSE,
                .shaderFloat64 = VK_FALSE,
                .shaderInt64 = VK_FALSE,
                .shaderInt16 = VK_TRUE,
                .shaderResourceResidency = VK_FALSE,
                .shaderResourceMinLod = VK_FALSE,
                .sparseBinding = VK_FALSE,
                .sparseResidencyBuffer = VK_FALSE,
                .sparseResidencyImage2D = VK_FALSE,
                .sparseResidencyImage3D = VK_FALSE,
                .sparseResidency2Samples = VK_FALSE,
                .sparseResidency4Samples = VK_FALSE,
                .sparseResidency8Samples = VK_FALSE,
                .sparseResidency16Samples = VK_FALSE,
                .sparseResidencyAliased = VK_FALSE,
                .variableMultisampleRate = VK_FALSE,
                .inheritedQueries = VK_FALSE,
            },
        };
        VkDeviceCreateInfo deviceCI{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features,
            .flags = 0,
            .queueCreateInfoCount = (uint32_t)queueCreateInfos.size(),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledLayerCount = (uint32_t)requiredDeviceLayers.size(),
            .ppEnabledLayerNames = requiredDeviceLayers.data(),
            .enabledExtensionCount = (uint32_t)requiredDeviceExtensions.size(),
            .ppEnabledExtensionNames = requiredDeviceExtensions.data(),
            .pEnabledFeatures = nullptr,
        };

        result = vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device);
        VkResultTry(result);

        volkLoadDevice(m_device);

        VmaVulkanFunctions vulkanFunctions{
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
            .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
            .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
            .vkAllocateMemory = vkAllocateMemory,
            .vkFreeMemory = vkFreeMemory,
            .vkMapMemory = vkMapMemory,
            .vkUnmapMemory = vkUnmapMemory,
            .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
            .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
            .vkBindBufferMemory = vkBindBufferMemory,
            .vkBindImageMemory = vkBindImageMemory,
            .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
            .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
            .vkCreateBuffer = vkCreateBuffer,
            .vkDestroyBuffer = vkDestroyBuffer,
            .vkCreateImage = vkCreateImage,
            .vkDestroyImage = vkDestroyImage,
            .vkCmdCopyBuffer = vkCmdCopyBuffer,
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
            .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR,
            .vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR,
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
            .vkBindBufferMemory2KHR = vkBindBufferMemory2KHR,
            .vkBindImageMemory2KHR = vkBindImageMemory2KHR,
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
            .vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR,
#endif
#if VMA_KHR_MAINTENANCE4 || VMA_VULKAN_VERSION >= 1003000
            .vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements,
            .vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements,
#endif
            .vkGetMemoryWin32HandleKHR = vkGetMemoryWin32HandleKHR,
        };

        VmaAllocatorCreateInfo vmaCI{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = m_physicalDevice,
            .device = m_device,
            .pVulkanFunctions = &vulkanFunctions,
            .instance = m_instance,
            .vulkanApiVersion = VK_API_VERSION_1_3,
        };
        result = vmaCreateAllocator(&vmaCI, &m_deviceAllocator);
        VkResultTry(result);

        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR, .pNext = nullptr};
        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR, .pNext = &rayTracingPipelineProperties};
        VkPhysicalDeviceMeshShaderPropertiesEXT meshShadersFeatures = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT, .pNext = &accelerationStructureProperties};
        VkPhysicalDevicePushDescriptorProperties pushDescriptorProperties = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR, .pNext = &meshShadersFeatures};
        VkPhysicalDeviceVulkan13Properties deviceProperties13 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES, .pNext = &pushDescriptorProperties};
        VkPhysicalDeviceVulkan12Properties deviceProperties12 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES, .pNext = &deviceProperties13};
        VkPhysicalDeviceVulkan11Properties deviceProperties11 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES, .pNext = &deviceProperties12};
        VkPhysicalDeviceProperties2 deviceProperties = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &deviceProperties11};
        vkGetPhysicalDeviceProperties2(m_physicalDevice, &deviceProperties);

        // Fill DeviceLimits
        m_limits.minUniformBufferOffsetAlignment = uint32_t(deviceProperties.properties.limits.minUniformBufferOffsetAlignment);
        m_limits.minStorageBufferOffsetAlignment = uint32_t(deviceProperties.properties.limits.minStorageBufferOffsetAlignment);
        m_limits.minAccelerationStructureScratchOffsetAlignment = accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment;
        m_limits.maxMeshWorkGroupInvocations = meshShadersFeatures.maxMeshWorkGroupInvocations;
        m_limits.maxMeshWorkGroupSize[0] = meshShadersFeatures.maxMeshWorkGroupSize[0];
        m_limits.maxMeshWorkGroupSize[1] = meshShadersFeatures.maxMeshWorkGroupSize[1];
        m_limits.maxMeshWorkGroupSize[2] = meshShadersFeatures.maxMeshWorkGroupSize[2];
        m_limits.rayTracingShaderGroupHandleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
        m_limits.rayTracingShaderGroupHandleAlignment = rayTracingPipelineProperties.shaderGroupHandleAlignment;
        m_limits.rayTracingShaderGroupBaseAlignment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
        result = m_queue[(uint32_t)QueueType::Graphics].Init(this, "Graphics", graphicsQueueFamilyIndex, 0);
        VkResultTry(result);

        if (computeQueueFamilyIndex != UINT32_MAX)
        {
            result = m_queue[(uint32_t)QueueType::Compute].Init(this, "Compute", computeQueueFamilyIndex, 0);
            VkResultTry(result);
        }

        if (transferQueueFamilyIndex != UINT32_MAX)
        {
            result = m_queue[(uint32_t)QueueType::Transfer].Init(this, "Transfer", transferQueueFamilyIndex, 0);
            VkResultTry(result);
        }

        result = m_bindGroupAllocator.Init(this);
        VkResultTry(result);
        return result;
    }

    void IDevice::WaitIdle()
    {
        ZoneScoped;

        vkDeviceWaitIdle(m_device);
    }

    void IDevice::Shutdown()
    {
        ZoneScoped;

        m_destroyQueue->shutdown(this);
        m_bindGroupAllocator.Shutdown();

        m_queue[(int)QueueType::Transfer].Shutdown();
        m_queue[(int)QueueType::Compute].Shutdown();
        m_queue[(int)QueueType::Graphics].Shutdown();

        vmaDestroyAllocator(m_deviceAllocator);
        vkDestroyDevice(m_device, nullptr);
        if (m_debugUtilsMessenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugUtilsMessenger, nullptr);
        }
        vkDestroyInstance(m_instance, nullptr);
    }

    void IDevice::SetDebugName(VkObjectType type, uint64_t handle, const char* name) const
    {
        if (handle == 0 /* VK_NULL_HANDLE */) return;

        if (auto fn = vkSetDebugUtilsObjectNameEXT; fn && name)
        {
            VkDebugUtilsObjectNameInfoEXT nameInfo{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = type,
                .objectHandle = handle,
                .pObjectName = name,
            };
            fn(m_device, &nameInfo);
        }
    }

    template<typename Resource, typename... Args>
    inline Resource* createImpl(IDevice* device, const char* debugName, Args... args)
    {
        Resource* resource = TL::constructFrom<Resource>(device->m_objectAllocator, debugName ? TL::StringView(debugName) : TL::StringView{});
        ResultCode result = resource->Init(device, args...);
        if (IsSuccess(result))
        {
            return resource;
        }
        TL_UNREACHABLE();
        return nullptr;
    }

    template<typename Resource>
    inline void destroyImpl(IDevice* device, Resource* resource)
    {
        resource->Shutdown(device);
        TL::destructFrom(device->m_objectAllocator, resource);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// IDevice interface implementation
    //////////////////////////////////////////////////////////////////////////////////////////

    BackendType deviceGetBackend(IDevice* self)
    {
        return self->m_backend;
    }

    DeviceFeatures deviceGetFeatures(IDevice* self)
    {
        return self->m_features;
    }

    DeviceLimits deviceGetLimits(IDevice* self)
    {
        return self->m_limits;
    }

    uint64_t deviceGarbageCollect(IDevice* self, uint64_t graphicsTimeline)
    {
        self->m_arena.reset();
        self->m_destroyQueue->Flush(self, graphicsTimeline);
        return graphicsTimeline;
    }

    uint64_t deviceGetNativeHandle(IDevice* self, NativeHandleType type, uint64_t _resource)
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
                return (uint64_t)resource->commandBuffer;
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

    Queue* deviceGetQueue(IDevice* self, QueueType queueType)
    {
        return &self->m_queue[(int)queueType];
    }

    ShaderModule* createShaderModule(IDevice* self, const ShaderModuleCreateInfo& createInfo)
    {
        return createImpl<IShaderModule>(self, createInfo.name, createInfo);
    }

    void destroyShaderModule(IDevice* self, ShaderModule* resource)
    {
        destroyImpl<IShaderModule>(self, (IShaderModule*)resource);
    }

    BindGroupLayout* createBindGroupLayout(IDevice* self, const BindGroupLayoutCreateInfo& createInfo)
    {
        return createImpl<IBindGroupLayout>(self, createInfo.name, createInfo);
    }

    void destroyBindGroupLayout(IDevice* self, BindGroupLayout* resource)
    {
        destroyImpl<IBindGroupLayout>(self, (IBindGroupLayout*)resource);
    }

    BindGroup* createBindGroup(IDevice* self, const BindGroupCreateInfo& createInfo)
    {
        return createImpl<IBindGroup>(self, createInfo.name, createInfo);
    }

    void destroyBindGroup(IDevice* self, BindGroup* resource)
    {
        destroyImpl<IBindGroup>(self, (IBindGroup*)resource);
    }

    void bindGroupUpdate(IDevice* self, BindGroup* handle, const BindGroupUpdateInfo& updateInfo)
    {
        ZoneScoped;
        auto bindGroup = (IBindGroup*)(handle);
        bindGroup->Update(self, updateInfo);
    }

    PipelineLayout* createPipelineLayout(IDevice* self, const PipelineLayoutCreateInfo& createInfo)
    {
        return createImpl<IPipelineLayout>(self, createInfo.name, createInfo);
    }

    void destroyPipelineLayout(IDevice* self, PipelineLayout* resource)
    {
        destroyImpl<IPipelineLayout>(self, (IPipelineLayout*)resource);
    }

    GraphicsPipeline* createGraphicsPipeline(IDevice* self, const GraphicsPipelineCreateInfo& createInfo)
    {
        return createImpl<IGraphicsPipeline>(self, createInfo.name, createInfo);
    }

    void destroyGraphicsPipeline(IDevice* self, GraphicsPipeline* resource)
    {
        destroyImpl<IGraphicsPipeline>(self, (IGraphicsPipeline*)resource);
    }

    ComputePipeline* createComputePipeline(IDevice* self, const ComputePipelineCreateInfo& createInfo)
    {
        return createImpl<IComputePipeline>(self, createInfo.name, createInfo);
    }

    void destroyComputePipeline(IDevice* self, ComputePipeline* resource)
    {
        destroyImpl<IComputePipeline>(self, (IComputePipeline*)resource);
    }

    RayTracingPipeline* createRayTracingPipeline(IDevice* self, const RayTracingPipelineCreateInfo& createInfo)
    {
        return createImpl<IRayTracingPipeline>(self, createInfo.name, createInfo);
    }

    void destroyRayTracingPipeline(IDevice* self, RayTracingPipeline* resource)
    {
        destroyImpl<IRayTracingPipeline>(self, (IRayTracingPipeline*)resource);
    }

    void rayTracingPipelineGetShaderBindingTableEntry(IDevice* self, RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle)
    {
        return ((IRayTracingPipeline*)handle)->GetShaderBindingTableEntry(self, group, size, dstHandle);
    }

    Buffer* createBuffer(IDevice* self, const BufferCreateInfo& createInfo)
    {
        return createImpl<IBuffer>(self, createInfo.name, createInfo);
    }

    void destroyBuffer(IDevice* self, Buffer* resource)
    {
        destroyImpl<IBuffer>(self, (IBuffer*)resource);
    }

    uint64_t bufferGetDeviceAddress(IDevice* self, Buffer* _buffer)
    {
        IBuffer* buffer = (IBuffer*)_buffer;
        TL_ASSERT(buffer->address != 0, "Buffer is not shader addressable");
        return buffer->address;
    }

    DeviceMemoryPtr bufferMap(IDevice* self, Buffer* _buffer, uint64_t offset,  uint64_t sizeBytes)
    {
        (void)sizeBytes;
        IBuffer* buffer = (IBuffer*)_buffer;
        auto* ptr = (char*)buffer->Map(self);
        return ptr ? ptr + offset : nullptr;
    }

    void bufferUnmap(IDevice* self, Buffer* _buffer)
    {
        IBuffer* buffer = (IBuffer*)_buffer;
        buffer->Unmap(self);
    }

    Image* createImage(IDevice* self, const ImageCreateInfo& createInfo)
    {
        return createImpl<IImage>(self, createInfo.name, createInfo);
    }

    Image* createImageView(IDevice* self, const ImageViewCreateInfo& createInfo)
    {
        return createImpl<IImage>(self, createInfo.name, createInfo);
    }

    void destroyImage(IDevice* self, Image* resource)
    {
        destroyImpl<IImage>(self, (IImage*)resource);
    }

    Sampler* createSampler(IDevice* self, const SamplerCreateInfo& createInfo)
    {
        return createImpl<ISampler>(self, createInfo.name, createInfo);
    }

    void destroySampler(IDevice* self, Sampler* resource)
    {
        destroyImpl<ISampler>(self, (ISampler*)resource);
    }

    AccelerationStructure* createAccelerationStructure(IDevice* self, const AccelerationStructureCreateInfo& createInfo)
    {
        return createImpl<IAccelerationStructure>(self, createInfo.name, createInfo);
    }

    void destroyAccelerationStructure(IDevice* self, AccelerationStructure* handle)
    {
        destroyImpl<IAccelerationStructure>(self, (IAccelerationStructure*)handle);
    }

    uint64_t accelerationStructureGetDeviceAddress(IDevice* self, AccelerationStructure* handle)
    {
        return ((IAccelerationStructure*)handle)->address;
    }

    AccelerationStructureSizesInfo accelerationStructureGetSizesInfo(IDevice* self, AccelerationStructure* handle)
    {
        return ((IAccelerationStructure*)handle)->sizes;
    }

    Micromap* createMicromap(IDevice* self, const MicromapCreateInfo& createInfo)
    {
        return createImpl<IMicromap>(self, createInfo.name, createInfo);
    }

    void destroyMicromap(IDevice* self, Micromap* handle)
    {
        destroyImpl<IMicromap>(self, (IMicromap*)handle);
    }

    CommandPool* createCommandPool(IDevice* self, const CommandPoolCreateInfo& createInfo)
    {
        auto pool = TL::constructFrom<ICommandPool>(self->m_objectAllocator);
        pool->Init(self, createInfo);
        return pool;
    }

    void destroyCommandPool(IDevice* self, CommandPool* resource)
    {
        auto pool = (ICommandPool*)resource;
        pool->Shutdown(self);
        TL::destructFrom(self->m_objectAllocator, pool);
    }

    Fence* createFence(IDevice* self, const FenceCreateInfo& createInfo)
    {
        return createImpl<IFence>(self, createInfo.name, createInfo);
    }

    void destroyFence(IDevice* self, Fence* resource)
    {
        destroyImpl<IFence>(self, (IFence*)resource);
    }

    uint64_t fenceGetValue(IDevice* self, Fence* _fence)
    {
        IFence* fence = (IFence*)_fence;

        uint64_t value;
        VK_CHECK(vkGetSemaphoreCounterValue(self->m_device, fence->semaphore, &value));
        return value;
    }

    QueryPool* createQueryPool(IDevice* self, const QueryPoolCreateInfo& createInfo)
    {
        return createImpl<IQueryPool>(self, createInfo.name, createInfo);
    }

    void destroyQueryPool(IDevice* self, QueryPool* resource)
    {
        destroyImpl<IQueryPool>(self, (IQueryPool*)resource);
    }

    Swapchain* createSwapchain(IDevice* self, const SwapchainCreateInfo& createInfo)
    {
        return createImpl<ISwapchain>(self, createInfo.name, createInfo);
    }

    void destroySwapchain(IDevice* self, Swapchain* resource)
    {
        destroyImpl<ISwapchain>(self, (ISwapchain*)resource);
    }

    uint32_t swapchainGetImagesCount(IDevice* self, Swapchain* _swapchain)
    {
        auto swapchain = (ISwapchain*)_swapchain;
        return swapchain->GetImagesCount();
    }

    SwapchainAcquireResult swapchainAcquireImage(IDevice* self, Swapchain* _swapchain)
    {
        auto swapchain = (ISwapchain*)_swapchain;
        return swapchain->AcquireSwapchainImage();
    }

    SurfaceCapabilities swapchainGetSurfaceCapabilities(IDevice* self, Swapchain* _swapchain)
    {
        auto swapchain = (ISwapchain*)_swapchain;
        return swapchain->GetSurfaceCapabilities(self);
    }

    ResultCode swapchainResize(IDevice* self, Swapchain* _swapchain, const ImageSize2D& size)
    {
        auto swapchain = (ISwapchain*)_swapchain;
        return swapchain->ResizeSwapchain(self, size);
    }

    ResultCode swapchainConfigure(IDevice* self, Swapchain* _swapchain, const SwapchainConfigureInfo& configInfo)
    {
        auto swapchain = (ISwapchain*)_swapchain;
        return swapchain->ConfigureSwapchain(self, configInfo);
    }

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
        TL_ASSERT(m_queryPool.empty());
        TL_ASSERT(m_swapchain.empty());
        TL_ASSERT(m_surface.empty());
        TL_ASSERT(m_semaphore.empty());
        TL_ASSERT(m_accelerationStructure.empty());
        TL_ASSERT(m_micromap.empty());
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
        else if constexpr (std::is_same_v<VkAccelerationStructureKHR, ResourceType>) vkDestroyAccelerationStructureKHR(device->m_device, handle, nullptr);
        else if constexpr (std::is_same_v<VkMicromapEXT, ResourceType>) vkDestroyMicromapEXT(device->m_device, handle, nullptr);
        else if constexpr (std::is_same_v<VmaBufferAllocation, ResourceType>) vmaDestroyBuffer(device->m_deviceAllocator, handle.first, handle.second);
        else if constexpr (std::is_same_v<VmaImageAllocation, ResourceType>) vmaDestroyImage(device->m_deviceAllocator, handle.first, handle.second);
        else
        {
            static_assert(false, "Invalid ResourceType");
        }
    }

    template<typename ResourceType>
    void DeleteQueue::FlushQueue(IDevice* device, TL::Vector<ResourceDeleteQueueEntry<ResourceType>>& queue, uint64_t timeline)
    {
        uint32_t deleteCount = 0;
        for (const auto& entry : queue)
        {
            if (entry.timeline > timeline)
                break;

            destroyVkResource(device, entry.resource);

            uint64_t handleVal = 0;
            memcpy(&handleVal, &entry.resource, sizeof(entry.resource));
            uint64_t key = TL::HashCombine(typeKey<ResourceType>(), handleVal);
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
        FlushQueue(device, m_queryPool, timeline);
        FlushQueue(device, m_pipeline, timeline);
        FlushQueue(device, m_sampler, timeline);
        FlushQueue(device, m_accelerationStructure, timeline);
        FlushQueue(device, m_micromap, timeline);
        FlushQueue(device, m_buffer, timeline);
        FlushQueue(device, m_image, timeline);
        FlushQueue(device, m_swapchain, timeline);
        FlushQueue(device, m_surface, timeline);
        FlushQueue(device, m_semaphore, timeline);
        FlushQueue(device, m_allocation, timeline);
    }

} // namespace RHI::Vulkan
