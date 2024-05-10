#include "RHI/Context.hpp"
#include "RHI/CommandList.hpp"
#include "RHI/RenderGraph.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Swapchain.hpp"

#include "RHI/Common/Assert.hpp"
#include "RHI/Common/LeakDetector.hpp"

#include <tracy/Tracy.hpp>

namespace RHI
{
    class ResourceTracker
    {
    public:
        ResourceTracker() = default;

        inline uint32_t LiveResourcesCount()
        {
            uint32_t totalCount = 0;
            totalCount += m_images.LeakedResourcesCount();
            totalCount += m_buffers.LeakedResourcesCount();
            totalCount += m_imageViews.LeakedResourcesCount();
            totalCount += m_bufferViews.LeakedResourcesCount();
            totalCount += m_bindGroupLayouts.LeakedResourcesCount();
            totalCount += m_bindGroups.LeakedResourcesCount();
            totalCount += m_pipelineLayouts.LeakedResourcesCount();
            totalCount += m_graphicsPipelines.LeakedResourcesCount();
            totalCount += m_computePipelines.LeakedResourcesCount();
            totalCount += m_samplers.LeakedResourcesCount();
            return totalCount;
        }

        inline TL::String ReportLiveResources()
        {
            std::string report = m_images.ReportLiveResources();
            report += m_buffers.ReportLiveResources();
            report += m_imageViews.ReportLiveResources();
            report += m_bufferViews.ReportLiveResources();
            report += m_bindGroupLayouts.ReportLiveResources();
            report += m_bindGroups.ReportLiveResources();
            report += m_pipelineLayouts.ReportLiveResources();
            report += m_graphicsPipelines.ReportLiveResources();
            report += m_computePipelines.ReportLiveResources();
            report += m_samplers.ReportLiveResources();
            return TL::String{ report };
        }

        // clang-format off

        inline Handle<Image>            Register(Handle<Image> handle) { m_images.OnCreate(handle); return handle; }
        inline void                     Unregister(Handle<Image> handle) { m_images.OnDestroy(handle); }
        inline Handle<Buffer>           Register(Handle<Buffer> handle) { m_buffers.OnCreate(handle); return handle; }
        inline void                     Unregister(Handle<Buffer> handle) { m_buffers.OnDestroy(handle); }
        inline Handle<ImageView>        Register(Handle<ImageView> handle) { m_imageViews.OnCreate(handle); return handle; }
        inline void                     Unregister(Handle<ImageView> handle) { m_imageViews.OnDestroy(handle); }
        inline Handle<BufferView>       Register(Handle<BufferView> handle) { m_bufferViews.OnCreate(handle); return handle; }
        inline void                     Unregister(Handle<BufferView> handle) { m_bufferViews.OnDestroy(handle); }
        inline Handle<BindGroupLayout>  Register(Handle<BindGroupLayout> handle) { m_bindGroupLayouts.OnCreate(handle); return handle; }
        inline void                     Unregister(Handle<BindGroupLayout> handle) { m_bindGroupLayouts.OnDestroy(handle); }
        inline Handle<BindGroup>        Register(Handle<BindGroup> handle) { m_bindGroups.OnCreate(handle); return handle; }
        inline void                     Unregister(Handle<BindGroup> handle) { m_bindGroups.OnDestroy(handle); }
        inline Handle<PipelineLayout>   Register(Handle<PipelineLayout> handle) { m_pipelineLayouts.OnCreate(handle); return handle; }
        inline void                     Unregister(Handle<PipelineLayout> handle) { m_pipelineLayouts.OnDestroy(handle); }
        inline Handle<GraphicsPipeline> Register(Handle<GraphicsPipeline> handle) { m_graphicsPipelines.OnCreate(handle); return handle; }
        inline void                     Unregister(Handle<GraphicsPipeline> handle) { m_graphicsPipelines.OnDestroy(handle); }
        inline Handle<ComputePipeline>  Register(Handle<ComputePipeline> handle) { m_computePipelines.OnCreate(handle); return handle; }
        inline void                     Unregister(Handle<ComputePipeline> handle) { m_computePipelines.OnDestroy(handle); }
        inline Handle<Sampler>          Register(Handle<Sampler> handle) { m_samplers.OnCreate(handle); return handle; }
        inline void                     Unregister(Handle<Sampler> handle) { m_samplers.OnDestroy(handle); }

        // clang-format on

