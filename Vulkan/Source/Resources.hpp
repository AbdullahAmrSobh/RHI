#pragma once

#include <RHI/Resources.hpp>

#include <vk_mem_alloc.h>

namespace RHI
{
    struct Attachment;
} // namespace RHI

namespace RHI::Vulkan
{

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

    struct IImage : Image
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

        ResultCode Init(IContext* context, const VmaAllocationCreateInfo allocationInfo, const ImageCreateInfo& createInfo, IImagePool* parentPool, bool isTransientResource);
        void Shutdown(IContext* context);
        VkMemoryRequirements GetMemoryRequirements(VkDevice device) const;
    };

    struct IBuffer : Buffer
    {
        // allocation backing this resource.
        Allocation allocation;

        // Pointer to the pool this resource is created from.
        IBufferPool* pool;

        // Handle to valid VkImage resource (Might not be backed by an allocation).
        VkBuffer handle;

        ResultCode Init(IContext* context, const VmaAllocationCreateInfo allocationInfo, const BufferCreateInfo& createInfo, IBufferPool* parentPool, bool isTransientResource);
        void Shutdown(IContext* context);
        VkMemoryRequirements GetMemoryRequirements(VkDevice device) const;
    };

    struct IImageView : ImageView
    {
        VkImageView handle;

        ResultCode Init(IContext* context, Handle<IImage> imageHandle, const ImageViewCreateInfo& useInfo);
        void Shutdown(IContext* context);
    };

    struct IBufferView : BufferView
    {
        VkBufferView handle;

        ResultCode Init(IContext* context, Handle<IBuffer> bufferHandle, const BufferViewCreateInfo& useInfo);
        void Shutdown(IContext* context);
    };

    struct IBindGroupLayout : BindGroupLayout
    {
        VkDescriptorSetLayout handle;

        ResultCode Init(IContext* context, const BindGroupLayoutCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };

    struct IBindGroup : BindGroup
    {
        VkDescriptorSet handle;
        VkDescriptorPool pool;

        ResultCode Init(IContext* context, VkDescriptorSetLayout layout, VkDescriptorPool pool);
        void Shutdown(IContext* context);
    };

    struct IPipelineLayout : PipelineLayout
    {
        VkPipelineLayout handle;

        ResultCode Init(IContext* context, const PipelineLayoutCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };

    struct IGraphicsPipeline : GraphicsPipeline
    {
        VkPipeline handle;
        VkPipelineLayout layout;

        ResultCode Init(IContext* context, const GraphicsPipelineCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };

    struct IComputePipeline : ComputePipeline
    {
        VkPipeline handle;
        VkPipelineLayout layout;

        ResultCode Init(IContext* context, const ComputePipelineCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };

    struct ISampler : Sampler
    {
        VkSampler handle;

        ResultCode Init(IContext* context, const SamplerCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };

    class IShaderModule final : public ShaderModule
    {
    public:
        IShaderModule(IContext* context)
            : m_context(context)
        {
        }

        ~IShaderModule();

        VkResult Init(const ShaderModuleCreateInfo& createInfo);

    public:
        IContext* m_context;
        VkShaderModule m_shaderModule;
    };

    class IBindGroupAllocator final : public BindGroupAllocator
    {
    public:
        IBindGroupAllocator(IContext* context)
            : m_context(context)
        {
        }

        ~IBindGroupAllocator();

        VkResult Init();

        std::vector<Handle<BindGroup>> AllocateBindGroups(TL::Span<Handle<BindGroupLayout>> bindGroupLayouts) override;
        void Free(TL::Span<Handle<BindGroup>> bindGroup) override;
        void Update(Handle<BindGroup> bindGroup, const BindGroupData& data) override;

        IContext* m_context;
        std::vector<VkDescriptorPool> m_descriptorPools;
    };

    class IBufferPool final : public BufferPool
    {
    public:
        IBufferPool(IContext* context)
            : m_context(context)
        {
        }

        ~IBufferPool();

        VkResult Init(const PoolCreateInfo& createInfo);

        Result<Handle<Buffer>> Allocate(const BufferCreateInfo& createInfo) override;
        void FreeBuffer(Handle<Buffer> handle) override;
        size_t GetSize(Handle<Buffer> handle) const override;
        DeviceMemoryPtr MapBuffer(Handle<Buffer> handle) override;
        void UnmapBuffer(Handle<Buffer> handle) override;

    public:
        IContext* m_context;
        VmaPool m_pool;
        PoolCreateInfo m_poolInfo;
    };

    class IImagePool final : public ImagePool
    {
    public:
        IImagePool(IContext* context)
            : m_context(context)
        {
        }

        ~IImagePool();

        VkResult Init(const PoolCreateInfo& createInfo);

        Result<Handle<Image>> Allocate(const ImageCreateInfo& createInfo) override;
        void FreeImage(Handle<Image> handle) override;
        size_t GetSize(Handle<Image> handle) const override;

    public:
        IContext* m_context;
        VmaPool m_pool;
        PoolCreateInfo m_poolInfo;
    };

    /// @brief Fence object used to preform CPU-GPU sync
    class IFence final : public Fence
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

    class ISwapchain final : public Swapchain
    {
    public:
        ISwapchain(IContext* context);
        ~ISwapchain();

        VkResult Init(const SwapchainCreateInfo& createInfo);

        ResultCode Recreate(ImageSize2D newSize, uint32_t imageCount, SwapchainPresentMode presentMode) override;
        ResultCode Present() override;

    private:
        VkPresentModeKHR ConvertPresentMode(SwapchainPresentMode presentMode);
        VkResult InitSurface(const SwapchainCreateInfo& createInfo);
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

}; // namespace RHI::Vulkan