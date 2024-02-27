#include "RHI/Common//Callstack.hpp"

#include "RHI-Vulkan/Loader.hpp"

#include "Common.hpp"
#include "Resources.hpp"
#include "CommandList.hpp"
#include "FrameScheduler.hpp"
#include "Swapchain.hpp"

#include "Context.hpp"

#include <tracy/Tracy.hpp>

#if RHI_PLATFORM_WINDOWS
    #define VULKAN_SURFACE_OS_EXTENSION_NAME "VK_KHR_win32_surface"
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

namespace RHI
{
    Ptr<Context> CreateVulkanContext(const ApplicationInfo& appInfo, Ptr<DebugCallbacks> debugCallbacks)
    {
        ZoneScoped;

        auto context = CreatePtr<Vulkan::IContext>(std::move(debugCallbacks));
        auto result = context->Init(appInfo);
        RHI_ASSERT(result == VK_SUCCESS);
        return std::move(context);
    }
} // namespace RHI

namespace RHI::Vulkan
{
    std::vector<VkLayerProperties> _GetAvailableInstanceLayerExtensions()
    {
        uint32_t instanceLayerCount;
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> layers;
        layers.resize(instanceLayerCount);
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, layers.data());
        return layers;
    }

    std::vector<VkExtensionProperties> _GetAvailableInstanceExtensions()
    {
        uint32_t instanceExtensionsCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, nullptr);
        std::vector<VkExtensionProperties> extensions;
        extensions.resize(instanceExtensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, extensions.data());
        return extensions;
    }

    std::vector<VkLayerProperties> _GetAvailableDeviceLayerExtensions(VkPhysicalDevice physicalDevice)
    {
        uint32_t instanceLayerCount;
        vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> layers;
        layers.resize(instanceLayerCount);
        vkEnumerateDeviceLayerProperties(physicalDevice, &instanceLayerCount, layers.data());
        return layers;
    }

    std::vector<VkExtensionProperties> _GetAvailableDeviceExtensions(VkPhysicalDevice physicalDevice)
    {
        uint32_t extensionsCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr);
        std::vector<VkExtensionProperties> extnesions;
        extnesions.resize(extensionsCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, extnesions.data());
        return extnesions;
    }

    std::vector<VkPhysicalDevice> _GetAvailablePhysicalDevices(VkInstance instance)
    {
        uint32_t physicalDeviceCount;
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
        VkResult result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
        RHI_ASSERT(result == VK_SUCCESS);
        return physicalDevices;
    }

    std::vector<VkQueueFamilyProperties> _GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice)
    {
        uint32_t queueFamilyPropertiesCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties{};
        queueFamilyProperties.resize(queueFamilyPropertiesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());
        return queueFamilyProperties;
    }

    IContext::IContext(Ptr<DebugCallbacks> debugCallbacks)
        : Context(std::move(debugCallbacks))
    {
        m_frameScheduler = CreatePtr<IFrameScheduler>(this);
        // m_bindGroupAllocator = CreatePtr<BindGroupAllocator>();
        m_stagingBuffer = CreatePtr<IStagingBuffer>(this);
        m_transferCommandsAllocator = CreatePtr<ICommandListAllocator>(this);
    }

    IContext::~IContext()
    {
        ZoneScoped;

        DebugReportLiveResources();

        vkDeviceWaitIdle(m_device);

        DestroyResources();

        vmaDestroyAllocator(m_allocator);
        vkDestroyDevice(m_device, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }

    VkResult IContext::Init(const ApplicationInfo& appInfo)
    {
        ZoneScoped;

        bool debugExtensionEnabled = false;

        VkResult result = VK_ERROR_UNKNOWN;

        result = InitInstance(appInfo, &debugExtensionEnabled);
        VULKAN_RETURN_VKERR_CODE(result);

        result = InitDevice();
        VULKAN_RETURN_VKERR_CODE(result);

        result = InitDeviceQueues();
        VULKAN_RETURN_VKERR_CODE(result);

        result = InitMemoryAllocator();
        VULKAN_RETURN_VKERR_CODE(result);

        result = LoadFunctions(debugExtensionEnabled);
        VULKAN_RETURN_VKERR_CODE(result);

        result = InitFrameScheduler();
        VULKAN_RETURN_VKERR_CODE(result);

        result = InitStagingBuffer();
        VULKAN_RETURN_VKERR_CODE(result);

        m_bindGroupAllocator = CreatePtr<BindGroupAllocator>(m_device);

        auto scheduler = (IFrameScheduler*)m_frameScheduler.get();
        result = scheduler->Init();
        VULKAN_RETURN_VKERR_CODE(result);

        auto stagingBuffer = (IStagingBuffer*)m_stagingBuffer.get();
        result = stagingBuffer->Init();
        VULKAN_RETURN_VKERR_CODE(result);

        auto commandAllocator = (ICommandListAllocator*)m_transferCommandsAllocator.get();
        result = commandAllocator->Init(QueueType::Graphics);
        VULKAN_RETURN_VKERR_CODE(result);

        return VK_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    // Interface implementation
    ////////////////////////////////////////////////////////////
    Ptr<Swapchain> IContext::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        auto swapchain = CreatePtr<ISwapchain>(this);
        auto result = swapchain->Init(createInfo);
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create swapchain object");
        }
        return swapchain;
    }

    Ptr<ShaderModule> IContext::CreateShaderModule(TL::Span<const uint8_t> shaderBlob)
    {
        ZoneScoped;

        auto shaderModule = CreatePtr<IShaderModule>(this);
        auto result = shaderModule->Init(shaderBlob);
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create shader module");
        }
        return shaderModule;
    }

    Ptr<Fence> IContext::CreateFence()
    {
        ZoneScoped;

        auto fence = CreatePtr<IFence>(this);
        auto result = fence->Init();
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create a fence object");
        }
        return fence;
    }

    Ptr<CommandListAllocator> IContext::CreateCommandListAllocator(QueueType queueType)
    {
        ZoneScoped;

        auto commandListAllocator = CreatePtr<ICommandListAllocator>(this);
        auto result = commandListAllocator->Init(queueType);
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create a command_list_allocator object");
        }
        return commandListAllocator;
    }

    Ptr<ResourcePool> IContext::CreateResourcePool(const ResourcePoolCreateInfo& createInfo)
    {
        ZoneScoped;

        auto resourcePool = CreatePtr<IResourcePool>(this);
        auto result = resourcePool->Init(createInfo);
        if (result != VK_SUCCESS)
        {
            DebugLogError("Failed to create a resource_pool object");
        }
        return resourcePool;
    }

    Handle<BindGroupLayout> IContext::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        ZoneScoped;

        auto [handle, bindGroupLayout] = m_bindGroupLayoutsOwner.InsertZerod();
        auto result = bindGroupLayout.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create bindGroupLayout");
        }
        m_LeakDetector.m_bindGroupLayouts.OnCreate(handle);
        return handle;
    }

    void IContext::DestroyBindGroupLayout(Handle<BindGroupLayout> handle)
    {
        ZoneScoped;

        auto bindGroupLayout = m_bindGroupLayoutsOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ bindGroupLayout->Shutdown(this); });
        // clang-format on
        m_LeakDetector.m_bindGroupLayouts.OnDestroy(handle);
    }

    Handle<BindGroup> IContext::CreateBindGroup(Handle<BindGroupLayout> layoutHandle)
    {
        ZoneScoped;

        auto [handle, bindGroup] = m_bindGroupOwner.InsertZerod();
        auto result = bindGroup.Init(this, layoutHandle);
        if (IsError(result))
        {
            DebugLogError("Failed to create bindGroup");
        }
        m_LeakDetector.m_bindGroups.OnCreate(handle);
        return handle;
    }

    void IContext::DestroyBindGroup(Handle<BindGroup> handle)
    {
        ZoneScoped;

        auto bindGroup = m_bindGroupOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ bindGroup->Shutdown(this); });
        // clang-format on
        m_LeakDetector.m_bindGroups.OnDestroy(handle);
    }

    void IContext::UpdateBindGroup(Handle<BindGroup> handle, const BindGroupData& data)
    {
        ZoneScoped;

        auto bindGroup = m_bindGroupOwner.Get(handle);
        bindGroup->Write(this, data);
    }

    Handle<PipelineLayout> IContext::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        ZoneScoped;

        auto [handle, pipelineLayout] = m_pipelineLayoutOwner.InsertZerod();
        auto result = pipelineLayout.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create pipelineLayout");
        }
        m_LeakDetector.m_pipelineLayouts.OnCreate(handle);
        return handle;
    }

    void IContext::DestroyPipelineLayout(Handle<PipelineLayout> handle)
    {
        ZoneScoped;

        auto pipelineLayout = m_pipelineLayoutOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ pipelineLayout->Shutdown(this); });
        // clang-format on
        m_LeakDetector.m_pipelineLayouts.OnDestroy(handle);
    }

    Handle<GraphicsPipeline> IContext::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        ZoneScoped;

        auto [handle, graphicsPipeline] = m_graphicsPipelineOwner.InsertZerod();
        auto result = graphicsPipeline.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create graphicsPipeline");
        }
        m_LeakDetector.m_graphicsPipelines.OnCreate(handle);
        return handle;
    }

    void IContext::DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)
    {
        ZoneScoped;

        auto graphicsPipeline = m_graphicsPipelineOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ graphicsPipeline->Shutdown(this); });
        // clang-format on
        m_LeakDetector.m_graphicsPipelines.OnDestroy(handle);
    }

    Handle<ComputePipeline> IContext::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        ZoneScoped;

        auto [handle, computePipeline] = m_computePipelineOwner.InsertZerod();
        auto result = computePipeline.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create computePipeline");
        }
        m_LeakDetector.m_computePipelines.OnCreate(handle);
        return handle;
    }

    void IContext::DestroyComputePipeline(Handle<ComputePipeline> handle)
    {
        ZoneScoped;

        auto computePipeline = m_computePipelineOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ computePipeline->Shutdown(this); });
        // clang-format on
        m_LeakDetector.m_computePipelines.OnDestroy(handle);
    }

    Handle<Sampler> IContext::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        ZoneScoped;

        auto [handle, sampler] = m_samplerOwner.InsertZerod();
        auto result = sampler.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create sampler");
        }
        m_LeakDetector.m_samplers.OnCreate(handle);
        return handle;
    }

    void IContext::DestroySampler(Handle<Sampler> handle)
    {
        ZoneScoped;

        auto sampler = m_samplerOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ sampler->Shutdown(this); });
        // clang-format on
        m_LeakDetector.m_samplers.OnDestroy(handle);
    }

    Result<Handle<Image>> IContext::CreateImage(const ImageCreateInfo& createInfo)
    {
        ZoneScoped;

        auto [handle, image] = m_imageOwner.InsertZerod();
        auto result = image.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create image");
            return result;
        }
        m_LeakDetector.m_images.OnCreate(handle);
        return Result<Handle<Image>>(handle);
    }

    void IContext::DestroyImage(Handle<Image> handle)
    {
        ZoneScoped;

        auto image = m_imageOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ image->Shutdown(this); });
        // clang-format on
        m_LeakDetector.m_images.OnDestroy(handle);
    }

    Result<Handle<Buffer>> IContext::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        ZoneScoped;

        auto [handle, buffer] = m_bufferOwner.InsertZerod();
        auto result = buffer.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create buffer");
            return result;
        }
        m_LeakDetector.m_buffers.OnCreate(handle);
        return Result<Handle<Buffer>>(handle);
    }

    void IContext::DestroyBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;

        auto buffer = m_bufferOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ buffer->Shutdown(this); });
        // clang-format on
        m_LeakDetector.m_buffers.OnDestroy(handle);
    }

    Handle<ImageView> IContext::CreateImageView(const ImageViewCreateInfo& createInfo)
    {
        ZoneScoped;

        auto [handle, imageView] = m_imageViewOwner.InsertZerod();
        auto result = imageView.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create image view");
        }
        m_LeakDetector.m_imageViews.OnCreate(handle);
        return handle;
    }

    void IContext::DestroyImageView(Handle<ImageView> handle)
    {
        ZoneScoped;

        auto imageView = m_imageViewOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ imageView->Shutdown(this); });
        // clang-format on
        m_LeakDetector.m_imageViews.OnDestroy(handle);
    }

    Handle<BufferView> IContext::CreateBufferView(const BufferViewCreateInfo& createInfo)
    {
        ZoneScoped;

        auto [handle, bufferView] = m_bufferViewOwner.InsertZerod();
        auto result = bufferView.Init(this, createInfo);
        if (IsError(result))
        {
            DebugLogError("Failed to create buffer view");
        }
        m_LeakDetector.m_bufferViews.OnCreate(handle);
        return handle;
    }

    void IContext::DestroyBufferView(Handle<BufferView> handle)
    {
        ZoneScoped;

        auto imageView = m_bufferViewOwner.Get(handle);
        // clang-format off
        m_deferDeleteQueue.push_back([&](){ imageView->Shutdown(this); });
        // clang-format on
        m_LeakDetector.m_bufferViews.OnDestroy(handle);
    }

    DeviceMemoryPtr IContext::MapBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;

        // TODO: Remove from here (wrap as new function Resources.hpp)
        auto resource = m_bufferOwner.Get(handle);
        auto allocation = resource->allocation.handle;

        DeviceMemoryPtr memoryPtr = nullptr;
        VkResult result = vmaMapMemory(m_allocator, allocation, &memoryPtr);
        VULKAN_ASSERT_SUCCESS(result);
        return memoryPtr;
    }

    void IContext::UnmapBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;

        // TODO: Remove from here (wrap as new function Resources.hpp)
        auto resource = m_bufferOwner.Get(handle)->allocation.handle;
        vmaUnmapMemory(m_allocator, resource);
    }

    ////////////////////////////////////////////////////////////
    // Interface implementation
    ////////////////////////////////////////////////////////////

    void IContext::DestroyResources()
    {
        ZoneScoped;

        for (auto destroyItem : m_deferDeleteQueue)
        {
            destroyItem();
        }
        m_deferDeleteQueue.clear();
    }

    VkSemaphore IContext::CreateSemaphore()
    {
        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        VkSemaphore semaphore = VK_NULL_HANDLE;
        auto result = vkCreateSemaphore(m_device, &createInfo, nullptr, &semaphore);
        VULKAN_ASSERT_SUCCESS(result);
        return semaphore;
    }

    void IContext::FreeSemaphore(VkSemaphore semaphore)
    {
        ZoneScoped;

        if (semaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device, semaphore, nullptr);
        }
    }

    uint32_t IContext::GetMemoryTypeIndex(MemoryType memoryType)
    {
        VkMemoryPropertyFlags flags = 0;
        VkMemoryPropertyFlags negateFlags = 0;
        switch (memoryType)
        {
        case MemoryType::CPU: flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; break;
        case MemoryType::GPULocal:
            flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            negateFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
        case MemoryType::GPUShared: flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; break;
        }

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

        // TODO: if multiple memory types with the desired flags are present,
        // then select the based on size, performance charactersitcs ...
        uint32_t index = UINT32_MAX;
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        {
            VkMemoryType type = memoryProperties.memoryTypes[i];

            if ((type.propertyFlags & flags) == flags && (type.propertyFlags & negateFlags) == 0)
            {
                index = type.heapIndex;
            }
        }

        return index;
    }

    VkBool32 IContext::DebugMessengerCallbacks(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        (void)messageTypes;
        auto context = (IContext*)pUserData;

        if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            context->DebugLogError(pCallbackData->pMessage);
        }
        else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            context->DebugLogWarn(pCallbackData->pMessage);
        }
        else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            context->DebugLogInfo(pCallbackData->pMessage);
        }
        else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            context->DebugLogInfo(pCallbackData->pMessage);
        }

        return VK_FALSE;
    }

    VkResult IContext::InitInstance(const ApplicationInfo& appInfo, bool* debugExtensionEnabled)
    {
        std::vector<const char*> enabledLayersNames = {
            "VK_LAYER_KHRONOS_validation",
        };

        std::vector<const char*> enabledExtensionsNames = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VULKAN_SURFACE_OS_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        };

        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pNext = nullptr;
        applicationInfo.pApplicationName = appInfo.applicationName;
        applicationInfo.applicationVersion = VK_MAKE_API_VERSION(0, appInfo.applicationVersion.major, appInfo.applicationVersion.minor, appInfo.applicationVersion.patch);
        applicationInfo.pEngineName = appInfo.engineName;
        applicationInfo.engineVersion = VK_MAKE_API_VERSION(0, appInfo.engineVersion.major, appInfo.engineVersion.minor, appInfo.engineVersion.patch);
        applicationInfo.apiVersion = VK_API_VERSION_1_3;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugMessengerCallbacks;
        debugCreateInfo.pUserData = this;

