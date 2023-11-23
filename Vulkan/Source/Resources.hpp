#pragma once

#include "Allocator.hpp"

#include <RHI/Resources.hpp>
#include <RHI/Swapchain.hpp>

namespace RHI
{
    struct ImageAttachmentUseInfo;
    struct BufferAttachmentUseInfo;

    struct Attachment;
} // namespace RHI

namespace Vulkan
{
    namespace TL = ::RHI::TL;

    class Context;
    class ShaderModule;
    class ResourcePool;
    class Swapchain;

    VkSampleCountFlagBits ConvertSampleCount(RHI::SampleCount sampleCount);

    VkSampleCountFlags ConvertSampleCountFlags(RHI::Flags<RHI::SampleCount> sampleCountFlags);

    VkImageUsageFlagBits ConvertImageUsage(RHI::ImageUsage imageUsage);

    VkImageUsageFlags ConvertImageUsageFlags(RHI::Flags<RHI::ImageUsage> imageUsageFlags);

    VkImageType ConvertImageType(RHI::ImageType imageType);

    VkImageAspectFlagBits ConvertImageAspect(RHI::Flags<RHI::ImageAspect> imageAspect);

    VkImageAspectFlags ConvertImageAspect(RHI::ImageAspect imageAspect);

    VkComponentSwizzle ConvertComponentSwizzle(RHI::ComponentSwizzle componentSwizzle);

    VkBufferUsageFlagBits ConvertBufferUsage(RHI::BufferUsage bufferUsage);

    VkBufferUsageFlags ConvertBufferUsageFlags(RHI::Flags<RHI::BufferUsage> bufferUsageFlags);

    VkShaderStageFlagBits ConvertShaderStage(RHI::ShaderStage shaderStage);

    VkShaderStageFlags ConvertShaderStage(RHI::Flags<RHI::ShaderStage> shaderStageFlags);

    VkDescriptorType ConvertDescriptorType(RHI::ShaderBindingType bindingType);

    VkAccessFlags ConvertAccessFlags(RHI::ShaderBindingAccess bindingAccess);

    VkVertexInputRate ConvertVertexInputRate(RHI::PipelineVertexInputRate inputRate);

    VkCullModeFlags ConvertCullModeFlags(RHI::PipelineRasterizerStateCullMode cullMode);

    VkPolygonMode ConvertPolygonMode(RHI::PipelineRasterizerStateFillMode fillMode);

    VkPrimitiveTopology ConvertPrimitiveTopology(RHI::PipelineTopologyMode topologyMode);

    VkFrontFace ConvertFrontFace(RHI::PipelineRasterizerStateFrontFace frontFace);

    VkCompareOp ConvertCompareOp(RHI::CompareOperator compareOperator);

    VkFilter ConvertFilter(RHI::SamplerFilter samplerFilter);

    VkSamplerAddressMode ConvertSamplerAddressMode(RHI::SamplerAddressMode addressMode);

    VkCompareOp ConvertCompareOp(RHI::SamplerCompareOperation compareOperation);

    VkBlendFactor ConvertBlendFactor(RHI::BlendFactor blendFactor);

    VkBlendOp ConvertBlendOp(RHI::BlendEquation blendEquation);

    VkImageSubresource ConvertSubresource(const RHI::ImageSubresource& subresource);

    VkImageSubresourceLayers ConvertSubresourceLayer(const RHI::ImageSubresource& subresource);

    VkImageSubresourceRange ConvertSubresourceRange(const RHI::ImageSubresource& subresource);

    VkExtent3D ConvertExtent3D(RHI::ImageSize size);

    VkExtent2D ConvertExtent2D(RHI::ImageSize size);

    VkOffset3D ConvertOffset3D(RHI::ImageOffset offset);

    VkOffset3D ConvertOffset2D(RHI::ImageOffset offset);

    struct Image : RHI::Image
    {
        // allocation backing this resource.
        Allocation allocation;

        // Pointer to the pool this resource is created from.
        ResourcePool* pool;

        // Handle to valid VkImage resource (Might not be backed by an allocation).
        VkImage handle;

        // description of the resource.
        VkImageCreateInfo createInfo;

        // pointer to swapchain (if this image is backed by swapchain).
        Swapchain* swapchain;

        RHI::ResultCode      Init(Context* context, const VmaAllocationCreateInfo allocationInfo, const RHI::ImageCreateInfo& createInfo, ResourcePool* parentPool, bool isTransientResource);
        void                 Shutdown(Context* context);
        VkMemoryRequirements GetMemoryRequirements(VkDevice device) const;
    };

    struct Buffer : RHI::Buffer
    {
        // allocation backing this resource.
        Allocation allocation;

        // Pointer to the pool this resource is created from.
        ResourcePool* pool;

        // Handle to valid VkImage resource (Might not be backed by an allocation).
        VkBuffer handle;

        // description of the resource.
        VkBufferCreateInfo createInfo;

        RHI::ResultCode      Init(Context* context, const VmaAllocationCreateInfo allocationInfo, const RHI::BufferCreateInfo& createInfo, ResourcePool* parentPool, bool isTransientResource);
        void                 Shutdown(Context* context);
        VkMemoryRequirements GetMemoryRequirements(VkDevice device) const;
    };

