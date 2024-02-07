#pragma once

#include <RHI/Resources.hpp>

#include <vk_mem_alloc.h>

namespace RHI
{
    struct Attachment;
} // namespace RHI

namespace Vulkan
{
    namespace TL = ::RHI::TL;

    class IContext;
    class IShaderModule;
    class IImagePool;
    class IBufferPool;
    class ISwapchain;

    struct Allocation
    {
        VmaAllocation handle;
        VmaAllocationInfo info;

        size_t offset;

        VmaVirtualBlock virtualBlock;
        VmaVirtualAllocation virtualHandle;
    };

    struct IImage : RHI::Image
    {
        // allocation backing this resource.
        Allocation allocation;

        // Pointer to the pool this resource is created from.
        IImagePool* pool;

        // Handle to valid VkImage resource (Might not be backed by an allocation).
        VkImage handle;

        // Image pixel Format
        VkFormat format;

        // Image dimensions
        VkImageType imageType;

        // pointer to swapchain (if this image is backed by swapchain).
        ISwapchain* swapchain;

        RHI::ResultCode Init(IContext* context, const VmaAllocationCreateInfo allocationInfo, const RHI::ImageCreateInfo& createInfo, IImagePool* parentPool, bool isTransientResource);
        void Shutdown(IContext* context);
        VkMemoryRequirements GetMemoryRequirements(VkDevice device) const;
    };

    struct IBuffer : RHI::Buffer
    {
        // allocation backing this resource.
        Allocation allocation;

        // Pointer to the pool this resource is created from.
        IBufferPool* pool;

        // Handle to valid VkImage resource (Might not be backed by an allocation).
        VkBuffer handle;

        RHI::ResultCode Init(IContext* context, const VmaAllocationCreateInfo allocationInfo, const RHI::BufferCreateInfo& createInfo, IBufferPool* parentPool, bool isTransientResource);
        void Shutdown(IContext* context);
        VkMemoryRequirements GetMemoryRequirements(VkDevice device) const;
    };

    struct IImageView : RHI::ImageView
    {
        VkImageView handle;

        RHI::ResultCode Init(IContext* context, RHI::Handle<IImage> imageHandle, const RHI::ImageViewCreateInfo& useInfo);
        void Shutdown(IContext* context);
    };

    struct IBufferView : RHI::BufferView
    {
        VkBufferView handle;

        RHI::ResultCode Init(IContext* context, RHI::Handle<IBuffer> bufferHandle, const RHI::BufferViewCreateInfo& useInfo);
        void Shutdown(IContext* context);
    };

    struct IBindGroupLayout : RHI::BindGroupLayout
    {
        VkDescriptorSetLayout handle;

        RHI::ResultCode Init(IContext* context, const RHI::BindGroupLayoutCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };

    struct IBindGroup : RHI::BindGroup
    {
        VkDescriptorSet handle;
        VkDescriptorPool pool;

        RHI::ResultCode Init(IContext* context, VkDescriptorSetLayout layout, VkDescriptorPool pool);
        void Shutdown(IContext* context);
    };

    struct IPipelineLayout : RHI::PipelineLayout
    {
        VkPipelineLayout handle;

        RHI::ResultCode Init(IContext* context, const RHI::PipelineLayoutCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };

    struct IGraphicsPipeline : RHI::GraphicsPipeline
    {
        VkPipeline handle;
        VkPipelineLayout layout;

        RHI::ResultCode Init(IContext* context, const RHI::GraphicsPipelineCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };

    struct IComputePipeline : RHI::ComputePipeline
    {
        VkPipeline handle;
        VkPipelineLayout layout;

        RHI::ResultCode Init(IContext* context, const RHI::ComputePipelineCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };

    struct ISampler : RHI::Sampler
    {
        VkSampler handle;

        RHI::ResultCode Init(IContext* context, const RHI::SamplerCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };

    class IShaderModule final : public RHI::ShaderModule
    {
    public:
        IShaderModule(IContext* context)
            : m_context(context)
        {
        }

        ~IShaderModule();

