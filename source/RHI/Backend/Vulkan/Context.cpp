#define VK_USE_PLATFORM_WIN32_KHR
#include "RHI/Backend/Vulkan/Context.hpp"

#include "RHI/Backend/Vulkan/Conversion.inl"
#include "RHI/Backend/Vulkan/Resources.hpp"

namespace Vulkan
{

inline static VkBool32 VKAPI_CALL DebugMessengerCallbacks(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                          VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                                                          const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                          void*                                       pUserData)
{
    (void)messageTypes;
    (void)pUserData;

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        RHI_LOG(pCallbackData->pMessage);
    }
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        RHI_LOG(pCallbackData->pMessage);
    }
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        RHI_WARN(pCallbackData->pMessage);
    }
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        RHI_ERROR(pCallbackData->pMessage);
    }

    return VK_FALSE;
}

RHI::ResultCode Context::Init(const RHI::ApplicationInfo& appInfo)
{
    std::vector<const char*> enabledLayersNames;
    enabledLayersNames.push_back("VK_LAYER_KHRONOS_validation");

    std::vector<const char*> enabledExtensionsNames;
    enabledExtensionsNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    enabledExtensionsNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    enabledExtensionsNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    enabledExtensionsNames.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

    vk::ApplicationInfo vkAppInfo;
    vkAppInfo.setPApplicationName(appInfo.applicationName);
    vkAppInfo.setApplicationVersion(appInfo.applicationVersion);
    vkAppInfo.setPEngineName(appInfo.engineName);
    vkAppInfo.setEngineVersion(appInfo.engineVersion);
    vkAppInfo.setApiVersion(VK_API_VERSION_1_3);

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags {};
    severityFlags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
    severityFlags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

    vk::DebugUtilsMessageTypeFlagsEXT typeFlags {};
    typeFlags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral;
    typeFlags |= vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    typeFlags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    debugCreateInfo.setMessageSeverity(severityFlags);
    debugCreateInfo.setMessageType(typeFlags);
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
    if (m_allocator)
    {
        vmaDestroyAllocator(m_allocator);
    }

    m_device.destroy();
    m_instance.destroy();
}

RHI::ResultCode Context::SetDevice(uint32_t deviceId)
{
    m_physicalDevice = m_instance.enumeratePhysicalDevices().value[deviceId];

    if (m_device)
    {
        RHI_ASSERT_MSG(m_device.waitIdle() == vk::Result::eSuccess, "Timeout");

        if (m_allocator)
        {
            vmaDestroyAllocator(m_allocator);
        }

        m_device.destroy();
    }

    vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures {};
    dynamicRenderingFeatures.setDynamicRendering(VK_TRUE);

    vk::DeviceCreateInfo createInfo {};
    createInfo.setPNext(&dynamicRenderingFeatures);

    vk::PhysicalDeviceFeatures             features {};
    std::vector<vk::DeviceQueueCreateInfo> queues {};
    std::vector<const char*>               enabledLayers {};
    std::vector<const char*>               enabledExtensions {};
    enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    enabledExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    enabledExtensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    enabledExtensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    enabledExtensions.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    enabledExtensions.push_back(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);

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

    float                     prio = 1.0;
    vk::DeviceQueueCreateInfo queueInfo;
    queueInfo.setQueueCount(1);
    queueInfo.setQueuePriorities(prio);

    if (graphicsQueueIndex != -1)
    {
        queueInfo.setQueueFamilyIndex(static_cast<uint32_t>(graphicsQueueIndex));
        queues.push_back(queueInfo);
    }

    if (graphicsQueueIndex != -1)
    {
        queueInfo.setQueueFamilyIndex(static_cast<uint32_t>(computeQueueIndex));
        queues.push_back(queueInfo);
    }

    if (transferQueueIndex != -1)
    {
        queueInfo.setQueueFamilyIndex(static_cast<uint32_t>(transferQueueIndex));
        queues.push_back(queueInfo);
    }

    createInfo.setQueueCreateInfos(queues);
    createInfo.setPEnabledLayerNames(enabledLayers);
    createInfo.setPEnabledExtensionNames(enabledExtensions);
    createInfo.setPEnabledFeatures(&features);

    m_device = m_physicalDevice.createDevice(createInfo).value;

    m_graphicsQueue = m_device.getQueue(static_cast<uint32_t>(graphicsQueueIndex), 0);
    m_computeQueue  = computeQueueIndex != -1 ? m_device.getQueue(static_cast<uint32_t>(computeQueueIndex), 0u) : m_graphicsQueue;
    m_transferQueue = transferQueueIndex != -1 ? m_device.getQueue(static_cast<uint32_t>(computeQueueIndex), 0u) : m_graphicsQueue;

    // Create VmaAllocator Instance
    {
        VmaAllocatorCreateInfo vmaCreateInfo {};
        vmaCreateInfo.instance         = m_instance;
        vmaCreateInfo.physicalDevice   = m_physicalDevice;
        vmaCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        vmaCreateInfo.device           = m_device;

        VkResult result = vmaCreateAllocator(&vmaCreateInfo, &m_allocator);
        RHI_ASSERT_MSG(result == VK_SUCCESS, "Failed to create VmaAllocator");
    }

    return RHI::ResultCode::Success;
}