    private:
        ResourceLeakDetector<Image> m_images;
        ResourceLeakDetector<Buffer> m_buffers;
        ResourceLeakDetector<ImageView> m_imageViews;
        ResourceLeakDetector<BufferView> m_bufferViews;
        ResourceLeakDetector<BindGroupLayout> m_bindGroupLayouts;
        ResourceLeakDetector<BindGroup> m_bindGroups;
        ResourceLeakDetector<PipelineLayout> m_pipelineLayouts;
        ResourceLeakDetector<GraphicsPipeline> m_graphicsPipelines;
        ResourceLeakDetector<ComputePipeline> m_computePipelines;
        ResourceLeakDetector<Sampler> m_samplers;
    };

    inline static ImageViewCreateInfo GetViewCreateInfo(Handle<Image> image, const ImageAttachmentUseInfo& useInfo)
    {
        ImageViewCreateInfo createInfo{};
        createInfo.image = image;
        createInfo.subresource = useInfo.subresourceRange;
        createInfo.viewType = ImageViewType::View2D; // TODO: this should be refelected based on other paramters
        createInfo.components = useInfo.componentMapping;
        return createInfo;
    }

    Context::Context(Ptr<DebugCallbacks> debugCallbacks)
        : m_limits(CreatePtr<Limits>())
        , m_debugCallbacks(std::move(debugCallbacks))
        , m_resourceTracker(new ResourceTracker())
    {
    }

    Ptr<RenderGraph> Context::CreateRenderGraph()
    {
        ZoneScoped;

        return CreatePtr<RenderGraph>(this);
    }

    void Context::CompileRenderGraph(RenderGraph& renderGraph)
    {
        ZoneScoped;

        for (auto graphAttachmentHandle : renderGraph.m_graphImageAttachments)
        {
            auto graphAttachment = renderGraph.m_graphImageAttachmentOwner.Get(graphAttachmentHandle);
            if (graphAttachment->lifetime == AttachmentLifetime::Transient)
            {
                graphAttachment->handle = CreateImage(graphAttachment->info).GetValue();
            }

            if (graphAttachment->swapchain)
            {
                continue;
            }

            for (auto attachment = renderGraph.GetAttachment(graphAttachment->begin); attachment != nullptr; attachment = renderGraph.GetAttachmentNext(attachment->next))
            {
                auto createInfo = GetViewCreateInfo(graphAttachment->handle, attachment->useInfo);
                attachment->view = CreateImageView(createInfo);
            }
        }
    }

    void Context::ExecuteRenderGraph(RenderGraph& renderGraph, Fence* signalFence)
    {
        ZoneScoped;

        Internal_DispatchGraph(renderGraph, signalFence);
    }

    Ptr<Swapchain> Context::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        RHI_ASSERT(createInfo.imageSize.width > 0 && createInfo.imageSize.height > 0);
        RHI_ASSERT(createInfo.imageCount >= Swapchain::MinImageCount && createInfo.imageCount <= Swapchain::MaxImageCount);
        RHI_ASSERT(createInfo.imageFormat != Format::Unknown);
        RHI_ASSERT(createInfo.imageUsage != ImageUsage::None);

