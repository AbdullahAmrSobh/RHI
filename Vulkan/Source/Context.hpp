#pragma once

#include <RHI/Context.hpp>
#include <RHI/FrameScheduler.hpp>

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
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

    class IContext final : public Context
    {
    public:
        IContext(std::unique_ptr<DebugCallbacks> debugMessengerCallbacks)
            : Context(std::move(debugMessengerCallbacks))
        {
        }

        ~IContext();

        VkResult Init(const ApplicationInfo& appInfo);

        std::unique_ptr<Swapchain> CreateSwapchain(const SwapchainCreateInfo& createInfo) override;

        std::unique_ptr<Fence> CreateFence() override;

        std::unique_ptr<Pass> CreatePass(const char* name, QueueType type) override;

        std::unique_ptr<FrameScheduler> CreateFrameScheduler() override;

        std::unique_ptr<CommandListAllocator> CreateCommandListAllocator(QueueType queueType, uint32_t bufferedFramesCount) override;

        std::unique_ptr<ShaderModule> CreateShaderModule(const ShaderModuleCreateInfo& createInfo) override;

        Handle<BindGroupLayout> CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void DestroyBindGroupLayout(Handle<BindGroupLayout> layout) override;

        Handle<PipelineLayout> CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) override;
        void DestroyPipelineLayout(Handle<PipelineLayout> layout) override;

        std::unique_ptr<BindGroupAllocator> CreateBindGroupAllocator() override;

        std::unique_ptr<BufferPool> CreateBufferPool(const PoolCreateInfo& createInfo) override;
        std::unique_ptr<ImagePool> CreateImagePool(const PoolCreateInfo& createInfo) override;

        Handle<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void DestroyGraphicsPipeline(Handle<GraphicsPipeline> pso) override;

        Handle<ComputePipeline> CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
        void DestroyComputePipeline(Handle<ComputePipeline> pso) override;

        Handle<Sampler> CreateSampler(const SamplerCreateInfo& createInfo) override;
        void DestroySampler(Handle<Sampler> sampler) override;

        Handle<ImageView> CreateImageView(Handle<Image> handle, const ImageViewCreateInfo& useInfo) override;
        void DestroyImageView(Handle<ImageView> view) override;

        Handle<BufferView> CreateBufferView(Handle<Buffer> handle, const BufferViewCreateInfo& useInfo) override;
        void DestroyBufferView(Handle<BufferView> view) override;

        uint32_t GetQueueFamilyIndex(QueueType queueType) const;
        VkQueue GetQueue(QueueType queueType) const;

        inline VkSemaphore CreateVulkanSemaphore() { return CreateSemaphore(); }

        VkSemaphore CreateSemaphore();
        void FreeSemaphore(VkSemaphore semaphore);

        std::vector<VkLayerProperties> GetAvailableInstanceLayerExtensions() const;

        std::vector<VkExtensionProperties> GetAvailableInstanceExtensions() const;

        std::vector<VkLayerProperties> GetAvailableDeviceLayerExtensions(VkPhysicalDevice physicalDevice) const;

        std::vector<VkExtensionProperties> GetAvailableDeviceExtensions(VkPhysicalDevice physicalDevice) const;

        std::vector<VkPhysicalDevice> GetAvailablePhysicalDevices() const;

        std::vector<VkQueueFamilyProperties> GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice) const;

        uint32_t GetMemoryTypeIndex(MemoryType memoryType);

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

        HandlePool<IImage> m_imageOwner = HandlePool<IImage>();
        HandlePool<IBuffer> m_bufferOwner = HandlePool<IBuffer>();
        HandlePool<IImageView> m_imageViewOwner = HandlePool<IImageView>();
        HandlePool<IBufferView> m_bufferViewOwner = HandlePool<IBufferView>();
        HandlePool<IBindGroupLayout> m_bindGroupLayoutsOwner = HandlePool<IBindGroupLayout>();
        HandlePool<IBindGroup> m_bindGroupOwner = HandlePool<IBindGroup>();
        HandlePool<IPipelineLayout> m_pipelineLayoutOwner = HandlePool<IPipelineLayout>();
        HandlePool<IGraphicsPipeline> m_graphicsPipelineOwner = HandlePool<IGraphicsPipeline>();
        HandlePool<IComputePipeline> m_computePipelineOwner = HandlePool<IComputePipeline>();
        HandlePool<ISampler> m_samplerOwner = HandlePool<ISampler>();
    };

    inline uint32_t IContext::GetQueueFamilyIndex(QueueType queueType) const
    {
        switch (queueType)
        {
        case QueueType::Graphics: return m_graphicsQueueFamilyIndex;
        case QueueType::Compute:  return m_computeQueueFamilyIndex;
        case QueueType::Transfer: return m_transferQueueFamilyIndex;
        default:                       RHI_UNREACHABLE(); return UINT32_MAX;
        }
    }

    inline VkQueue IContext::GetQueue(QueueType queueType) const
    {
        switch (queueType)
        {
        case QueueType::Graphics: return m_graphicsQueue;
        case QueueType::Compute:  return m_computeQueue;
        case QueueType::Transfer: return m_transferQueue;
        default:                       RHI_UNREACHABLE(); return VK_NULL_HANDLE;
        }
    }

} // namespace RHI::Vulkan