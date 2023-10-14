#pragma once

#include <RHI/Pass.hpp>
#include <RHI/Resources.hpp>
#include <RHI/Swapchain.hpp>

#include <vk_mem_alloc.h>

namespace Vulkan
{

    class Context;
    class ShaderModule;
    class ResourcePool;
    class Swapchain;

    struct Image final : public RHI::Image
    {
        VkImage handle;
        VmaAllocationInfo allocationInfo;
        VmaAllocation allocationHandle;
        Swapchain* swapchain;
        VkFormat format;
        VkImageType type;
        ResourcePool* pool;
    };

    struct Buffer final : public RHI::Buffer
    {
        VkBuffer handle;
        VmaAllocationInfo allocationInfo;
        VmaAllocation allocationHandle;
        ResourcePool* pool;
    };

    struct ImageView : RHI::ImageView
    {
        VkImageView handle;
    };

    struct BufferView : RHI::BufferView
    {
        VkBufferView handle;
    };

    struct DescriptorSetLayout
    {
        VkDescriptorSetLayout handle;
    };

    struct DescriptorSet : RHI::ShaderBindGroup
    {
        VkDescriptorSet handle;
    };

    struct PipelineLayout
    {
        VkPipelineLayout handle;
    };

    struct GraphicsPipeline
    {
        VkPipeline handle;
        RHI::Handle<PipelineLayout> layout;
    };

    struct ComputePipeline
    {
        VkPipeline handle;
        RHI::Handle<PipelineLayout> layout;
    };

    struct Sampler
    {
        VkSampler handle;
    };

    struct Fence
    {
        VkFence handle;
    };

    VkSampleCountFlagBits ConvertToVkSampleCount(RHI::SampleCount sampleCount);

    VkSampleCountFlags ConvertToVkSampleCountFlags(RHI::Flags<RHI::SampleCount> sampleCountFlags);

    VkImageUsageFlagBits ConvertToVkImageUsage(RHI::ImageUsage imageUsage);

    VkImageUsageFlags ConvertToVkImageUsageFlags(RHI::Flags<RHI::ImageUsage> imageUsageFlags);

    VkImageType ConvertToVkImageType(RHI::ImageType imageType);

    VkImageAspectFlagBits ConvertToVkImageAspect(RHI::Flags<RHI::ImageAspect> imageAspect);

    VkImageAspectFlags ConvertToVkImageAspect(RHI::ImageAspect imageAspect);

    VkComponentSwizzle ConvertToVkComponentSwizzle(RHI::ComponentSwizzle componentSwizzle);

    VkBufferUsageFlagBits ConvertToVkBufferUsage(RHI::BufferUsage bufferUsage);

    VkBufferUsageFlags ConvertToVkBufferUsageFlags(RHI::Flags<RHI::BufferUsage> bufferUsageFlags);

    VkShaderStageFlagBits ConvertToVkShaderStage(RHI::ShaderStage shaderStage);

    VkShaderStageFlags ConvertToVkShaderStage(RHI::Flags<RHI::ShaderStage> shaderStageFlags);

    VkDescriptorType ConvertToVkDescriptorType(RHI::ShaderBindingType bindingType);

    VkAccessFlags ConvertToVkAccessFlags(RHI::ShaderBindingAccess bindingAccess);

    VkVertexInputRate ConvertToVkVertexInputRate(RHI::PipelineVertexInputRate inputRate);

    VkCullModeFlags ConvertToVkCullModeFlags(RHI::PipelineRasterizerStateCullMode cullMode);

    VkPolygonMode ConvertToVkPolygonMode(RHI::PipelineRasterizerStateFillMode fillMode);

    VkPrimitiveTopology ConvertToVkPrimitiveTopology(RHI::PipelineTopologyMode topologyMode);

    VkFrontFace ConvertToVkFrontFace(RHI::PipelineRasterizerStateFrontFace frontFace);

    VkCompareOp ConvertToVkCompareOp(RHI::CompareOperator compareOperator);

    VkFilter ConvertToVkFilter(RHI::SamplerFilter samplerFilter);

    VkSamplerAddressMode ConvertToVkSamplerAddressMode(RHI::SamplerAddressMode addressMode);

    VkCompareOp ConvertToVkCompareOp(RHI::SamplerCompareOperation compareOperation);

    VkBlendFactor ConvertToVkBlendFactor(RHI::BlendFactor blendFactor);

    VkBlendOp ConvertToVkBlendOp(RHI::BlendEquation blendEquation);

    class ShaderModule final : public RHI::ShaderModule
    {
    public:
        using RHI::ShaderModule::ShaderModule;
        ~ShaderModule();

        VkResult Init(const RHI::ShaderModuleCreateInfo& createInfo);

    public:
        VkShaderModule m_shaderModule;
    };

    class ShaderBindGroupAllocator final : public RHI::ShaderBindGroupAllocator
    {
    public:
        using RHI::ShaderBindGroupAllocator::ShaderBindGroupAllocator;
        ~ShaderBindGroupAllocator();

        VkResult Init();

        std::vector<RHI::Handle<RHI::ShaderBindGroup>> AllocateShaderBindGroups(RHI::TL::Span<const RHI::ShaderBindGroupLayout> layouts) override;

        void Free(RHI::TL::Span<RHI::Handle<RHI::ShaderBindGroup>> groups) override;

        void Update(RHI::Handle<RHI::ShaderBindGroup> group, const RHI::ShaderBindGroupData& data) override;
    };