        return Internal_CreateSwapchain(createInfo);
    }

    Ptr<ShaderModule> Context::CreateShaderModule(TL::Span<const uint8_t> shaderBlob)
    {
        ZoneScoped;

        return Internal_CreateShaderModule(shaderBlob);
    }

    Ptr<Fence> Context::CreateFence()
    {
        ZoneScoped;

        return Internal_CreateFence();
    }

    Ptr<CommandPool> Context::CreateCommandPool()
    {
        ZoneScoped;

        return Internal_CreateCommandPool();
    }

    Ptr<ResourcePool> Context::CreateResourcePool(const ResourcePoolCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateResourcePool(createInfo);
    }

    Handle<BindGroupLayout> Context::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        ZoneScoped;

        auto handle = Internal_CreateBindGroupLayout(createInfo);
        m_resourceTracker->Register(handle);
        return handle;
    }

    void Context::DestroyBindGroupLayout(Handle<BindGroupLayout> handle)
    {
        ZoneScoped;

        Internal_DestroyBindGroupLayout(handle);
        m_resourceTracker->Unregister(handle);
    }

    Handle<BindGroup> Context::CreateBindGroup(Handle<BindGroupLayout> bindGroupLayoutHandle)
    {
        ZoneScoped;

        auto handle = Internal_CreateBindGroup(bindGroupLayoutHandle);
        m_resourceTracker->Register(handle);
        return handle;
    }

    void Context::DestroyBindGroup(Handle<BindGroup> handle)
    {
        ZoneScoped;

        Internal_DestroyBindGroup(handle);
        m_resourceTracker->Unregister(handle);
    }

    void Context::UpdateBindGroup(Handle<BindGroup> handle, const BindGroupData& data)
    {
        ZoneScoped;

        Internal_UpdateBindGroup(handle, data);
        m_resourceTracker->Unregister(handle);
    }

    Handle<PipelineLayout> Context::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreatePipelineLayout(createInfo);
    }

    void Context::DestroyPipelineLayout(Handle<PipelineLayout> handle)
    {
        ZoneScoped;

        Internal_DestroyPipelineLayout(handle);
        m_resourceTracker->Unregister(handle);
    }

    Handle<GraphicsPipeline> Context::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateGraphicsPipeline(createInfo);
    }

    void Context::DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)
    {
        ZoneScoped;

        Internal_DestroyGraphicsPipeline(handle);
        m_resourceTracker->Unregister(handle);
    }

    Handle<ComputePipeline> Context::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateComputePipeline(createInfo);
    }

    void Context::DestroyComputePipeline(Handle<ComputePipeline> handle)
    {
        ZoneScoped;

        Internal_DestroyComputePipeline(handle);
        m_resourceTracker->Unregister(handle);
    }

    Handle<Sampler> Context::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateSampler(createInfo);
    }

    void Context::DestroySampler(Handle<Sampler> handle)
    {
        ZoneScoped;

        Internal_DestroySampler(handle);
        m_resourceTracker->Unregister(handle);
    }

    Result<Handle<Image>> Context::CreateImage(const ImageCreateInfo& createInfo)
    {
        ZoneScoped;

        RHI_ASSERT(createInfo.usageFlags != ImageUsage::None);
        RHI_ASSERT(createInfo.type != ImageType::None);

        if (createInfo.type == ImageType::Image1D)
        {
            RHI_ASSERT(createInfo.size.width >= 1);
            RHI_ASSERT(createInfo.size.height == 1);
            RHI_ASSERT(createInfo.size.depth == 1);
        }
        else if (createInfo.type == ImageType::Image2D)
        {
            RHI_ASSERT(createInfo.size.width >= 1);
            RHI_ASSERT(createInfo.size.height >= 1);
            RHI_ASSERT(createInfo.size.depth == 1);
        }
        else if (createInfo.type == ImageType::Image3D)
        {
            RHI_ASSERT(createInfo.size.width >= 1);
            RHI_ASSERT(createInfo.size.height >= 1);
            RHI_ASSERT(createInfo.size.depth >= 1);
        }
        else
        {
            RHI_UNREACHABLE();
        }

        RHI_ASSERT(createInfo.format != Format::Unknown);

        return Internal_CreateImage(createInfo);
    }

    void Context::DestroyImage(Handle<Image> handle)
    {
        ZoneScoped;

        Internal_DestroyImage(handle);
        m_resourceTracker->Unregister(handle);
    }

    Result<Handle<Buffer>> Context::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        ZoneScoped;

        RHI_ASSERT(createInfo.usageFlags != BufferUsage::None);
        RHI_ASSERT(createInfo.byteSize != 0);

        return Internal_CreateBuffer(createInfo);
    }

    void Context::DestroyBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;

        Internal_DestroyBuffer(handle);
        m_resourceTracker->Unregister(handle);
    }

    Handle<ImageView> Context::CreateImageView(const ImageViewCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateImageView(createInfo);
    }

    void Context::DestroyImageView(Handle<ImageView> handle)
    {
        ZoneScoped;

        Internal_DestroyImageView(handle);
        m_resourceTracker->Unregister(handle);
    }

    Handle<BufferView> Context::CreateBufferView(const BufferViewCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateBufferView(createInfo);
    }

    void Context::DestroyBufferView(Handle<BufferView> handle)
    {
        ZoneScoped;

        Internal_DestroyBufferView(handle);
        m_resourceTracker->Unregister(handle);
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

    void Context::DebugLogError(std::string_view message)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogError(message);
#else
        (void)message;
#endif
    }

    void Context::DebugLogWarn(std::string_view message)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogWarnning(message);
#else
        (void)message;
#endif
    }

    void Context::DebugLogInfo(std::string_view message)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogInfo(message);
#else
        (void)message;
#endif
    }
} // namespace RHI