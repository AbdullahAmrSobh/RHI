#pragma once

#include <RHI/Resources.hpp>
#include <RHI/Swapchain.hpp>
#include "Allocator.hpp"

namespace RHI
{
    struct ImageAttachmentUseInfo;
    struct BufferAttachmentUseInfo;

    class Attachment;
} // namespace RHI

namespace Vulkan
{

    class Context;
    class ShaderModule;
    class ResourcePool;
    class Swapchain;

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

        // Querys the memory requirements of this resource.
        VkMemoryRequirements GetMemoryRequirements(VkDevice device) const;

        void Shutdown(Context* context);
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

        // Querys the memory requirements of this resource.
        VkMemoryRequirements GetMemoryRequirements(VkDevice device) const;
    
        void Shutdown(Context* context);
    };

    struct ImageView : RHI::ImageView
    {
        VkImageView handle;
    
        void Shutdown(Context* context);
    };

    struct BufferView : RHI::BufferView
    {
        VkBufferView handle;
    
        void Shutdown(Context* context);
    };

    struct DescriptorSetLayout
    {
        VkDescriptorSetLayout handle;
    
        void Shutdown(Context* context);
    };

    struct DescriptorSet : RHI::ShaderBindGroup
    {
        VkDescriptorSet handle;
    
        void Shutdown(Context* context);
    };

    struct PipelineLayout
    {
        VkPipelineLayout handle;
    
        void Shutdown(Context* context);
    };

    struct GraphicsPipeline : RHI::GraphicsPipeline
    {
        VkPipeline handle;
        RHI::Handle<PipelineLayout> layout;
    
        void Shutdown(Context* context);
    };

    struct ComputePipeline : RHI::ComputePipeline
    {
        VkPipeline handle;
        RHI::Handle<PipelineLayout> layout;
    
        void Shutdown(Context* context);
    };

    struct Sampler : RHI::Sampler
    {
        VkSampler handle;
    
        void Shutdown(Context* context);
    };

    struct Fence
    {
        VkFence handle;
    
        void Shutdown(Context* context);
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
        Context* m_context;
        VkShaderModule m_shaderModule;
    };

    class ShaderBindGroupAllocator final : public RHI::ShaderBindGroupAllocator
    {
    public:
        ShaderBindGroupAllocator(Context* context)
            : m_context(context)
        {
        }
        ~ShaderBindGroupAllocator();

        VkResult Init();

        std::vector<RHI::Handle<RHI::ShaderBindGroup>> AllocateShaderBindGroups(RHI::TL::Span<const RHI::ShaderBindGroupLayout> layouts) override;

        void Free(RHI::TL::Span<RHI::Handle<RHI::ShaderBindGroup>> groups) override;

        void Update(RHI::Handle<RHI::ShaderBindGroup> group, const RHI::ShaderBindGroupData& data) override;

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

        RHI::Result<RHI::Handle<RHI::Image>> Allocate(const RHI::ImageCreateInfo& createInfo) override;
        RHI::Result<RHI::Handle<RHI::Buffer>> Allocate(const RHI::BufferCreateInfo& createInfo) override;

        void Free(RHI::Handle<RHI::Image> image) override;
        void Free(RHI::Handle<RHI::Buffer> buffer) override;

        size_t GetSize(RHI::Handle<RHI::Image> image) const override;
        size_t GetSize(RHI::Handle<RHI::Buffer> buffer) const override;

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

        RHI::ResultCode Present() override;

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

    class ResourceManager final
    {
    public:
        ResourceManager(Context* context);
        ~ResourceManager();

        RHI::Result<RHI::Handle<Image>> CreateImage(const VmaAllocationCreateInfo allocationInfo, const RHI::ImageCreateInfo& createInfo, ResourcePool* parentPool = nullptr, bool isTransientResource = false);

        RHI::Result<RHI::Handle<Buffer>> CreateBuffer(const VmaAllocationCreateInfo allocationInfo, const RHI::BufferCreateInfo& createInfo, ResourcePool* parentPool = nullptr, bool isTransientResource = false);

        RHI::Result<RHI::Handle<ImageView>> CreateImageView(RHI::Handle<Image> image, const RHI::ImageAttachmentUseInfo& useInfo);

        RHI::Result<RHI::Handle<BufferView>> CreateBufferView(RHI::Handle<Buffer> buffer, const RHI::BufferAttachmentUseInfo& useInfo);

        RHI::Result<RHI::Handle<DescriptorSetLayout>> CreateDescriptorSetLayout(const RHI::ShaderBindGroupLayout& layout);

        RHI::Result<RHI::Handle<DescriptorSet>> CreateDescriptorSet(VkDescriptorPool pool, RHI::Handle<DescriptorSetLayout> descriptorSetLayout);

        RHI::Result<RHI::Handle<PipelineLayout>> CreatePipelineLayout(RHI::TL::Span<const RHI::ShaderBindGroupLayout> shaderBindGroupLayouts);

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

        std::unordered_map<uint64_t, RHI::Handle<DescriptorSetLayout>> m_descriptorSetLayoutCache;

        std::unordered_map<uint64_t, RHI::Handle<PipelineLayout>> m_pipelineLayoutCache;
    };

}; // namespace Vulkan