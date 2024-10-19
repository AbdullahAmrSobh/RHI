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

    Device::Device()
        : m_limits(TL::CreatePtr<DeviceLimits>())
    {
    }

    Device::~Device()
    {
    }

    DeviceLimits Device::GetLimits() const
    {
        return *m_limits;
    }

    TL::Ptr<RenderGraph> Device::CreateRenderGraph()
    {
        ZoneScoped;

        return TL::CreatePtr<RenderGraph>(this);
    }

    TL::Ptr<Swapchain> Device::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        TL_ASSERT(createInfo.imageSize.width > 0 && createInfo.imageSize.height > 0);
        TL_ASSERT(createInfo.minImageCount >= Swapchain::MinImageCount);
        TL_ASSERT(createInfo.minImageCount <= Swapchain::MaxImageCount);
        TL_ASSERT(createInfo.imageFormat != Format::Unknown);
        TL_ASSERT(createInfo.imageUsage != ImageUsage::None);

        return Impl_CreateSwapchain(createInfo);
    }

    TL::Ptr<ShaderModule> Device::CreateShaderModule(TL::Span<const uint32_t> shaderBlob)
    {
        ZoneScoped;

        return Impl_CreateShaderModule(shaderBlob);
    }

    TL::Ptr<Fence> Device::CreateFence()
    {
        ZoneScoped;

        return Impl_CreateFence();
    }

    TL::Ptr<CommandPool> Device::CreateCommandPool(CommandPoolFlags flags)
    {
        ZoneScoped;

        return Impl_CreateCommandPool(flags);
    }

    Handle<BindGroupLayout> Device::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        ZoneScoped;

        return Impl_CreateBindGroupLayout(createInfo);
    }

    void Device::DestroyBindGroupLayout(Handle<BindGroupLayout> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Impl_DestroyBindGroupLayout(handle);
    }

    Handle<BindGroup> Device::CreateBindGroup(Handle<BindGroupLayout> bindGroupLayoutHandle)
    {
        ZoneScoped;

        return Impl_CreateBindGroup(bindGroupLayoutHandle);
    }

    void Device::DestroyBindGroup(Handle<BindGroup> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Impl_DestroyBindGroup(handle);
    }

    void Device::UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo)
    {
        ZoneScoped;

        Impl_UpdateBindGroup(handle, updateInfo);
    }

    Handle<PipelineLayout> Device::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        ZoneScoped;

        return Impl_CreatePipelineLayout(createInfo);
    }

    void Device::DestroyPipelineLayout(Handle<PipelineLayout> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Impl_DestroyPipelineLayout(handle);
    }

    Handle<GraphicsPipeline> Device::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        ZoneScoped;

        return Impl_CreateGraphicsPipeline(createInfo);
    }

    void Device::DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Impl_DestroyGraphicsPipeline(handle);
    }

    Handle<ComputePipeline> Device::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        ZoneScoped;

        return Impl_CreateComputePipeline(createInfo);
    }

    void Device::DestroyComputePipeline(Handle<ComputePipeline> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Impl_DestroyComputePipeline(handle);
    }

    Handle<Sampler> Device::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        ZoneScoped;

        return Impl_CreateSampler(createInfo);
    }

    void Device::DestroySampler(Handle<Sampler> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Impl_DestroySampler(handle);
    }

    Result<Handle<Image>> Device::CreateImage(const ImageCreateInfo& createInfo)
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

        return Impl_CreateImage(createInfo);
    }

    void Device::DestroyImage(Handle<Image> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Impl_DestroyImage(handle);
    }

    Result<Handle<Buffer>> Device::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        ZoneScoped;

        return Impl_CreateBuffer(createInfo);
    }

    void Device::DestroyBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Impl_DestroyBuffer(handle);
    }

    Handle<ImageView> Device::CreateImageView(const ImageViewCreateInfo& createInfo)
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

        return Impl_CreateImageView(createInfo);
    }

    void Device::DestroyImageView(Handle<ImageView> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Impl_DestroyImageView(handle);
    }

    Handle<BufferView> Device::CreateBufferView(const BufferViewCreateInfo& createInfo)
    {
        ZoneScoped;

        return Impl_CreateBufferView(createInfo);
    }

    void Device::DestroyBufferView(Handle<BufferView> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Impl_DestroyBufferView(handle);
    }

    DeviceMemoryPtr Device::MapBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;

        return Impl_MapBuffer(handle);
    }

    void Device::UnmapBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;

        Impl_UnmapBuffer(handle);
    }

    Handle<Semaphore> Device::CreateSemaphore(const SemaphoreCreateInfo& createInfo)
    {
        ZoneScoped;

        return Impl_CreateSemaphore(createInfo);
    }

    void Device::DestroySemaphore(Handle<Semaphore> handle)
    {
        return Impl_DestroySemaphore(handle);
    }

    Queue* Device::GetQueue(QueueType queueType)
    {
        return Impl_GetQueue(queueType);
    }

    void Device::CollectResources()
    {
        Impl_CollectResources();
    }

} // namespace RHI