#if RHI_DEBUG
        for (VkExtensionProperties extension : _GetAvailableInstanceExtensions())
        {
            auto extensionName = extension.extensionName;
            if (!strcmp(extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
            {
                *debugExtensionEnabled = true;
                continue;
            }
        }

        if (*debugExtensionEnabled)
        {
            enabledExtensionsNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            enabledExtensionsNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            
        }
        else
        {
            DebugLogWarn("RHI Vulkan: Debug extension not present.\n Vulkan layer validation is disabled.");
        }
#endif

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = *debugExtensionEnabled ? &debugCreateInfo : nullptr;
        createInfo.flags = {};
        createInfo.pApplicationInfo = &applicationInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayersNames.size());
        createInfo.ppEnabledLayerNames = enabledLayersNames.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensionsNames.size());
        createInfo.ppEnabledExtensionNames = enabledExtensionsNames.data();
        return vkCreateInstance(&createInfo, nullptr, &m_instance);
    }

    VkResult IContext::InitDevice()
    {
        for (VkPhysicalDevice physicalDevice : _GetAvailablePhysicalDevices(m_instance))
        {
            bool swapchainExtension = false;
            bool dynamicRenderingExtension = false;
            bool maintenance2Extension = false;
            bool multiviewExtension = false;
            bool createRenderpass2Extension = false;
            bool depthStencilResolveExtension = false;

            for (auto extension : _GetAvailableDeviceExtensions(physicalDevice))
            {
                swapchainExtension |= strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0;
                dynamicRenderingExtension |= strcmp(extension.extensionName, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0;
                maintenance2Extension |= strcmp(extension.extensionName, VK_KHR_MAINTENANCE2_EXTENSION_NAME) == 0;
                multiviewExtension |= strcmp(extension.extensionName, VK_KHR_MULTIVIEW_EXTENSION_NAME) == 0;
                createRenderpass2Extension |= strcmp(extension.extensionName, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME) == 0;
                depthStencilResolveExtension |= strcmp(extension.extensionName, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME) == 0;
            }

            if (swapchainExtension && dynamicRenderingExtension && maintenance2Extension && multiviewExtension && createRenderpass2Extension && depthStencilResolveExtension)
            {
                m_physicalDevice = physicalDevice;
                break;
            }
        }

        std::vector<const char*> deviceLayerNames = {

        };

        std::vector<const char*> deviceExtensionNames = {
#if RHI_DEBUG
            VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
#endif
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        auto queueFamilyProperties = _GetPhysicalDeviceQueueFamilyProperties(m_physicalDevice);
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
            {
                m_transferQueueFamilyIndex = queueFamilyIndex;
            }

            // Search for transfer queue
            if ((queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT && (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
            {
                m_computeQueueFamilyIndex = queueFamilyIndex;
            }
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
        enabledFeatures.samplerAnisotropy = VK_TRUE;

        VkPhysicalDeviceSynchronization2Features syncFeature{};
        syncFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        syncFeature.synchronization2 = VK_TRUE;

        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
        dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamicRenderingFeatures.pNext = &syncFeature;
        dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &dynamicRenderingFeatures;
        createInfo.flags = 0;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(deviceLayerNames.size());
        createInfo.ppEnabledLayerNames = deviceLayerNames.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionNames.size());
        createInfo.ppEnabledExtensionNames = deviceExtensionNames.data();
        createInfo.pEnabledFeatures = &enabledFeatures;
        return vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);
    }

    VkResult IContext::InitDeviceQueues()
    {
        vkGetDeviceQueue(m_device, m_graphicsQueueFamilyIndex, 0, &m_graphicsQueue);

        m_computeQueue = m_graphicsQueue;
        m_transferQueue = m_graphicsQueue;

        // if (m_computeQueueFamilyIndex != UINT32_MAX)
        //     vkGetDeviceQueue(m_device, m_computeQueueFamilyIndex, 0, &m_computeQueue);

        // if (m_transferQueueFamilyIndex != UINT32_MAX)
        //     vkGetDeviceQueue(m_device, m_transferQueueFamilyIndex, 0, &m_transferQueue);

        return VK_SUCCESS;
    }

    VkResult IContext::InitMemoryAllocator()
    {
        VmaAllocatorCreateInfo createInfo{};
        createInfo.physicalDevice = m_physicalDevice;
        createInfo.device = m_device;
        createInfo.instance = m_instance;
        createInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        return vmaCreateAllocator(&createInfo, &m_allocator);
    }

    VkResult IContext::LoadFunctions(bool debugExtensionEnabled)
    {
#if RHI_DEBUG
        if (debugExtensionEnabled)
        {
            m_vkCmdDebugMarkerBeginEXT = VULKAN_LOAD_PROC(m_device, vkCmdDebugMarkerBeginEXT);
            m_vkCmdDebugMarkerInsertEXT = VULKAN_LOAD_PROC(m_device, vkCmdDebugMarkerInsertEXT);
            m_vkCmdDebugMarkerEndEXT = VULKAN_LOAD_PROC(m_device, vkCmdDebugMarkerEndEXT);

            RHI_ASSERT(m_vkCmdDebugMarkerBeginEXT);
            RHI_ASSERT(m_vkCmdDebugMarkerInsertEXT);
            RHI_ASSERT(m_vkCmdDebugMarkerEndEXT);
        }
#endif
        return VK_SUCCESS;
    }

    VkResult IContext::InitFrameScheduler()
    {
        auto scheduler = (IFrameScheduler*)m_frameScheduler.get();
        scheduler->Init();
        scheduler->SetBufferedFramesCount(2);
        return VK_SUCCESS;
    }

    VkResult IContext::InitStagingBuffer()
    {
        return VK_SUCCESS;
    }

} // namespace RHI::Vulkan