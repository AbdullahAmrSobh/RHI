#pragma once

#include <RHI/Common/Result.hpp>
#include <RHI/Resources.hpp>

#include <vk_mem_alloc.h>

#include <unordered_set>

namespace RHI::Vulkan
{
    struct IBindGroup;
    struct IBindGroupLayout;

    class IContext;
    class IResourcePool;
    class ISwapchain;

    struct Allocation
    {
        VmaAllocation handle;
        VmaAllocationInfo info;

        size_t offset;

        VmaVirtualBlock virtualBlock;
        VmaVirtualAllocation virtualHandle;
    };

    struct DescriptorPool
    {
        VkDescriptorPool descriptorPool;
        uint32_t referenceCount;
    };

    class BindGroupAllocator
    {
    public:
        BindGroupAllocator(VkDevice device)
            : m_device(device)
        {
        }

        ResultCode InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout);
        void FreePool(Handle<DescriptorPool> handle);

    private:
        std::pair<Handle<DescriptorPool>, DescriptorPool> CreateDescriptorPool();
        VkDescriptorSet AllocateDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout layout);

    public:
        VkDevice m_device;
        HandlePool<DescriptorPool> m_descriptorPoolOwner;
        std::unordered_set<Handle<DescriptorPool>> m_descriptorPools;
    };

    struct IImage : Image
    {
        Allocation allocation; // allocation backing this resource.
        IResourcePool* pool;   // Pointer to the pool this resource is created from.
        VkImage handle;        // Handle to valid VkImage resource (Might not be backed by an allocation).
        VkFormat format;       // Image pixel Format
        VkImageType imageType; // Image dimensions
        ISwapchain* swapchain; // pointer to swapchain (if this image is backed by swapchain).

        ResultCode Init(IContext* context, const ImageCreateInfo& createInfo, bool isTransient = false);
        void Shutdown(IContext* context);

        VkMemoryRequirements GetMemoryRequirements(VkDevice device) const;
    };

    struct IBuffer : Buffer
    {
        Allocation allocation; // allocation backing this resource.
        IResourcePool* pool;   // Pointer to the pool this resource is created from.
        VkBuffer handle;       // Handle to valid VkImage resource (Might not be backed by an allocation).

        ResultCode Init(IContext* context, const BufferCreateInfo& createInfo, bool isTransient = false);
        void Shutdown(IContext* context);

        VkMemoryRequirements GetMemoryRequirements(VkDevice device) const;
    };

    struct IImageView : ImageView
    {
        VkImageView handle;

        ResultCode Init(IContext* context, const ImageViewCreateInfo& useInfo);
        void Shutdown(IContext* context);
    };

    struct IBufferView : BufferView
    {
        VkBufferView handle;

        ResultCode Init(IContext* context, const BufferViewCreateInfo& useInfo);
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
        VkDescriptorSet descriptorSet;
        Handle<DescriptorPool> poolHandle;

        ResultCode Init(IContext* context, Handle<BindGroupLayout> layout);
        void Shutdown(IContext* context);

        void Write(IContext* context, BindGroupData data);
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

        VkResult Init(TL::Span<const uint8_t> shaderBlob);

    public:
        IContext* m_context;
        VkShaderModule m_shaderModule;
    };

    class IResourcePool final : public ResourcePool
    {
    public:
        IResourcePool(IContext* context)
            : m_context(context)
        {
        }

        ~IResourcePool();

        VkResult Init(const ResourcePoolCreateInfo& createInfo);

    public:
        IContext* m_context;
        VmaPool m_pool;
        ResourcePoolCreateInfo m_poolInfo;
    };

    class IStagingBuffer final : public StagingBuffer
    {
    public:
        IStagingBuffer(IContext* context);
        ~IStagingBuffer();

        VkResult Init();

        StagingBuffer::TempBuffer Allocate(size_t newSize) override;
        void Free(TempBuffer mappedBuffer) override;
        void Flush() override;

    private:
        IContext* m_context;
    };

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
        FenceState GetState() override;

        // This should only be called when passing the fence to a vulkan siganl command
        VkFence UseFence();

    private:
        IContext* m_context;
        VkFence m_fence;
        FenceState m_state;
    };
}; // namespace RHI::Vulkan