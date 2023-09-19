#include "pch.hpp"

#include "RHI-Vulkan/Loader.hpp"

#include "Context.hpp"

#include "ShaderBindGroup.hpp"
#include "ResourcePool.hpp"
#include "Pipeline.hpp"
#include "ShaderModule.hpp"
#include "FrameScheduler.hpp"
#include "Swapchain.hpp"

namespace Vulkan
{

inline static VkBool32 VKAPI_CALL DebugMessengerCallbacks(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
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
}

Context* Context::Create(const RHI::ApplicationInfo& appInfo, RHI::DeviceSelectionCallback deviceSelectionCallbacks, std::unique_ptr<RHI::DebugCallbacks> debugCallbacks)
{
    // std::vector<const char*> enabledLayersNames;
    // enabledLayersNames.push_back("VK_LAYER_KHRONOS_validation");

    // std::vector<const char*> enabledExtensionsNames;
    // enabledExtensionsNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    // enabledExtensionsNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    // enabledExtensionsNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    // enabledExtensionsNames.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

    // vk::ApplicationInfo vkAppInfo;
    // vkAppInfo.setPApplicationName(appInfo.applicationName);
    // vkAppInfo.setApplicationVersion(appInfo.applicationVersion);
    // vkAppInfo.setPEngineName(appInfo.engineName);
    // vkAppInfo.setEngineVersion(appInfo.engineVersion);
    // vkAppInfo.setApiVersion(VK_API_VERSION_1_3);

    // vk::DebugUtilsMessageSeverityFlagsEXT severityFlags {};
    // severityFlags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
    // severityFlags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

    // vk::DebugUtilsMessageTypeFlagsEXT typeFlags {};
    // typeFlags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral;
    // typeFlags |= vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    // typeFlags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

    // vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    // debugCreateInfo.setMessageSeverity(severityFlags);
    // debugCreateInfo.setMessageType(typeFlags);
    // debugCreateInfo.setPfnUserCallback(DebugMessengerCallbacks);

    // vk::InstanceCreateInfo createInfo;
    // createInfo.setPNext(&debugCreateInfo);
    // createInfo.setPApplicationInfo(&vkAppInfo);
    // createInfo.setPEnabledLayerNames(enabledLayersNames);
    // createInfo.setPEnabledExtensionNames(enabledExtensionsNames);

    // m_instance = vk::createInstance(createInfo).value;

    // {
    //     m_physicalDevice = m_instance.enumeratePhysicalDevices().value[deviceId];

    //     if (m_device)
    //     {
    //         RHI_ASSERT_MSG(m_device.waitIdle() == vk::Result::eSuccess, "Timeout");

    //         if (m_allocator)
    //         {
    //             vmaDestroyAllocator(m_allocator);
    //         }

    //         m_device.destroy();
    //     }

    //     vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures {};
    //     dynamicRenderingFeatures.setDynamicRendering(VK_TRUE);

    //     vk::DeviceCreateInfo createInfo {};
    //     createInfo.setPNext(&dynamicRenderingFeatures);

    //     vk::PhysicalDeviceFeatures             features {};
    //     std::vector<vk::DeviceQueueCreateInfo> queues {};
    //     std::vector<const char*>               enabledLayers {};
    //     std::vector<const char*>               enabledExtensions {};
    //     enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    //     enabledExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    //     enabledExtensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    //     enabledExtensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    //     enabledExtensions.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    //     enabledExtensions.push_back(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);

    //     int32_t graphicsQueueIndex = -1;
    //     int32_t computeQueueIndex  = -1;
    //     int32_t transferQueueIndex = -1;

    //     int32_t queueFamilyIndex = 0;
    //     for (auto queueFamilyProperties : m_physicalDevice.getQueueFamilyProperties2())
    //     {
    //         auto properties = queueFamilyProperties.queueFamilyProperties;

    //         if (properties.queueFlags & vk::QueueFlagBits::eGraphics)
    //         {
    //             graphicsQueueIndex = queueFamilyIndex;
    //         }
    //         else if (properties.queueFlags & vk::QueueFlagBits::eCompute)
    //         {
    //             computeQueueIndex = queueFamilyIndex;
    //         }
    //         else if (properties.queueFlags & vk::QueueFlagBits::eTransfer)
    //         {
    //             transferQueueIndex = queueFamilyIndex;
    //         }

    //         queueFamilyIndex++;
    //     }

    //     float                     prio = 1.0;
    //     vk::DeviceQueueCreateInfo queueInfo;
    //     queueInfo.setQueueCount(1);
    //     queueInfo.setQueuePriorities(prio);

    //     if (graphicsQueueIndex != -1)
    //     {
    //         queueInfo.setQueueFamilyIndex(static_cast<uint32_t>(graphicsQueueIndex));
    //         queues.push_back(queueInfo);
    //     }

    //     if (graphicsQueueIndex != -1)
    //     {
    //         queueInfo.setQueueFamilyIndex(static_cast<uint32_t>(computeQueueIndex));
    //         queues.push_back(queueInfo);
    //     }

    //     if (transferQueueIndex != -1)
    //     {
    //         queueInfo.setQueueFamilyIndex(static_cast<uint32_t>(transferQueueIndex));
    //         queues.push_back(queueInfo);
    //     }

    //     createInfo.setQueueCreateInfos(queues);
    //     createInfo.setPEnabledLayerNames(enabledLayers);
    //     createInfo.setPEnabledExtensionNames(enabledExtensions);
    //     createInfo.setPEnabledFeatures(&features);

    //     m_device = m_physicalDevice.createDevice(createInfo).value;

    //     m_graphicsQueue = m_device.getQueue(static_cast<uint32_t>(graphicsQueueIndex), 0);
    //     m_computeQueue  = computeQueueIndex != -1 ? m_device.getQueue(static_cast<uint32_t>(computeQueueIndex), 0u) : m_graphicsQueue;
    //     m_transferQueue = transferQueueIndex != -1 ? m_device.getQueue(static_cast<uint32_t>(computeQueueIndex), 0u) : m_graphicsQueue;

    //     // Create VmaAllocator Instance
    //     {
    //         VmaAllocatorCreateInfo vmaCreateInfo {};
    //         vmaCreateInfo.instance         = m_instance;
    //         vmaCreateInfo.physicalDevice   = m_physicalDevice;
    //         vmaCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    //         vmaCreateInfo.device           = m_device;

    //         VkResult result = vmaCreateAllocator(&vmaCreateInfo, &m_allocator);
    //         RHI_ASSERT_MSG(result == VK_SUCCESS, "Failed to create VmaAllocator");
    //     }
    // }
    auto context = new Context();
    return context;
}

std::unique_ptr<RHI::ShaderModule> Context::CreateShaderModule(const RHI::ShaderModuleCreateInfo& createInfo)
{
    return {};
}

std::unique_ptr<RHI::Swapchain> Context::CreateSwapchain(const RHI::SwapchainCreateInfo& createInfo)
{
    return {};
}

std::unique_ptr<RHI::ResourcePool> Context::CreateResourcePool(const RHI::ResourcePoolCreateInfo& createInfo)
{
    return {};
}

RHI::Handle<RHI::GraphicsPipeline> Context::CreateGraphicsPipeline(const RHI::GraphicsPipelineCreateInfo& createInfo)
{
    return {};
}

RHI::Handle<RHI::ComputePipeline> Context::CreateComputePipeline(const RHI::ComputePipelineCreateInfo& createInfo)
{
    return {};
}

RHI::Handle<RHI::Sampler> Context::CreateSampler(const RHI::SamplerCreateInfo& createInfo)
{
    return {};
}

std::unique_ptr<RHI::FrameScheduler> Context::CreateFrameScheduler()
{
    return {};
}

std::unique_ptr<RHI::ShaderBindGroupAllocator> Context::CreateShaderBindGroupAllocator()
{
    return {};
}

void Context::Free(RHI::Handle<RHI::GraphicsPipeline> pso)
{
}

void Context::Free(RHI::Handle<RHI::ComputePipeline> pso)
{
}

void Context::Free(RHI::Handle<RHI::Sampler> pso)
{
}

}  // namespace Vulkan