void Context::SetImageContent(RHI::Image& _image, size_t byteOffset, void* data, size_t byteSize)
{
    Image& image = static_cast<Image&>(_image);
    void*  imageData;
    RHI_ASSERT_MSG(vmaMapMemory(m_allocator, image.m_allocation, &imageData) == VK_SUCCESS, "Failed to map resource's memory");
    memcpy(static_cast<uint8_t*>(imageData) + byteOffset, data, byteSize);
    vmaUnmapMemory(m_allocator, image.m_allocation);
}

void Context::SetBufferContent(RHI::Buffer& _buffer, size_t byteOffset, void* data, size_t byteSize)
{
    Buffer& buffer = static_cast<Buffer&>(_buffer);
    void*   bufferData;
    RHI_ASSERT_MSG(vmaMapMemory(m_allocator, buffer.m_allocation, &bufferData) == VK_SUCCESS, "Failed to map resource's memory");
    memcpy(static_cast<uint8_t*>(bufferData) + byteOffset, data, byteSize);
    vmaUnmapMemory(m_allocator, buffer.m_allocation);
}

vk::UniqueShaderModule Context::CreateModule(std::vector<uint32_t> code)
{
    vk::ShaderModuleCreateInfo createInfo {};
    createInfo.setCode(code);
    return m_device.createShaderModuleUnique(createInfo).value;
}

std::shared_ptr<vk::UniqueSurfaceKHR> Context::CreateSurface(void* windowHandle)
{
    size_t hash = RHI::HashAny(windowHandle);
    if (auto surface = m_surfaceCache.Get(hash); surface != nullptr)
        return surface;

    vk::Win32SurfaceCreateInfoKHR surfaceInfo {};
    surfaceInfo.setHwnd(reinterpret_cast<HWND>(windowHandle));

    auto surface = std::make_shared<vk::UniqueSurfaceKHR>(m_instance.createWin32SurfaceKHRUnique(surfaceInfo).value);
    m_surfaceCache.Insert(hash, surface);

    return surface;
}

std::shared_ptr<vk::UniqueDescriptorSetLayout> Context::CreateDescriptorSetLayout(const RHI::ShaderResourceGroupLayout& layout)
{
    size_t hash = layout.hash;
    if (auto descriptorLayout = m_descriptorSetLayoutCache.Get(hash); descriptorLayout != nullptr)
    {
        return descriptorLayout;
    }

    std::vector<vk::DescriptorSetLayoutBinding> bindings;

    uint32_t bindingLocation = 0;
    for (auto shaderResource : layout.bindings)
    {
        if (shaderResource.type == RHI::ShaderResourceType::ConstantBuffer)
            continue;

        vk::DescriptorSetLayoutBinding resourceBinding {};
        resourceBinding.setBinding(bindingLocation);
        resourceBinding.setDescriptorType(GetDescriptorType(shaderResource.type));
        resourceBinding.setDescriptorCount(shaderResource.arrayCount);
        resourceBinding.setStageFlags(ConvertShaderStages(shaderResource.stages));
        // resourceBinding.setImmutableSamplers();
    }

    vk::DescriptorSetLayoutCreateInfo createInfo {};

    auto dsl = std::make_shared<vk::UniqueDescriptorSetLayout>(m_device.createDescriptorSetLayoutUnique(createInfo).value);
    m_descriptorSetLayoutCache.Insert(hash, dsl);

    return dsl;
}

std::shared_ptr<vk::UniquePipelineLayout> Context::CreatePipelineLayout(const std::vector<RHI::ShaderResourceGroupLayout>& layouts)
{
    size_t hash = std::accumulate(
        layouts.begin(), layouts.end(), 0ull, [](size_t h, const auto& binding) { return RHI::HashCombine(h, RHI::HashAny(binding)); });

    if (auto pipelineLayout = m_pipelineLayoutCache.Get(hash); pipelineLayout != nullptr)
    {
        return pipelineLayout;
    }

    std::vector<vk::DescriptorSetLayout> setsLayout {};
    std::vector<vk::PushConstantRange>   constantBuffers {};

    for (const auto& layout : layouts)
    {
        auto dsl = CreateDescriptorSetLayout(layout);
        setsLayout.push_back(dsl->get());
    }

    vk::PipelineLayoutCreateInfo createInfo {};
    createInfo.setSetLayouts(setsLayout);
    createInfo.setPushConstantRanges(constantBuffers);

    auto pipelineLayout = std::make_shared<vk::UniquePipelineLayout>(m_device.createPipelineLayoutUnique(createInfo).value);
    m_pipelineLayoutCache.Insert(hash, pipelineLayout);

    return pipelineLayout;
}

}  // namespace Vulkan