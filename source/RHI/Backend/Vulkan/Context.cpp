#define VK_USE_PLATFORM_WIN32_KHR
#include "RHI/Backend/Vulkan/Context.hpp"

namespace Vulkan
{

inline static VkBool32 VKAPI_CALL DebugMessengerCallbacks(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                          VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                                                          const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                          void*                                       pUserData)
{
    if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        RHI_LOG(pCallbackData->pMessage);
    else if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        RHI_LOG(pCallbackData->pMessage);
    else if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        RHI_WARN(pCallbackData->pMessage);
    else if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        RHI_ERROR(pCallbackData->pMessage);

    return VK_FALSE;
}

RHI::ResultCode Context::Init(const RHI::ApplicationInfo& appInfo)
{
    vk::ApplicationInfo vkAppInfo;
    vkAppInfo.setPApplicationName(appInfo.applicationName.data());
    vkAppInfo.setApplicationVersion(appInfo.applicationVersion);
    vkAppInfo.setPEngineName(appInfo.engineName.data());
    vkAppInfo.setEngineVersion(appInfo.engineVersion);
    vkAppInfo.setApiVersion(VK_API_VERSION_1_3);

    std::vector<const char*> enabledExtensionsNames {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface"};
    std::vector<const char*> enabledLayersNames {"VK_LAYER_KHRONOS_validation"};

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    debugCreateInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                                       | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                       | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning);
    debugCreateInfo.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                                   | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    debugCreateInfo.setPfnUserCallback(DebugMessengerCallbacks);

    vk::InstanceCreateInfo createInfo;
    createInfo.setPNext(&debugCreateInfo);
    createInfo.setPApplicationInfo(&vkAppInfo);
    createInfo.setPEnabledLayerNames(enabledLayersNames);
    createInfo.setPEnabledExtensionNames(enabledExtensionsNames);

    m_instance = vk::createInstance(createInfo).value;

    SetDevice(0);

    return RHI::ResultCode::Success;
}

Context::~Context()
{
    m_device.destroy();
    m_instance.destroy();
}

RHI::ResultCode Context::SetDevice(uint32_t deviceId)
{
    m_physicalDevice = m_instance.enumeratePhysicalDevices().value[deviceId];

    if (m_device)
    {
        m_device.waitIdle();
        m_device.destroy();
    }

    vk::DeviceCreateInfo                   createInfo;
    vk::PhysicalDeviceFeatures             features;
    std::vector<vk::DeviceQueueCreateInfo> queues;
    std::vector<const char*>               enabledLayers;
    std::vector<const char*>               enabledExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    int32_t graphicsQueueIndex = -1;
    int32_t computeQueueIndex  = -1;
    int32_t transferQueueIndex = -1;

    int32_t queueFamilyIndex = 0;
    for (auto queueFamilyProperties : m_physicalDevice.getQueueFamilyProperties2())
    {
        auto properties = queueFamilyProperties.queueFamilyProperties;

        if (properties.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            graphicsQueueIndex = queueFamilyIndex;
        }
        else if (properties.queueFlags & vk::QueueFlagBits::eCompute)
        {
            computeQueueIndex = queueFamilyIndex;
        }
        else if (properties.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            transferQueueIndex = queueFamilyIndex;
        }

        queueFamilyIndex++;
    }

    vk::DeviceQueueCreateInfo queueInfo {};

    if (graphicsQueueIndex != -1)
    {
        queueInfo.setQueueFamilyIndex(graphicsQueueIndex);
        queues.push_back(queueInfo);
    }

    if (graphicsQueueIndex != -1)
    {
        queueInfo.setQueueFamilyIndex(computeQueueIndex);
        queues.push_back(queueInfo);
    }

    if (transferQueueIndex != -1)
    {
        queueInfo.setQueueFamilyIndex(transferQueueIndex);
        queues.push_back(queueInfo);
    }

    createInfo.setQueueCreateInfos(queues);
    createInfo.setPEnabledLayerNames(enabledLayers);
    createInfo.setPEnabledExtensionNames(enabledExtensions);
    createInfo.setPEnabledFeatures(&features);

    m_device = m_physicalDevice.createDevice(createInfo).value;

    m_graphicsQueue = m_device.getQueue(graphicsQueueIndex, 0);
    m_computeQueue  = computeQueueIndex != -1 ? m_device.getQueue(computeQueueIndex, 0u) : m_graphicsQueue;
    m_transferQueue = transferQueueIndex != -1 ? m_device.getQueue(computeQueueIndex, 0u) : m_graphicsQueue;

    return RHI::ResultCode::Success;
}

std::shared_ptr<vk::UniqueSurfaceKHR> Context::CreateSurface(void* windowHandle)
{
    auto   device = m_device;
    size_t hash   = RHI::HashAny(windowHandle);
    if (auto surface = m_surfaceCache.Get(hash); surface != nullptr)
        return surface;

    vk::Win32SurfaceCreateInfoKHR surfaceInfo {};
    surfaceInfo.setHwnd(reinterpret_cast<HWND>(windowHandle));

    m_surfaceCache.Insert(hash, std::make_shared<vk::UniqueSurfaceKHR>(m_instance.createWin32SurfaceKHRUnique(surfaceInfo).value));
    return m_surfaceCache.Get(hash);
}

inline static vk::DescriptorType GetDescriptorType(RHI::ShaderBindingResourceType resourceType)
{
    return {};
}

inline static vk::ShaderStageFlags GetStageFlags(RHI::Flags<RHI::ShaderType> shaderStages)
{
    return {};
}

std::shared_ptr<vk::UniqueDescriptorSetLayout> Context::CreateDescriptorSetLayout(const RHI::ShaderResourceGroupLayout& layout)
{
    if (auto descriptorSetLayout = m_descriptorSetLayoutCache.Get(layout.hash); descriptorSetLayout == nullptr)
        return descriptorSetLayout;

    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    uint32_t                                    bindingIndex = 0;
    for (auto binding : layout.bindings)
    {
        vk::DescriptorSetLayoutBinding newBinding {};
        newBinding.setBinding(bindingIndex);
        newBinding.setDescriptorType(GetDescriptorType(binding.type));
        newBinding.setDescriptorCount(binding.arrayCount);
        newBinding.setStageFlags(GetStageFlags(binding.stages));
        // newBinding.setImmutableSamplers(nullptr);
        bindingIndex++;
    }

    vk::DescriptorSetLayoutCreateInfo createInfo {};
    createInfo.setBindings(bindings);

    m_descriptorSetLayoutCache.Insert(
        layout.hash, std::make_shared<vk::UniqueDescriptorSetLayout>(m_device.createDescriptorSetLayoutUnique(createInfo).value));
    return m_descriptorSetLayoutCache.Get(layout.hash);
}

std::shared_ptr<vk::UniquePipelineLayout> Context::CreatePipelineLayout(const std::span<const RHI::ShaderResourceGroupLayout> layouts)
{
    size_t hash = std::accumulate(layouts.begin(), layouts.end(), 0, [](auto& a, auto& b) { return RHI::HashCombine(a.hash, b.hash); });

    if (auto pipelineLayout = m_pipelineLayoutCache.Get(hash); pipelineLayout != nullptr)
        return pipelineLayout;

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    for (auto layout : layouts)
    {
        auto descriptorSetLayout = CreateDescriptorSetLayout(layout);
        descriptorSetLayouts.push_back(descriptorSetLayout->get());
    }

    vk::PipelineLayoutCreateInfo createInfo {};
    createInfo.setSetLayouts(descriptorSetLayouts);

    m_pipelineLayoutCache.Insert(hash, std::make_shared<vk::UniquePipelineLayout>(m_device.createPipelineLayout(createInfo).value));
    return m_pipelineLayoutCache.Get(hash);
}

}  // namespace Vulkan