        VkResult Init(const RHI::ShaderModuleCreateInfo& createInfo);

    public:
        IContext* m_context;
        VkShaderModule m_shaderModule;
    };

    class IBindGroupAllocator final : public RHI::BindGroupAllocator
    {
    public:
        IBindGroupAllocator(IContext* context)
            : m_context(context)
        {
        }

        ~IBindGroupAllocator();

        VkResult Init();

        std::vector<RHI::Handle<RHI::BindGroup>> AllocateBindGroups(TL::Span<RHI::Handle<RHI::BindGroupLayout>> bindGroupLayouts) override;
        void Free(TL::Span<RHI::Handle<RHI::BindGroup>> bindGroup) override;
        void Update(RHI::Handle<RHI::BindGroup> bindGroup, const RHI::BindGroupData& data) override;

        IContext* m_context;
        std::vector<VkDescriptorPool> m_descriptorPools;
    };

    class IBufferPool final : public RHI::BufferPool
    {
    public:
        IBufferPool(IContext* context)
            : m_context(context)
        {
        }

        ~IBufferPool();

        VkResult Init(const RHI::PoolCreateInfo& createInfo);

        RHI::Result<RHI::Handle<RHI::Buffer>> Allocate(const RHI::BufferCreateInfo& createInfo) override;
        void FreeBuffer(RHI::Handle<RHI::Buffer> handle) override;
        size_t GetSize(RHI::Handle<RHI::Buffer> handle) const override;
        RHI::DeviceMemoryPtr MapBuffer(RHI::Handle<RHI::Buffer> handle) override;
        void UnmapBuffer(RHI::Handle<RHI::Buffer> handle) override;

    public:
        IContext* m_context;
        VmaPool m_pool;
        RHI::PoolCreateInfo m_poolInfo;
    };

    class IImagePool final : public RHI::ImagePool
    {
    public:
        IImagePool(IContext* context)
            : m_context(context)
        {
        }

        ~IImagePool();

        VkResult Init(const RHI::PoolCreateInfo& createInfo);

        RHI::Result<RHI::Handle<RHI::Image>> Allocate(const RHI::ImageCreateInfo& createInfo) override;
        void FreeImage(RHI::Handle<RHI::Image> handle) override;
        size_t GetSize(RHI::Handle<RHI::Image> handle) const override;

    public:
        IContext* m_context;
        VmaPool m_pool;
        RHI::PoolCreateInfo m_poolInfo;
    };

    /// @brief Fence object used to preform CPU-GPU sync
    class IFence final : public RHI::Fence
    {
    public:
        IFence(IContext* context)
            : m_context(context)
        {
        }

        ~IFence();

        VkResult Init();

        void Reset() override;
        bool WaitInternal(uint64_t timeout) override;
        State GetState() override;

        // This should only be called when passing the fence to a vulkan siganl command
        VkFence UseFence();

    private:
        IContext* m_context;
        VkFence m_fence;
        State m_state;
    };

    class ISwapchain final : public RHI::Swapchain
    {
    public:
        ISwapchain(IContext* context);
        ~ISwapchain();

        VkResult Init(const RHI::SwapchainCreateInfo& createInfo);

        RHI::ResultCode Recreate(RHI::ImageSize2D newSize, uint32_t imageCount, RHI::SwapchainPresentMode presentMode) override;
        RHI::ResultCode Present() override;

    private:
        VkPresentModeKHR ConvertPresentMode(RHI::SwapchainPresentMode presentMode);
        VkResult InitSurface(const RHI::SwapchainCreateInfo& createInfo);
        VkResult InitSwapchain();

    public:
        friend class IFrameScheduler;

        IContext* m_context;

        struct Semaphores
        {
            VkSemaphore imageAcquired;
            VkSemaphore imageRenderComplete;
        } m_semaphores;

        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR m_surface;

        VkResult m_lastPresentResult;
        VkCompositeAlphaFlagBitsKHR m_compositeAlpha;
        VkSurfaceFormatKHR m_surfaceFormat;
    };

}; // namespace Vulkan