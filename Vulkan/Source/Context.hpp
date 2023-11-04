#pragma once

#include <RHI/Context.hpp>
#include <RHI/FrameGraph.hpp>

#include <vk_mem_alloc.h>

#include <span>

namespace Vulkan
{
    class ResourceManager;

    class Context final : public RHI::Context
    {
    public:
        Context();
        ~Context();

        VkResult Init(const RHI::ApplicationInfo& appInfo, std::unique_ptr<RHI::DebugCallbacks> debugCallbacks);

        uint32_t GetQueueFamilyIndex(RHI::QueueType queueType) const
        {
            switch (queueType)
            {
            case RHI::QueueType::Graphics: return m_graphicsQueueFamilyIndex;
            case RHI::QueueType::Compute:  return m_computeQueueFamilyIndex;
            case RHI::QueueType::Transfer: return m_transferQueueFamilyIndex;
            default:                       RHI_UNREACHABLE(); return UINT32_MAX;
            }
        }

        std::unique_ptr<RHI::ShaderModule> CreateShaderModule(const RHI::ShaderModuleCreateInfo& createInfo) override;

        std::unique_ptr<RHI::Swapchain> CreateSwapchain(const RHI::SwapchainCreateInfo& createInfo) override;

        std::unique_ptr<RHI::ResourcePool> CreateResourcePool(const RHI::ResourcePoolCreateInfo& createInfo) override;

        RHI::Handle<RHI::GraphicsPipeline> CreateGraphicsPipeline(const RHI::GraphicsPipelineCreateInfo& createInfo) override;

        RHI::Handle<RHI::ComputePipeline> CreateComputePipeline(const RHI::ComputePipelineCreateInfo& createInfo) override;

        RHI::Handle<RHI::Sampler> CreateSampler(const RHI::SamplerCreateInfo& createInfo) override;

        std::unique_ptr<RHI::FrameScheduler> CreateFrameScheduler() override;

        std::unique_ptr<RHI::ShaderBindGroupAllocator> CreateShaderBindGroupAllocator() override;

        RHI::DeviceMemoryPtr MapResource(RHI::Handle<RHI::Image> image) override;

        RHI::DeviceMemoryPtr MapResource(RHI::Handle<RHI::Buffer> buffer) override;

        void Unmap(RHI::Handle<RHI::Image> image) override;

        void Unmap(RHI::Handle<RHI::Buffer> buffer) override;

        void Free(RHI::Handle<RHI::GraphicsPipeline> pso) override;

        void Free(RHI::Handle<RHI::ComputePipeline> pso) override;

        void Free(RHI::Handle<RHI::Sampler> sampler) override;

    private:
        std::vector<VkLayerProperties> GetAvailableInstanceLayerExtensions() const;

        std::vector<VkExtensionProperties> GetAvailableInstanceExtensions() const;

        std::vector<VkLayerProperties> GetAvailableDeviceLayerExtensions(VkPhysicalDevice physicalDevice) const;

        std::vector<VkExtensionProperties> GetAvailableDeviceExtensions(VkPhysicalDevice physicalDevice) const;

        std::vector<VkPhysicalDevice> GetAvailablePhysicalDevices() const;

        std::vector<VkQueueFamilyProperties> GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice) const;

    public:
        VkInstance m_instance = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VmaAllocator m_allocator = VK_NULL_HANDLE;

        uint32_t m_graphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t m_computeQueueFamilyIndex = UINT32_MAX;
        uint32_t m_transferQueueFamilyIndex = UINT32_MAX;

        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_computeQueue = VK_NULL_HANDLE;
        VkQueue m_transferQueue = VK_NULL_HANDLE;

        std::unique_ptr<ResourceManager> m_resourceManager;

        // Vulkan extension functions
        PFN_vkCmdDebugMarkerBeginEXT m_vkCmdDebugMarkerBeginEXT = nullptr;
        PFN_vkCmdDebugMarkerInsertEXT m_vkCmdDebugMarkerInsertEXT = nullptr;
        PFN_vkCmdDebugMarkerEndEXT m_vkCmdDebugMarkerEndEXT = nullptr;
    };

} // namespace Vulkan