#pragma once

#include <RHI/Resources.hpp>

#include <vk_mem_alloc.h>

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

    enum class AllocationType
    {
        Default,   // allocation is made from blocks managed internally
        Dedicated, // allocation creates its own dedicated block
        Aliasing,  // multiple resources may share the same allocation
    };

    struct VirtualAllocation
    {
        VmaVirtualBlock blockHandle;
        VmaVirtualAllocation handle;
        size_t offset;
    };

    struct Allocation
    {
        VmaAllocation handle;
        VmaAllocationInfo info;
        AllocationType type;
        VirtualAllocation virtualAllocation;
    };

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

        RHI::ResultCode Init(Context* context, const VmaAllocationCreateInfo allocationInfo, const RHI::ImageCreateInfo& createInfo, ResourcePool* parentPool, bool isTransientResource);
        void Shutdown(Context* context);
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

        RHI::ResultCode Init(Context* context, const VmaAllocationCreateInfo allocationInfo, const RHI::BufferCreateInfo& createInfo, ResourcePool* parentPool, bool isTransientResource);
        void Shutdown(Context* context);
        VkMemoryRequirements GetMemoryRequirements(VkDevice device) const;
    };

    struct ImageView : RHI::ImageView
    {
        VkImageView handle;

        RHI::ResultCode Init(Context* context, RHI::Handle<Image> imageHandle, const RHI::ImageAttachmentUseInfo& useInfo);
        void Shutdown(Context* context);
    };

    struct BufferView : RHI::BufferView
    {
        VkBufferView handle;

        RHI::ResultCode Init(Context* context, RHI::Handle<Buffer> bufferHandle, const RHI::BufferAttachmentUseInfo& useInfo);
        void Shutdown(Context* context);
    };

    struct BindGroupLayout : RHI::BindGroupLayout
    {
        VkDescriptorSetLayout handle;

        RHI::ResultCode Init(Context* context, const RHI::BindGroupLayoutCreateInfo& createInfo);
        void Shutdown(Context* context);
    };

    struct BindGroup : RHI::BindGroup
    {
        VkDescriptorSet handle;
        VkDescriptorPool pool;

        RHI::ResultCode Init(Context* context, VkDescriptorSetLayout layout, VkDescriptorPool pool);
        void Shutdown(Context* context);
    };

    struct PipelineLayout : RHI::PipelineLayout
    {
        VkPipelineLayout handle;

        RHI::ResultCode Init(Context* context, const RHI::PipelineLayoutCreateInfo& createInfo);
        void Shutdown(Context* context);
    };

    struct GraphicsPipeline : RHI::GraphicsPipeline
    {
        VkPipeline handle;
        VkPipelineLayout layout;

        RHI::ResultCode Init(Context* context, const RHI::GraphicsPipelineCreateInfo& createInfo);
        void Shutdown(Context* context);
    };

    struct ComputePipeline : RHI::ComputePipeline
    {
        VkPipeline handle;
        VkPipelineLayout layout;

        RHI::ResultCode Init(Context* context, const RHI::ComputePipelineCreateInfo& createInfo);
        void Shutdown(Context* context);
    };

    struct Sampler : RHI::Sampler
    {
        VkSampler handle;

        RHI::ResultCode Init(Context* context, const RHI::SamplerCreateInfo& createInfo);
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
        void Free(TL::Span<RHI::Handle<RHI::BindGroup>> bindGroup) override;
        void Update(RHI::Handle<RHI::BindGroup> bindGroup, const RHI::BindGroupData& data) override;

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