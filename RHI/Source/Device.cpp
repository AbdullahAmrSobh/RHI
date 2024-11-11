#include <RHI/RHI.hpp>

#include <TL/Assert.hpp>
#include <TL/Log.hpp>

#include <tracy/Tracy.hpp>

namespace RHI
{
    Device::Device()
        : m_limits(TL::CreatePtr<DeviceLimits>())
    {
    }

    Device::~Device() = default;

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

    TL::Ptr<CommandList> Device::CreateCommandList(QueueType queueType)
    {
        ZoneScoped;

        return Impl_CreateCommandList(queueType);
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

        /// @todo: Validate using Device::GetLimits();

        if (true) // validation
        {
            auto limits = GetLimits();

            if (createInfo.usageFlags == RHI::ImageUsage::None)
            {
                TL_LOG_INFO("Invalid usage flags");
                return ResultCode::ErrorUnknown;
            }

            if (createInfo.type == ImageType::Image1D)
            {
                if (createInfo.size.height != 1 || createInfo.size.depth != 1)
                {
                    TL_LOG_INFO("1D Images should have 1 in width and depth size parameters");
                    return ResultCode::ErrorUnknown;
                }
            }
            else if (createInfo.type == ImageType::Image2D)
            {
                if (createInfo.size.depth != 1)
                {
                    TL_LOG_INFO("2D Images should have 1 in the depth parameter");
                    return ResultCode::ErrorUnknown;
                }
            }
            else if (createInfo.type != ImageType::Image3D)
            {
                TL_LOG_INFO("Invalid value for ImageCreateInfo::type");
                return ResultCode::ErrorUnknown;
            }

            if (createInfo.format == Format::Unknown)
            {
                TL_LOG_INFO("Invalid format for image");
                return ResultCode::ErrorUnknown;
            }

            if (createInfo.sampleCount == SampleCount::None)
            {
                TL_LOG_INFO("Invalid value for ImageCreateInfo::sampleCount");
                return ResultCode::ErrorUnknown;
            }

            if (createInfo.mipLevels == 0)
            {
                TL_LOG_INFO("Invalid value for ImageCreateInfo::mipLevels");
                return ResultCode::ErrorUnknown;
            }

            if (createInfo.arrayCount == 0)
            {
                TL_LOG_INFO("Invalid value for ImageCreateInfo::arrayCount");
                return ResultCode::ErrorUnknown;
            }
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

        /// @todo: Validate using Device::GetLimits();

        // Validation
        auto limits = GetLimits();

        if (true)
        {
            if (createInfo.usageFlags == RHI::BufferUsage::None)
            {
                TL_LOG_INFO("Invalid usage flags for buffer");
                return ResultCode::ErrorUnknown;
            }

            if (createInfo.byteSize == 0)
            {
                TL_LOG_INFO("Buffer size must be greater than zero");
                return ResultCode::ErrorUnknown;
            }

            if (createInfo.heapType == MemoryType::None)
            {
                TL_LOG_INFO("Invalid memory type for buffer");
                return ResultCode::ErrorUnknown;
            }
        }

        return Impl_CreateBuffer(createInfo);
    }

    void Device::DestroyBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;
        TL_ASSERT(handle != NullHandle);
        Impl_DestroyBuffer(handle);
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

    Queue* Device::GetQueue(QueueType queueType)
    {
        return Impl_GetQueue(queueType);
    }

    void Device::WaitTimelineValue(uint64_t value)
    {
        Impl_WaitTimelineValue(value);
    }

    StagingBuffer Device::StagingAllocate(size_t size)
    {
        return Impl_StagingAllocate(size);
    }

    uint64_t Device::UploadImage(const ImageUploadInfo& uploadInfo)
    {
        return Impl_UploadImage(uploadInfo);
    }

    void Device::CollectResources()
    {
        Impl_CollectResources();
    }

} // namespace RHI