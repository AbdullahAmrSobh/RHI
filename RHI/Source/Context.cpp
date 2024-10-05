#include <RHI/RHI.hpp>

#include <TL/Assert.hpp>
#include <TL/Log.hpp>

#include <tracy/Tracy.hpp>

namespace RHI
{
#define TRY(condition, message, return_type) \
    if ((condition) == false)                \
    {                                        \
        TL_LOG_INFO(message);                \
        return return_type;                  \
    }

    Context::Context()
        : m_limits(TL::CreatePtr<Limits>())
    {
    }

    Context::~Context()
    {
    }

    Limits Context::GetLimits() const
    {
        return *m_limits;
    }

    TL::Ptr<RenderGraph> Context::CreateRenderGraph()
    {
        ZoneScoped;

        return TL::CreatePtr<RenderGraph>(this);
    }

    void Context::CompileRenderGraph([[maybe_unused]] RenderGraph& renderGraph)
    {
        ZoneScoped;
    }

    TL::Ptr<Swapchain> Context::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        TL_ASSERT(createInfo.imageSize.width > 0 && createInfo.imageSize.height > 0);
        TL_ASSERT(createInfo.minImageCount >= Swapchain::MinImageCount);
        TL_ASSERT(createInfo.minImageCount <= Swapchain::MaxImageCount);
        TL_ASSERT(createInfo.imageFormat != Format::Unknown);
        TL_ASSERT(createInfo.imageUsage != ImageUsage::None);

        return Internal_CreateSwapchain(createInfo);
    }

    TL::Ptr<ShaderModule> Context::CreateShaderModule(TL::Span<const uint32_t> shaderBlob)
    {
        ZoneScoped;

        return Internal_CreateShaderModule(shaderBlob);
    }

    TL::Ptr<Fence> Context::CreateFence()
    {
        ZoneScoped;

        return Internal_CreateFence();
    }

    TL::Ptr<CommandPool> Context::CreateCommandPool(CommandPoolFlags flags)
    {
        ZoneScoped;

        return Internal_CreateCommandPool(flags);
    }

    Handle<BindGroupLayout> Context::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateBindGroupLayout(createInfo);
    }

    void Context::DestroyBindGroupLayout(Handle<BindGroupLayout> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Internal_DestroyBindGroupLayout(handle);
    }

    Handle<BindGroup> Context::CreateBindGroup(Handle<BindGroupLayout> bindGroupLayoutHandle)
    {
        ZoneScoped;

        return Internal_CreateBindGroup(bindGroupLayoutHandle);
    }

    void Context::DestroyBindGroup(Handle<BindGroup> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Internal_DestroyBindGroup(handle);
    }

    void Context::UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo)
    {
        ZoneScoped;

        Internal_UpdateBindGroup(handle, updateInfo);
    }

    Handle<PipelineLayout> Context::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        ZoneScoped;

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

        if (true) // validation
        {
            TRY(createInfo.usageFlags != RHI::ImageUsage::None, "Invalid usage flags", ResultCode::ErrorUnknown);
            if (createInfo.type == ImageType::Image1D)
            {
                TRY(createInfo.size.height == 1 && createInfo.size.depth == 1, "1D Images should have 1 in width and depth size paramters", ResultCode::ErrorUnknown);
            }
            else if (createInfo.type == ImageType::Image2D)
            {
                TRY(createInfo.size.depth == 1, "2D Images should have 1 in the depth paramter", ResultCode::ErrorUnknown);
            }
            else if (createInfo.type == ImageType::Image3D)
            {
            }
            else
            {
                TL_LOG_INFO("Invalid value for ImageCreateInfo::type");
                return ResultCode::ErrorUnknown;
            }

            TRY(createInfo.format != Format::Unknown, "Inavlid format for image", ResultCode::ErrorUnknown);
            TRY(createInfo.sampleCount != SampleCount::None, "Invalid value for ImageCreateInfo::sampleCount", ResultCode::ErrorUnknown);
            TRY(createInfo.mipLevels != 0, "Invalid value for ImageCreateInfo::mipLevels", ResultCode::ErrorUnknown);
            TRY(createInfo.arrayCount != 0, "Invalid value for ImageCreateInfo::arrayCount", ResultCode::ErrorUnknown);
        }

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

        if (true)
        {
            TRY(createInfo.image != NullHandle, "Invalid Image handle", NullHandle);
            TRY(createInfo.viewType != ImageViewType::None, "Invalid value for ImageViewCreateInfo::type", NullHandle);
            TRY(createInfo.subresource.imageAspects != ImageAspect::None, "Invalid value for ImageViewCreateInfo::subresource::imageAspects", NullHandle);
            TRY(createInfo.subresource.arrayCount != 0, "Invalid value for ImageViewCreateInfo::subresource::arrayCount", NullHandle);
            TRY(createInfo.subresource.mipLevelCount != 0, "Invalid value for ImageViewCreateInfo::subresource::mipLevelCount", NullHandle);
        }

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

    Handle<Semaphore> Context::CreateSemaphore(const SemaphoreCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateSemaphore(createInfo);
    }

    void Context::DestroySemaphore(Handle<Semaphore> handle)
    {
        return Internal_DestroySemaphore(handle);
    }

    Queue* Context::GetQueue(QueueType queueType)
    {
        return Internal_GetQueue(queueType);
    }

    void Context::CollectResources()
    {
        Internal_CollectResources();
    }

} // namespace RHI