    class ResourcePool final : public RHI::ResourcePool
    {
    public:
        using RHI::ResourcePool::ResourcePool;
        ~ResourcePool();

        VkResult Init(const RHI::ResourcePoolCreateInfo& createInfo);

        RHI::Result<RHI::Handle<RHI::Image>> Allocate(const RHI::ImageCreateInfo& createInfo) override;
        RHI::Result<RHI::Handle<RHI::Buffer>> Allocate(const RHI::BufferCreateInfo& createInfo) override;

        void Free(RHI::Handle<RHI::Image> image) override;
        void Free(RHI::Handle<RHI::Buffer> buffer) override;

        size_t GetSize(RHI::Handle<RHI::Image> image) const override;
        size_t GetSize(RHI::Handle<RHI::Buffer> buffer) const override;

    public:
        VmaPool m_pool;

        RHI::ResourcePoolCreateInfo m_poolInfo;
    };

    class Swapchain final : public RHI::Swapchain
    {
    public:
        using RHI::Swapchain::Swapchain;
        ~Swapchain();

        VkResult Init(const RHI::SwapchainCreateInfo& createInfo);

        RHI::ResultCode Resize(uint32_t newWidth, uint32_t newHeight) override;

        RHI::ResultCode SetExclusiveFullScreenMode(bool enable_fullscreen) override;

        RHI::ResultCode Present() override;

    private:
        VkResult CreateNativeSwapchain();

        VkSurfaceFormatKHR GetSurfaceFormat(VkFormat format);

        VkCompositeAlphaFlagBitsKHR GetCompositeAlpha(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        VkPresentModeKHR GetPresentMode();

    public:
        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

        VkSurfaceKHR m_surface = VK_NULL_HANDLE;

        VkResult m_lastPresentResult = VK_ERROR_UNKNOWN;

        bool m_fullscreenMode = false;

        RHI::SwapchainCreateInfo m_swapchainInfo;

        inline static constexpr uint64_t SwapchainAcquireTime = 1000000000;
    };

    class ResourceManager final
    {
    public:
        ResourceManager(Context* context);
        ~ResourceManager();

        RHI::Result<RHI::Handle<Image>> CreateImage(const VmaAllocationCreateInfo allocationInfo, const RHI::ImageCreateInfo& createInfo);

        RHI::Result<RHI::Handle<Buffer>> CreateBuffer(const VmaAllocationCreateInfo allocationInfo, const RHI::BufferCreateInfo& createInfo);

        RHI::Result<RHI::Handle<ImageView>> CreateImageView(RHI::Handle<Image> image, const RHI::ImageAttachmentUseInfo& useInfo);

        RHI::Result<RHI::Handle<BufferView>> CreateBufferView(RHI::Handle<Buffer> buffer, const RHI::BufferAttachmentUseInfo& useInfo);

        RHI::Result<RHI::Handle<DescriptorSetLayout>> CreateDescriptorSetLayout(const RHI::ShaderBindGroupLayout& layout);

        RHI::Result<RHI::Handle<DescriptorSet>> CreateDescriptorSet(RHI::Handle<DescriptorSetLayout> descriptorSetLayout);

        RHI::Result<RHI::Handle<PipelineLayout>> CreatePipelineLayout(RHI::TL::Span<RHI::ShaderBindGroupLayout> shaderBindGroupLayouts);

        RHI::Result<RHI::Handle<GraphicsPipeline>> CreateGraphicsPipeline(const RHI::GraphicsPipelineCreateInfo& createInfo);

        RHI::Result<RHI::Handle<ComputePipeline>> CreateComputePipeline(const RHI::ComputePipelineCreateInfo& createInfo);

        RHI::Result<RHI::Handle<Sampler>> CreateSampler(const RHI::SamplerCreateInfo& createInfo);

        RHI::Result<RHI::Handle<Fence>> CreateFence();

        void FreeImage(RHI::Handle<Image> handle);

        void FreeBuffer(RHI::Handle<Buffer> handle);

        void FreeImageView(RHI::Handle<ImageView> handle);

        void FreeBufferView(RHI::Handle<BufferView> handle);

        void FreeDescriptorSetLayout(RHI::Handle<DescriptorSetLayout> handle);

        void FreeDescriptorSet(RHI::Handle<DescriptorSet> handle);

        void FreePipelineLayout(RHI::Handle<PipelineLayout> handle);

        void FreeGraphicsPipeline(RHI::Handle<GraphicsPipeline> handle);

        void FreeComputePipeline(RHI::Handle<ComputePipeline> handle);

        void FreeSampler(RHI::Handle<Sampler> handle);

        void FreeFence(RHI::Handle<Fence> handle);

        Context* m_context;

        RHI::HandlePool<Image> m_imageOwner;

        RHI::HandlePool<Buffer> m_bufferOwner;

        RHI::HandlePool<ImageView> m_imageViewOwner;

        RHI::HandlePool<BufferView> m_bufferViewOwner;

        RHI::HandlePool<DescriptorSetLayout> m_descriptorSetLayoutOwner;

        RHI::HandlePool<DescriptorSet> m_descriptorSetOwner;

        RHI::HandlePool<PipelineLayout> m_pipelineLayoutOwner;

        RHI::HandlePool<GraphicsPipeline> m_graphicsPipelineOwner;

        RHI::HandlePool<ComputePipeline> m_computePipelineOwner;

        RHI::HandlePool<Sampler> m_samplerOwner;

        RHI::HandlePool<Fence> m_fenceOwner;
    };

}; // namespace Vulkan