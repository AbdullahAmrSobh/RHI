#define VOLK_IMPLEMENTATION

#include <TL/Assert.hpp>
#define VMA_ASSERT(expr) TL_ASSERT(expr)
#define VMA_IMPLEMENTATION

#include "device.h"
#include "common.h"

#include <tracy/TracyVulkan.hpp>

#include <TL/Log.hpp>
#include <TL/Allocator/Allocator.hpp>
#include <TL/Containers/InlineVector.hpp>

#include <algorithm>
#include <bit>
#include <cstring>
#include <format>

namespace RHI::Vulkan
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DeleteQueue
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void DeleteQueue::shutdown(IDevice* device)
    {
        Flush(device, UINT64_MAX);
        TL_ASSERT(m_entries.empty());
    }

    // Entries are destroyed in submission order, so each Push site must push child-first
    // (views before the image/buffer they view, resources before their memory allocation).
    void DeleteQueue::Flush(IDevice* device, uint64_t timeline)
    {
        size_t count = 0;
        for (const DeferredResource& entry : m_entries)
        {
            if (entry.timeline <= timeline)
            {
                switch (entry.type)
                {
                case DeferredResourceType::Allocation: vmaFreeMemory(device->m_deviceAllocator, std::bit_cast<VmaAllocation>(entry.handle)); break;
                case DeferredResourceType::Buffer: vkDestroyBuffer(device->m_device, std::bit_cast<VkBuffer>(entry.handle), nullptr); break;
                case DeferredResourceType::BufferView: vkDestroyBufferView(device->m_device, std::bit_cast<VkBufferView>(entry.handle), nullptr); break;
                case DeferredResourceType::Image: vkDestroyImage(device->m_device, std::bit_cast<VkImage>(entry.handle), nullptr); break;
                case DeferredResourceType::ImageView: vkDestroyImageView(device->m_device, std::bit_cast<VkImageView>(entry.handle), nullptr); break;
                case DeferredResourceType::Sampler: vkDestroySampler(device->m_device, std::bit_cast<VkSampler>(entry.handle), nullptr); break;
                case DeferredResourceType::Pipeline: vkDestroyPipeline(device->m_device, std::bit_cast<VkPipeline>(entry.handle), nullptr); break;
                case DeferredResourceType::DescriptorPool: vkDestroyDescriptorPool(device->m_device, std::bit_cast<VkDescriptorPool>(entry.handle), nullptr); break;
                case DeferredResourceType::QueryPool: vkDestroyQueryPool(device->m_device, std::bit_cast<VkQueryPool>(entry.handle), nullptr); break;
                case DeferredResourceType::Swapchain: vkDestroySwapchainKHR(device->m_device, std::bit_cast<VkSwapchainKHR>(entry.handle), nullptr); break;
                case DeferredResourceType::Surface: vkDestroySurfaceKHR(device->m_instance, std::bit_cast<VkSurfaceKHR>(entry.handle), nullptr); break;
                case DeferredResourceType::Semaphore: vkDestroySemaphore(device->m_device, std::bit_cast<VkSemaphore>(entry.handle), nullptr); break;
                case DeferredResourceType::AccelerationStructure: vkDestroyAccelerationStructureKHR(device->m_device, std::bit_cast<VkAccelerationStructureKHR>(entry.handle), nullptr); break;
                case DeferredResourceType::Micromap: vkDestroyMicromapEXT(device->m_device, std::bit_cast<VkMicromapEXT>(entry.handle), nullptr); break;
                }
            }
            else
            {
                m_entries[count++] = entry;
            }
        }
        m_entries.resize(count);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IDevice lifetime
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Device* createDevice(const ApplicationInfo& appInfo)
    {
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
        const char* message = pCallbackData->pMessage;

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

    VkResult IQueue::Init(IDevice* device, const char* debugName, uint32_t familyIndex, uint32_t queueIndex, VkQueueFlags queueFlags, uint32_t timestampValidBits)
    {
        m_device = device;
        m_familyIndex = familyIndex;
        m_lastSubmitValue.store(0, std::memory_order_relaxed);

        vkGetDeviceQueue(device->m_device, familyIndex, queueIndex, &m_queue);
        if (debugName)
            m_device->SetDebugName(VK_OBJECT_TYPE_QUEUE, (uint64_t)m_queue, debugName);

        VkSemaphoreTypeCreateInfo timelineInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = 0,
        };
        VkSemaphoreCreateInfo semaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &timelineInfo,
        };
        VkResult result = vkCreateSemaphore(device->m_device, &semaphoreInfo, nullptr, &m_submissionTimeline);
        if (result != VK_SUCCESS)
            return result;
        if (debugName)
            m_device->SetDebugName(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_submissionTimeline, TL::fmt("{}: submission timeline", debugName));

#if defined(TRACY_ENABLE)
        const VkQueueFlags tracyQueueCapabilities = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
        if ((queueFlags & tracyQueueCapabilities) == 0 || timestampValidBits == 0)
            return VK_SUCCESS;

        VkCommandPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = familyIndex,
        };
        VkCommandPool tracyCommandPool = VK_NULL_HANDLE;
        result = vkCreateCommandPool(device->m_device, &poolInfo, nullptr, &tracyCommandPool);
        if (result != VK_SUCCESS)
        {
            vkDestroySemaphore(device->m_device, m_submissionTimeline, nullptr);
            m_submissionTimeline = VK_NULL_HANDLE;
            return result;
        }

        VkCommandBufferAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = tracyCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        VkCommandBuffer tracyCommandBuffer = VK_NULL_HANDLE;
        result = vkAllocateCommandBuffers(device->m_device, &allocateInfo, &tracyCommandBuffer);
        if (result != VK_SUCCESS)
        {
            vkDestroyCommandPool(device->m_device, tracyCommandPool, nullptr);
            vkDestroySemaphore(device->m_device, m_submissionTimeline, nullptr);
            m_submissionTimeline = VK_NULL_HANDLE;
            return result;
        }

        m_tracyContext = TracyVkContext(device->m_instance, device->m_physicalDevice, device->m_device, m_queue, tracyCommandBuffer, vkGetInstanceProcAddr, vkGetDeviceProcAddr);
        if (debugName)
            TracyVkContextName(static_cast<TracyVkCtx>(m_tracyContext), debugName, static_cast<uint16_t>(std::strlen(debugName)));
        vkFreeCommandBuffers(device->m_device, tracyCommandPool, 1, &tracyCommandBuffer);
        vkDestroyCommandPool(device->m_device, tracyCommandPool, nullptr);
