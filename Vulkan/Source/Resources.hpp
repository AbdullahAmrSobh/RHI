#pragma once

#include <RHI/Common/Result.hpp>
#include <RHI/Resources.hpp>

#include <vk_mem_alloc.h>

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

    class BindGroupAllocator
    {
    public:
        BindGroupAllocator(VkDevice device);
        ~BindGroupAllocator() = default;

        void Shutdown();

        ResultCode InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout);
        void ShutdownBindGroup(IBindGroup* bindGroup);

    public:
        VkDevice m_device;
        VkDescriptorPool m_descriptorPool;
    };

    struct IImage : Image
    {
        // TODO: break down to several parallel structures
        Allocation allocation; // allocation backing this resource.
        IResourcePool* pool;   // Pointer to the pool this resource is created from.
        VkImage handle;        // Handle to valid VkImage resource (Might not be backed by an allocation).
        VkFormat format;       // Image pixel Format
        VkImageType imageType; // Image dimensions
        ImageSize3D size;      // Image dimensions

        VkSemaphore waitSemaphore;   // wait semaphore: if the content of this resource is being written by the framescheduler (wait on this semaphore)
        VkSemaphore signalSemaphore; // signal semaphore: if the content of this resource is being read by the frameschduler (signal this semaphore)

        uint32_t queueFamilyIndex;
        VkImageLayout initalLayout;

        bool isTransient; // if this resource is

        ResultCode Init(IContext* context, const ImageCreateInfo& createInfo, bool isTransient = false);
        void Shutdown(IContext* context);

        VkMemoryRequirements GetMemoryRequirements(VkDevice device) const;
    };

    struct IBuffer : Buffer
    {
        // TODO: break down to several parallel structures
        Allocation allocation; // allocation backing this resource.
        IResourcePool* pool;   // Pointer to the pool this resource is created from.
        VkBuffer handle;       // Handle to valid VkImage resource (Might not be backed by an allocation).

        VkSemaphore waitSemaphore;   // wait semaphore: if the content of this resource is being written by the framescheduler (wait on this semaphore)
        VkSemaphore signalSemaphore; // signal semaphore: if the content of this resource is being read by the frameschduler (signal this semaphore)

        uint32_t queueFamilyIndex;

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
        BindGroupLayoutCreateInfo layoutInfo;
        VkDescriptorSetLayout handle;

        ResultCode Init(IContext* context, const BindGroupLayoutCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };

    struct IBindGroup : BindGroup
    {
        VkDescriptorSet descriptorSet;
        Handle<BindGroupLayout> layout;

        ResultCode Init(IContext* context, Handle<BindGroupLayout> layout);
        void Shutdown(IContext* context);

        void Write(IContext* context, TL::Span<const ResourceBinding> bindings);
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