    struct ImageView : RHI::ImageView
    {
        VkImageView handle;

        RHI::ResultCode Init(Context* context, RHI::Handle<Image> imageHandle, const RHI::ImageAttachmentUseInfo& useInfo);
        void            Shutdown(Context* context);
    };

    struct BufferView : RHI::BufferView
    {
        VkBufferView handle;

        RHI::ResultCode Init(Context* context, RHI::Handle<Buffer> bufferHandle, const RHI::BufferAttachmentUseInfo& useInfo);
        void            Shutdown(Context* context);
    };

    struct BindGroupLayout : RHI::BindGroupLayout
    {
        VkDescriptorSetLayout handle;

        RHI::ResultCode Init(Context* context, const RHI::BindGroupLayoutCreateInfo& createInfo);
        void            Shutdown(Context* context);
    };

    struct BindGroup : RHI::BindGroup
    {
        VkDescriptorSet  handle;
        VkDescriptorPool pool;

        RHI::ResultCode Init(Context* context, VkDescriptorSetLayout layout, VkDescriptorPool pool);
        void            Shutdown(Context* context);
    };

    struct PipelineLayout : RHI::PipelineLayout
    {
        VkPipelineLayout handle;

        RHI::ResultCode Init(Context* context, const RHI::PipelineLayoutCreateInfo& createInfo);
        void            Shutdown(Context* context);
    };

    struct GraphicsPipeline : RHI::GraphicsPipeline
    {
        VkPipeline       handle;
        VkPipelineLayout layout;

        RHI::ResultCode Init(Context* context, const RHI::GraphicsPipelineCreateInfo& createInfo);
        void            Shutdown(Context* context);
    };

    struct ComputePipeline : RHI::ComputePipeline
    {
        VkPipeline       handle;
        VkPipelineLayout layout;

        RHI::ResultCode Init(Context* context, const RHI::ComputePipelineCreateInfo& createInfo);
        void            Shutdown(Context* context);
    };

    struct Sampler : RHI::Sampler
    {
        VkSampler handle;

        RHI::ResultCode Init(Context* context, const RHI::SamplerCreateInfo& createInfo);
        void            Shutdown(Context* context);
    };

    class ShaderModule final : public RHI::ShaderModule
    {
    public:
        ShaderModule(Context* context)
            : m_context(context)
        {
        }

        ~ShaderModule();

        VkResult Init(const RHI::ShaderModuleCreateInfo& createInfo);

    public:
        Context*       m_context;
        VkShaderModule m_shaderModule;
    };

    class BindGroupAllocator final : public RHI::BindGroupAllocator
    {
    public:
        BindGroupAllocator(Context* context)
            : m_context(context)
        {
        }

        ~BindGroupAllocator();

        VkResult Init();

        std::vector<RHI::Handle<RHI::BindGroup>> AllocateBindGroups(TL::Span<RHI::Handle<RHI::BindGroupLayout>> bindGroupLayouts) override;
        void                                     Free(TL::Span<RHI::Handle<RHI::BindGroup>> bindGroup) override;
        void                                     Update(RHI::Handle<RHI::BindGroup> bindGroup, const RHI::BindGroupData& data) override;

        Context* m_context;

        std::vector<VkDescriptorPool> m_descriptorPools;
    };

    class ResourcePool final : public RHI::ResourcePool
    {
    public:
        ResourcePool(Context* context)
            : m_context(context)
        {
        }

        ~ResourcePool();

        VkResult Init(const RHI::ResourcePoolCreateInfo& createInfo);

        RHI::Result<RHI::Handle<RHI::Image>>  Allocate(const RHI::ImageCreateInfo& createInfo) override;
        RHI::Result<RHI::Handle<RHI::Buffer>> Allocate(const RHI::BufferCreateInfo& createInfo) override;
        void                                  Free(RHI::Handle<RHI::Image> image) override;
        void                                  Free(RHI::Handle<RHI::Buffer> buffer) override;
        size_t                                GetSize(RHI::Handle<RHI::Image> image) const override;
        size_t                                GetSize(RHI::Handle<RHI::Buffer> buffer) const override;

    public:
        Context* m_context;

        VmaPool m_pool;

        RHI::ResourcePoolCreateInfo m_poolInfo;
    };

    class Swapchain final : public RHI::Swapchain
    {
    public:
        Swapchain(Context* context)
            : m_context(context)
        {
        }

        ~Swapchain();

        VkResult Init(const RHI::SwapchainCreateInfo& createInfo);

        RHI::ResultCode Resize(uint32_t newWidth, uint32_t newHeight) override;
        RHI::ResultCode Present(RHI::Pass& pass) override;

    private:
        VkResult CreateNativeSwapchain();

        VkSurfaceFormatKHR GetSurfaceFormat(VkFormat format);

        VkCompositeAlphaFlagBitsKHR GetCompositeAlpha(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        VkPresentModeKHR GetPresentMode();

    public:
        Context* m_context;

        VkSemaphore m_imageReadySemaphore = VK_NULL_HANDLE;

        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

        VkSurfaceKHR m_surface = VK_NULL_HANDLE;

        VkResult m_lastPresentResult = VK_ERROR_UNKNOWN;

        RHI::SwapchainCreateInfo m_swapchainInfo;
    };

}; // namespace Vulkan