#endif
        return VK_SUCCESS;
    }

    void IQueue::Shutdown()
    {
        vkQueueWaitIdle(m_queue);
#if defined(TRACY_ENABLE)
        if (m_tracyContext)
        {
            TracyVkDestroy(static_cast<TracyVkCtx>(m_tracyContext));
            m_tracyContext = nullptr;
        }
#endif
        vkDestroySemaphore(m_device->m_device, m_submissionTimeline, nullptr);
        m_submissionTimeline = VK_NULL_HANDLE;
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
        TL_ASSERT(submitInfo.waitFences.size() <= Limits::QueueFences, "Too many queue wait fences");
        TL_ASSERT(submitInfo.signalFences.size() + submitInfo.presentSwapchains.size() <= Limits::QueueFences, "Too many queue signal fences");
        TL_ASSERT(submitInfo.commandLists.size() <= Limits::QueueCommandBuffers, "Too many submitted command buffers");
        TL_ASSERT(submitInfo.presentSwapchains.size() <= Limits::QueueSwapchains, "Too many presented swapchains");
        TL::InlineVector<VkSemaphoreSubmitInfo, Limits::QueueFences> waitSemaphores;
        TL::InlineVector<VkCommandBufferSubmitInfo, Limits::QueueCommandBuffers> commandBufferSubmitInfos;
        TL::InlineVector<VkSemaphoreSubmitInfo, Limits::QueueFences> signalSemaphores;

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

            TL_ASSERT(swapchain->m_imageAcquired);
            VkSemaphore presentSemaphore = swapchain->m_presentSemaphore[swapchain->m_imageIndex];
            signalSemaphores.push_back({
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = presentSemaphore,
                .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            });
        }

        const uint64_t submitValue = self->m_lastSubmitValue.fetch_add(1, std::memory_order_relaxed) + 1;
        signalSemaphores.push_back({
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = self->m_submissionTimeline,
            .value = submitValue,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        });

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

        if (submitInfo.presentSwapchains.empty() == false)
        {
            TL::InlineVector<VkSwapchainKHR, Limits::QueueSwapchains> swapchains;
            TL::InlineVector<uint32_t, Limits::QueueSwapchains> imageIndices;
            TL::InlineVector<VkSemaphore, Limits::QueueSwapchains> presentWaitSemaphores;

            for (auto _swapchain : submitInfo.presentSwapchains)
            {
                ISwapchain* swapchain = (ISwapchain*)_swapchain;
                VkSemaphore semaphore = swapchain->m_presentSemaphore[swapchain->m_imageIndex];
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
            for (auto _swapchain : submitInfo.presentSwapchains)
                ((ISwapchain*)_swapchain)->m_imageAcquired = false;
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

    ResultCode IDevice::Init(const ApplicationInfo& appInfo)
    {
        m_backend = BackendType::Vulkan1_3;

        VulkanResult result;

        VK_CHECK(volkInitialize());

        constexpr bool EnableAsyncQueues = true;

        TL::InlineVector<const char*, 4> requiredInstanceLayers;
        TL::InlineVector<const char*, 8> requiredInstanceExtensions{
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
            .ppEnabledLayerNames = requiredInstanceLayers.empty() ? nullptr : requiredInstanceLayers.data(),
            .enabledExtensionCount = (uint32_t)requiredInstanceExtensions.size(),
            .ppEnabledExtensionNames = requiredInstanceExtensions.empty() ? nullptr : requiredInstanceExtensions.data(),
        };

        result = vkCreateInstance(&instanceCI, nullptr, &m_instance);
        VkResultTry(result);

        volkLoadInstanceOnly(m_instance);

        // Select the physical device

        TL::InlineVector<const char*, 4> requiredDeviceLayers;
        TL::InlineVector<const char*, 16> requiredDeviceExtensions{
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
            TL_ASSERT(physicalDeviceCount <= Limits::PhysicalDevices, "Too many Vulkan physical devices");
            TL::InlineVector<VkPhysicalDevice, Limits::PhysicalDevices> physicalDevices;
            physicalDevices.resize(physicalDeviceCount);
            VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data()));
            for (VkPhysicalDevice physicalDevice : physicalDevices)
            {
                TL::InlineVector<VkLayerProperties, Limits::EnumerationEntries> availableDeviceLayers;
                {
                    uint32_t deviceLayerCount;
                    VK_CHECK(vkEnumerateDeviceLayerProperties(physicalDevice, &deviceLayerCount, nullptr));
                    TL_ASSERT(deviceLayerCount <= Limits::EnumerationEntries, "Too many Vulkan device layers");
                    availableDeviceLayers.resize(deviceLayerCount);
                    VK_CHECK(vkEnumerateDeviceLayerProperties(physicalDevice, &deviceLayerCount, availableDeviceLayers.data()));
                }

                TL::InlineVector<VkExtensionProperties, Limits::EnumerationEntries> availableDeviceExtensions;
                {
                    uint32_t extensionsCount;
                    VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr));
                    TL_ASSERT(extensionsCount <= Limits::EnumerationEntries, "Too many Vulkan device extensions");
                    availableDeviceExtensions.resize(extensionsCount);
                    VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, availableDeviceExtensions.data()));
                }

                // search for a suitable physical device if it contains the required extensions
                bool containAllLayers = std::all_of(requiredDeviceLayers.begin(), requiredDeviceLayers.end(), [&](const char* layer)
                    {
                        return std::any_of(availableDeviceLayers.begin(), availableDeviceLayers.end(), [layer](const VkLayerProperties& available)
                            {
                                return std::strcmp(available.layerName, layer) == 0;
                            });
                    });

                bool containAllExtensions = std::all_of(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end(), [&](const char* ext)
                    {
                        return std::any_of(availableDeviceExtensions.begin(), availableDeviceExtensions.end(), [ext](const VkExtensionProperties& available)
                            {
                                return std::strcmp(available.extensionName, ext) == 0;
                            });
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
        TL_ASSERT(queueFamilyPropertiesCount <= Limits::QueueFamilies, "Too many Vulkan queue families");
        TL::InlineVector<VkQueueFamilyProperties, Limits::QueueFamilies> queueFamilyProperties;
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

        TL::InlineVector<VkDeviceQueueCreateInfo, 3> queueCreateInfos;

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
            .robustImageAccess2 = VK_TRUE,
            .nullDescriptor = VK_TRUE,
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
            .shaderFloat16 = VK_TRUE, // shaders declare the SPIR-V Float16 capability (half in bxdf)
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
            .ppEnabledLayerNames = requiredDeviceLayers.empty() ? nullptr : requiredDeviceLayers.data(),
            .enabledExtensionCount = (uint32_t)requiredDeviceExtensions.size(),
            .ppEnabledExtensionNames = requiredDeviceExtensions.empty() ? nullptr : requiredDeviceExtensions.data(),
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
        result = m_queue[(uint32_t)QueueType::Graphics].Init(this, "Graphics", graphicsQueueFamilyIndex, 0, queueFamilyProperties[graphicsQueueFamilyIndex].queueFlags, queueFamilyProperties[graphicsQueueFamilyIndex].timestampValidBits);
        VkResultTry(result);

        if (computeQueueFamilyIndex != UINT32_MAX)
        {
            result = m_queue[(uint32_t)QueueType::Compute].Init(this, "Compute", computeQueueFamilyIndex, 0, queueFamilyProperties[computeQueueFamilyIndex].queueFlags, queueFamilyProperties[computeQueueFamilyIndex].timestampValidBits);
            VkResultTry(result);
        }

        if (transferQueueFamilyIndex != UINT32_MAX)
        {
            result = m_queue[(uint32_t)QueueType::Transfer].Init(this, "Transfer", transferQueueFamilyIndex, 0, queueFamilyProperties[transferQueueFamilyIndex].queueFlags, queueFamilyProperties[transferQueueFamilyIndex].timestampValidBits);
            VkResultTry(result);
        }

        result = m_bindGroupAllocator.Init(this);
        VkResultTry(result);
        return result;
    }

    void IDevice::WaitIdle()
    {
        vkDeviceWaitIdle(m_device);
    }

    void IDevice::Shutdown()
    {
        m_destroyQueue.shutdown(this);
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

    void IDevice::SetDebugName(VkObjectType type, uint64_t handle, TL::StringView name) const
    {
        if (handle == 0 /* VK_NULL_HANDLE */) return;

        if (auto fn = vkSetDebugUtilsObjectNameEXT; fn && !name.empty())
        {
            // pObjectName must be null-terminated, which a StringView does not guarantee.
            char nameBuffer[256];
            const size_t length = (std::min)(name.size(), sizeof(nameBuffer) - 1);
            memcpy(nameBuffer, name.data(), length);
            nameBuffer[length] = '\0';

            VkDebugUtilsObjectNameInfoEXT nameInfo{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = type,
                .objectHandle = handle,
                .pObjectName = nameBuffer,
            };
            fn(m_device, &nameInfo);
        }
    }

    template<typename Resource, typename... Args>
    inline Resource* createImpl(IDevice* device, const char* debugName, Args... args)
    {
        Resource* resource = TL::constructFrom<Resource>(TL::Context::getDefaultAllocator(), debugName ? TL::StringView(debugName) : TL::StringView{});
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
        TL::destructFrom(TL::Context::getDefaultAllocator(), resource);
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
        (void)graphicsTimeline;
        uint64_t completedValue = 0;
        IQueue* graphicsQueue = self->getQueue(QueueType::Graphics);
        VK_CHECK(vkGetSemaphoreCounterValue(self->m_device, graphicsQueue->m_submissionTimeline, &completedValue));
        self->m_destroyQueue.Flush(self, completedValue);
        return completedValue;
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

    DeviceMemoryPtr bufferMap(IDevice* self, Buffer* _buffer, uint64_t offset, uint64_t sizeBytes)
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
        auto pool = TL::constructFrom<ICommandPool>(TL::Context::getDefaultAllocator());
        pool->Init(self, createInfo);
        return pool;
    }

    void destroyCommandPool(IDevice* self, CommandPool* resource)
    {
        auto pool = (ICommandPool*)resource;
        pool->Shutdown(self);
        TL::destructFrom(TL::Context::getDefaultAllocator(), pool);
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
        return swapchain->AcquireSwapchainImage(self);
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

} // namespace RHI::Vulkan
