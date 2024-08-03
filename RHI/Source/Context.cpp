#include "RHI/Context.hpp"
#include "RHI/CommandList.hpp"
#include "RHI/RenderGraph.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Swapchain.hpp"
#include "RHI/Common/Callstack.hpp"
#include <RHI/Common/Handle.hpp>

#include <TL/Assert.hpp>
#include <TL/Log.hpp>

#include <tracy/Tracy.hpp>

namespace RHI
{
    Context::Context()
        : m_limits(CreatePtr<Limits>())
    {
    }

    Context::~Context()
    {
    }

    Limits Context::GetLimits() const
    {
        return *m_limits;
    }

    Ptr<RenderGraph> Context::CreateRenderGraph()
    {
        ZoneScoped;

        return CreatePtr<RenderGraph>(this);
    }

    void Context::CompileRenderGraph([[maybe_unused]] RenderGraph& renderGraph)
    {
        ZoneScoped;
        // nothing to do here (just ensure correct attachment sizes)
    }

    void Context::ExecuteRenderGraph(RenderGraph& renderGraph, Fence* signalFence)
    {
        ZoneScoped;

        Internal_DispatchGraph(renderGraph, signalFence);
    }

    Ptr<Swapchain> Context::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        TL_ASSERT(createInfo.imageSize.width > 0 && createInfo.imageSize.height > 0);
        TL_ASSERT(createInfo.minImageCount >= Swapchain::MinImageCount);
        TL_ASSERT(createInfo.minImageCount <= Swapchain::MaxImageCount);
        TL_ASSERT(createInfo.imageFormat != Format::Unknown);
        TL_ASSERT(createInfo.imageUsage != ImageUsage::None);

        return Internal_CreateSwapchain(createInfo);
    }

    Ptr<ShaderModule> Context::CreateShaderModule(TL::Span<const uint32_t> shaderBlob)
    {
        ZoneScoped;

        return Internal_CreateShaderModule(shaderBlob);
    }

    Ptr<Fence> Context::CreateFence()
    {
        ZoneScoped;

        return Internal_CreateFence();
    }

    Ptr<CommandPool> Context::CreateCommandPool(CommandPoolFlags flags)
    {
        ZoneScoped;

        return Internal_CreateCommandPool(flags);
    }

    Handle<BindGroupLayout> Context::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        ZoneScoped;

        TL_ASSERT(ValidateCreateInfo(createInfo));
        return Internal_CreateBindGroupLayout(createInfo);
    }

    void Context::DestroyBindGroupLayout(Handle<BindGroupLayout> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Internal_DestroyBindGroupLayout(handle);
    }

    Handle<BindGroup> Context::CreateBindGroup(Handle<BindGroupLayout> bindGroupLayoutHandle, uint32_t bindlessElementsCount)
    {
        ZoneScoped;

        return Internal_CreateBindGroup(bindGroupLayoutHandle, bindlessElementsCount);
    }

    void Context::DestroyBindGroup(Handle<BindGroup> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Internal_DestroyBindGroup(handle);
    }

    void Context::UpdateBindGroup(Handle<BindGroup> handle, TL::Span<const BindGroupUpdateInfo> bindings)
    {
        ZoneScoped;

        Internal_UpdateBindGroup(handle, bindings);
    }

    Handle<PipelineLayout> Context::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        ZoneScoped;

        TL_ASSERT(ValidateCreateInfo(createInfo));
        return Internal_CreatePipelineLayout(createInfo);
    }

    void Context::DestroyPipelineLayout(Handle<PipelineLayout> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Internal_DestroyPipelineLayout(handle);
    }

    Handle<GraphicsPipeline> Context::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        ZoneScoped;

        TL_ASSERT(ValidateCreateInfo(createInfo));
        return Internal_CreateGraphicsPipeline(createInfo);
    }

    void Context::DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Internal_DestroyGraphicsPipeline(handle);
    }

    Handle<ComputePipeline> Context::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        ZoneScoped;

        TL_ASSERT(ValidateCreateInfo(createInfo));
        return Internal_CreateComputePipeline(createInfo);
    }

    void Context::DestroyComputePipeline(Handle<ComputePipeline> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Internal_DestroyComputePipeline(handle);
    }

    Handle<Sampler> Context::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        ZoneScoped;

        TL_ASSERT(ValidateCreateInfo(createInfo));
        return Internal_CreateSampler(createInfo);
    }

    void Context::DestroySampler(Handle<Sampler> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Internal_DestroySampler(handle);
    }

    Result<Handle<Image>> Context::CreateImage(const ImageCreateInfo& createInfo)
    {
        ZoneScoped;

        TL_ASSERT(ValidateCreateInfo(createInfo));
        return Internal_CreateImage(createInfo);
    }

    void Context::DestroyImage(Handle<Image> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Internal_DestroyImage(handle);
    }

    Result<Handle<Buffer>> Context::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        ZoneScoped;

        TL_ASSERT(ValidateCreateInfo(createInfo));
        return Internal_CreateBuffer(createInfo);
    }

    void Context::DestroyBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Internal_DestroyBuffer(handle);
    }

    Handle<ImageView> Context::CreateImageView(const ImageViewCreateInfo& createInfo)
    {
        ZoneScoped;

        TL_ASSERT(ValidateCreateInfo(createInfo));
        return Internal_CreateImageView(createInfo);
    }

    void Context::DestroyImageView(Handle<ImageView> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Internal_DestroyImageView(handle);
    }

    Handle<BufferView> Context::CreateBufferView(const BufferViewCreateInfo& createInfo)
    {
        ZoneScoped;

        TL_ASSERT(ValidateCreateInfo(createInfo));
        return Internal_CreateBufferView(createInfo);
    }

    void Context::DestroyBufferView(Handle<BufferView> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Internal_DestroyBufferView(handle);
    }

    DeviceMemoryPtr Context::MapBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;

        return Internal_MapBuffer(handle);
    }

    void Context::UnmapBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;

        Internal_UnmapBuffer(handle);
    }

    StagingBuffer Context::AllocateTempBuffer(size_t size)
    {
        ZoneScoped;

        BufferCreateInfo createInfo{};
        createInfo.byteSize = size;
        createInfo.usageFlags = BufferUsage::CopySrc | BufferUsage::CopyDst;
        auto buffer = CreateBuffer(createInfo).GetValue();
        auto ptr = MapBuffer(buffer);
        m_stagingBuffers.push_back(buffer);
        return StagingBuffer{ ptr, buffer, 0 };
    }

    void Context::StageResourceWrite(Handle<Image> image, ImageSubresourceLayers subresources, Handle<Buffer> buffer, size_t bufferOffset)
    {
        ZoneScoped;

        Internal_StageResourceWrite(image, subresources, buffer, bufferOffset);
    }

    void Context::StageResourceWrite(Handle<Buffer> buffer, size_t offset, size_t size, Handle<Buffer> srcBuffer, size_t srcOffset)
    {
        ZoneScoped;

        Internal_StageResourceWrite(buffer, offset, size, srcBuffer, srcOffset);
    }

    void Context::StageResourceRead(Handle<Image> image, ImageSubresourceLayers subresources, Handle<Buffer> buffer, size_t bufferOffset, Fence* fence)
    {
        ZoneScoped;

        Internal_StageResourceRead(image, subresources, buffer, bufferOffset, fence);
    }

    void Context::StageResourceRead(Handle<Buffer> buffer, size_t offset, size_t size, Handle<Buffer> srcBuffer, size_t srcOffset, Fence* fence)
    {
        ZoneScoped;

        Internal_StageResourceRead(buffer, offset, size, srcBuffer, srcOffset, fence);
    }

