#pragma once

#include <RHI/Common/Result.hpp>
#include <RHI/Resources.hpp>

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    struct IBindGroup;
    struct IBindGroupLayout;

    class IContext;
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
        BindGroupAllocator(IContext* context);
        ~BindGroupAllocator() = default;

        ResultCode Init();
        void Shutdown();

        ResultCode InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout, uint32_t bindlessElementsCount);
        void ShutdownBindGroup(IBindGroup* bindGroup);

    public:
        IContext* m_context;
        VkDescriptorPool m_descriptorPool;
    };

    struct IImage : Image
    {
        Allocation allocation;
        VkImage handle;

        VkImageCreateFlags flags;
        VkImageType imageType;
        VkFormat format;
        VkExtent3D extent;
        uint32_t mipLevels;
        uint32_t arrayLayers;
        VkSampleCountFlagBits samples;
        VkImageUsageFlags usage;

        TL::Flags<ImageAspect> availableAspects;

        ResultCode Init(IContext* context, const ImageCreateInfo& createInfo);
        ResultCode Init(IContext* context, VkImage image, const VkSwapchainCreateInfoKHR& swapchainCreateInfo);
        void Shutdown(IContext* context);

        VkMemoryRequirements GetMemoryRequirements(IContext* context) const;
    };


    struct IBuffer : Buffer
    {
        Allocation allocation;
        VkBuffer handle;

        VkImageCreateFlags flags;
        size_t size;
        VkBufferUsageFlags usage;

        ResultCode Init(IContext* context, const BufferCreateInfo& createInfo);
        void Shutdown(IContext* context);

        VkMemoryRequirements GetMemoryRequirements(IContext* context) const;
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

        ResultCode Init(IContext* context, Handle<BindGroupLayout> layout, uint32_t bindlessElementsCount);
        void Shutdown(IContext* context);

        void Write(IContext* context, TL::Span<const BindGroupUpdateInfo> bindings);
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
        {
            m_context = (Context*)context;
        }

        ~IShaderModule();

        ResultCode Init(TL::Span<const uint32_t> shaderBlob);

    public:
        VkShaderModule m_shaderModule;
    };

    class IFence final : public Fence
    {
    public:
        IFence(IContext* context)
            : m_context(context)
        {
        }

        ~IFence();

        ResultCode Init();

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