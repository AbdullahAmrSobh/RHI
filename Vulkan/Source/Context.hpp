#pragma once

#include <RHI/Context.hpp>
#include <RHI/FrameScheduler.hpp>

#include <vk_mem_alloc.h>

namespace Vulkan
{
    struct IImage;
    struct IBuffer;
    struct IImageView;
    struct IBufferView;
    struct IBindGroupLayout;
    struct IBindGroup;
    struct IPipelineLayout;
    struct IGraphicsPipeline;
    struct IComputePipeline;
    struct ISampler;

    class IContext final : public RHI::Context
    {
    public:
        IContext(std::unique_ptr<RHI::DebugCallbacks> debugMessengerCallbacks)
            : RHI::Context(std::move(debugMessengerCallbacks))
        {
        }

        ~IContext();

        VkResult Init(const RHI::ApplicationInfo& appInfo);

        std::unique_ptr<RHI::Swapchain> CreateSwapchain(const RHI::SwapchainCreateInfo& createInfo) override;

        std::unique_ptr<RHI::Fence> CreateFence() override;

        std::unique_ptr<RHI::Pass> CreatePass(const char* name, RHI::QueueType type) override;

        std::unique_ptr<RHI::FrameScheduler> CreateFrameScheduler() override;

        std::unique_ptr<RHI::CommandListAllocator> CreateCommandListAllocator(RHI::QueueType queueType, uint32_t bufferedFramesCount) override;

        std::unique_ptr<RHI::ShaderModule> CreateShaderModule(const RHI::ShaderModuleCreateInfo& createInfo) override;

        RHI::Handle<RHI::BindGroupLayout> CreateBindGroupLayout(const RHI::BindGroupLayoutCreateInfo& createInfo) override;
        void DestroyBindGroupLayout(RHI::Handle<RHI::BindGroupLayout> layout) override;

        RHI::Handle<RHI::PipelineLayout> CreatePipelineLayout(const RHI::PipelineLayoutCreateInfo& createInfo) override;
        void DestroyPipelineLayout(RHI::Handle<RHI::PipelineLayout> layout) override;

        std::unique_ptr<RHI::BindGroupAllocator> CreateBindGroupAllocator() override;

        std::unique_ptr<RHI::BufferPool> CreateBufferPool(const RHI::PoolCreateInfo& createInfo) override;
        std::unique_ptr<RHI::ImagePool> CreateImagePool(const RHI::PoolCreateInfo& createInfo) override;

        RHI::Handle<RHI::GraphicsPipeline> CreateGraphicsPipeline(const RHI::GraphicsPipelineCreateInfo& createInfo) override;
        void DestroyGraphicsPipeline(RHI::Handle<RHI::GraphicsPipeline> pso) override;

        RHI::Handle<RHI::ComputePipeline> CreateComputePipeline(const RHI::ComputePipelineCreateInfo& createInfo) override;
        void DestroyComputePipeline(RHI::Handle<RHI::ComputePipeline> pso) override;

        RHI::Handle<RHI::Sampler> CreateSampler(const RHI::SamplerCreateInfo& createInfo) override;
        void DestroySampler(RHI::Handle<RHI::Sampler> sampler) override;

        RHI::Handle<RHI::ImageView> CreateImageView(RHI::Handle<RHI::Image> handle, const RHI::ImageViewCreateInfo& useInfo) override;
        void DestroyImageView(RHI::Handle<RHI::ImageView> view) override;

        RHI::Handle<RHI::BufferView> CreateBufferView(RHI::Handle<RHI::Buffer> handle, const RHI::BufferViewCreateInfo& useInfo) override;
        void DestroyBufferView(RHI::Handle<RHI::BufferView> view) override;

        uint32_t GetQueueFamilyIndex(RHI::QueueType queueType) const;
        VkQueue GetQueue(RHI::QueueType queueType) const;

        inline VkSemaphore CreateVulkanSemaphore() { return CreateSemaphore(); }

        VkSemaphore CreateSemaphore();
        void FreeSemaphore(VkSemaphore semaphore);

        std::vector<VkLayerProperties> GetAvailableInstanceLayerExtensions() const;

        std::vector<VkExtensionProperties> GetAvailableInstanceExtensions() const;

        std::vector<VkLayerProperties> GetAvailableDeviceLayerExtensions(VkPhysicalDevice physicalDevice) const;

        std::vector<VkExtensionProperties> GetAvailableDeviceExtensions(VkPhysicalDevice physicalDevice) const;

        std::vector<VkPhysicalDevice> GetAvailablePhysicalDevices() const;

        std::vector<VkQueueFamilyProperties> GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice) const;

        uint32_t GetMemoryTypeIndex(RHI::MemoryType memoryType);

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

        PFN_vkCmdDebugMarkerBeginEXT m_vkCmdDebugMarkerBeginEXT = nullptr;
        PFN_vkCmdDebugMarkerInsertEXT m_vkCmdDebugMarkerInsertEXT = nullptr;
        PFN_vkCmdDebugMarkerEndEXT m_vkCmdDebugMarkerEndEXT = nullptr;

        RHI::HandlePool<IImage> m_imageOwner = RHI::HandlePool<IImage>();
        RHI::HandlePool<IBuffer> m_bufferOwner = RHI::HandlePool<IBuffer>();
        RHI::HandlePool<IImageView> m_imageViewOwner = RHI::HandlePool<IImageView>();
        RHI::HandlePool<IBufferView> m_bufferViewOwner = RHI::HandlePool<IBufferView>();
        RHI::HandlePool<IBindGroupLayout> m_bindGroupLayoutsOwner = RHI::HandlePool<IBindGroupLayout>();
        RHI::HandlePool<IBindGroup> m_bindGroupOwner = RHI::HandlePool<IBindGroup>();
        RHI::HandlePool<IPipelineLayout> m_pipelineLayoutOwner = RHI::HandlePool<IPipelineLayout>();
        RHI::HandlePool<IGraphicsPipeline> m_graphicsPipelineOwner = RHI::HandlePool<IGraphicsPipeline>();
        RHI::HandlePool<IComputePipeline> m_computePipelineOwner = RHI::HandlePool<IComputePipeline>();
        RHI::HandlePool<ISampler> m_samplerOwner = RHI::HandlePool<ISampler>();
    };

    inline uint32_t IContext::GetQueueFamilyIndex(RHI::QueueType queueType) const
    {
        switch (queueType)
        {
        case RHI::QueueType::Graphics: return m_graphicsQueueFamilyIndex;
        case RHI::QueueType::Compute:  return m_computeQueueFamilyIndex;
        case RHI::QueueType::Transfer: return m_transferQueueFamilyIndex;
        default:                       RHI_UNREACHABLE(); return UINT32_MAX;
        }
    }

    inline VkQueue IContext::GetQueue(RHI::QueueType queueType) const
    {
        switch (queueType)
        {
        case RHI::QueueType::Graphics: return m_graphicsQueue;
        case RHI::QueueType::Compute:  return m_computeQueue;
        case RHI::QueueType::Transfer: return m_transferQueue;
        default:                       RHI_UNREACHABLE(); return VK_NULL_HANDLE;
        }
    }

} // namespace Vulkan