#define TRY(condition, message)    \
    if ((condition) == false)      \
    {                              \
        TL_LOG_INFO(message); \
        return false;              \
    }

    bool Context::ValidateCreateInfo(const SwapchainCreateInfo& createInfo) const
    {
        (void)createInfo;
        return true;
    }

    bool Context::ValidateCreateInfo(const BindGroupLayoutCreateInfo& createInfo) const
    {
        (void)createInfo;
        return true;
    }

    bool Context::ValidateCreateInfo(const PipelineLayoutCreateInfo& createInfo) const
    {
        (void)createInfo;
        return true;
    }

    bool Context::ValidateCreateInfo(const GraphicsPipelineCreateInfo& createInfo) const
    {
        (void)createInfo;
        return true;
    }

    bool Context::ValidateCreateInfo(const ComputePipelineCreateInfo& createInfo) const
    {
        (void)createInfo;
        return true;
    }

    bool Context::ValidateCreateInfo(const SamplerCreateInfo& createInfo) const
    {
        (void)createInfo;
        return true;
    }

    bool Context::ValidateCreateInfo(const ImageCreateInfo& createInfo) const
    {
        TRY(createInfo.usageFlags != RHI::ImageUsage::None, "Invalid usage flags");
        if (createInfo.type == ImageType::Image1D)
        {
            TRY(createInfo.size.height == 1 && createInfo.size.depth == 1, "1D Images should have 1 in width and depth size paramters");
        }
        else if (createInfo.type == ImageType::Image2D)
        {
            TRY(createInfo.size.depth == 1, "2D Images should have 1 in the depth paramter");
        }
        else if (createInfo.type == ImageType::Image3D)
        {
        }
        else
        {
            TL_LOG_INFO("Invalid value for ImageCreateInfo::type");
            return false;
        }

        TRY(createInfo.format != Format::Unknown, "Inavlid format for image");
        TRY(createInfo.sampleCount != SampleCount::None, "Invalid value for ImageCreateInfo::sampleCount");
        TRY(createInfo.mipLevels != 0, "Invalid value for ImageCreateInfo::mipLevels");
        TRY(createInfo.arrayCount != 0, "Invalid value for ImageCreateInfo::arrayCount");

        return true;
    }

    bool Context::ValidateCreateInfo(const BufferCreateInfo& createInfo) const
    {
        (void)createInfo;
        return true;
    }

    bool Context::ValidateCreateInfo(const ImageViewCreateInfo& createInfo) const
    {
        TRY(createInfo.image != NullHandle, "Invalid Image handle");
        TRY(createInfo.viewType != ImageViewType::None, "Invalid value for ImageViewCreateInfo::type");
        TRY(createInfo.subresource.imageAspects != ImageAspect::None, "Invalid value for ImageViewCreateInfo::subresource::imageAspects");
        TRY(createInfo.subresource.arrayCount != 0, "Invalid value for ImageViewCreateInfo::subresource::arrayCount");
        TRY(createInfo.subresource.mipLevelCount != 0, "Invalid value for ImageViewCreateInfo::subresource::mipLevelCount");
        return true;
    }

    bool Context::ValidateCreateInfo(const BufferViewCreateInfo& createInfo) const
    {
        (void)createInfo;
        return true;
    }

} // namespace RHI