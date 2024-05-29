#include "RHI/Context.hpp"
#include "RHI/CommandList.hpp"
#include "RHI/RenderGraph.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Swapchain.hpp"

#include "RHI/Common/Assert.hpp"
#include "RHI/Common/Callstack.hpp"

#include <tracy/Tracy.hpp>

#include <format>

namespace RHI
{
    class ResourceTracker
    {
    public:
        ResourceTracker() = default;

        inline size_t LiveResourcesCount()
        {
            size_t totalCount = 0;
            totalCount += m_images.size();
            totalCount += m_buffers.size();
            totalCount += m_imageViews.size();
            totalCount += m_bufferViews.size();
            totalCount += m_bindGroupLayouts.size();
            totalCount += m_bindGroups.size();
            totalCount += m_pipelineLayouts.size();
            totalCount += m_graphicsPipelines.size();
            totalCount += m_computePipelines.size();
            totalCount += m_samplers.size();
            return totalCount;
        }

        inline TL::String ReportLiveResources(bool countOnly)
        {
            TL::String report = "";
            report += countOnly ? TL::String(std::format("Leaked ({}) Images \n", m_images.size())) : ReportResourceStacktrace(m_images);
            report += countOnly ? TL::String(std::format("Leaked ({}) Buffers \n", m_buffers.size())) : ReportResourceStacktrace(m_buffers);
            report += countOnly ? TL::String(std::format("Leaked ({}) ImageViews \n", m_imageViews.size())) : ReportResourceStacktrace(m_imageViews);
            report += countOnly ? TL::String(std::format("Leaked ({}) BufferViews \n", m_bufferViews.size())) : ReportResourceStacktrace(m_bufferViews);
            report += countOnly ? TL::String(std::format("Leaked ({}) BindGroupLayouts \n", m_bindGroupLayouts.size())) : ReportResourceStacktrace(m_bindGroupLayouts);
            report += countOnly ? TL::String(std::format("Leaked ({}) BindGroups \n", m_bindGroups.size())) : ReportResourceStacktrace(m_bindGroups);
            report += countOnly ? TL::String(std::format("Leaked ({}) PipelineLayouts \n", m_pipelineLayouts.size())) : ReportResourceStacktrace(m_pipelineLayouts);
            report += countOnly ? TL::String(std::format("Leaked ({}) GraphicsPipelines \n", m_graphicsPipelines.size())) : ReportResourceStacktrace(m_graphicsPipelines);
            report += countOnly ? TL::String(std::format("Leaked ({}) ComputePipelines \n", m_computePipelines.size())) : ReportResourceStacktrace(m_computePipelines);
            report += countOnly ? TL::String(std::format("Leaked ({}) Samplers \n", m_samplers.size())) : ReportResourceStacktrace(m_samplers);
            return report;
        }

        // clang-format off
        inline Handle<Image>            Register(Handle<Image> handle)              { Register(m_images, handle); return handle; }
        inline void                     Unregister(Handle<Image> handle)            { Unregister(m_images, handle); }
        inline Handle<Buffer>           Register(Handle<Buffer> handle)             { Register(m_buffers, handle); return handle; }
        inline void                     Unregister(Handle<Buffer> handle)           { Unregister(m_buffers, handle); }
        inline Handle<ImageView>        Register(Handle<ImageView> handle)          { Register(m_imageViews, handle); return handle; }
        inline void                     Unregister(Handle<ImageView> handle)        { Unregister(m_imageViews, handle); }
        inline Handle<BufferView>       Register(Handle<BufferView> handle)         { Register(m_bufferViews, handle); return handle; }
        inline void                     Unregister(Handle<BufferView> handle)       { Unregister(m_bufferViews, handle); }
        inline Handle<BindGroupLayout>  Register(Handle<BindGroupLayout> handle)    { Register(m_bindGroupLayouts, handle); return handle; }
        inline void                     Unregister(Handle<BindGroupLayout> handle)  { Unregister(m_bindGroupLayouts, handle); }
        inline Handle<BindGroup>        Register(Handle<BindGroup> handle)          { Register(m_bindGroups, handle); return handle; }
        inline void                     Unregister(Handle<BindGroup> handle)        { Unregister(m_bindGroups, handle); }
        inline Handle<PipelineLayout>   Register(Handle<PipelineLayout> handle)     { Register(m_pipelineLayouts, handle); return handle; }
        inline void                     Unregister(Handle<PipelineLayout> handle)   { Unregister(m_pipelineLayouts, handle); }
        inline Handle<GraphicsPipeline> Register(Handle<GraphicsPipeline> handle)   { Register(m_graphicsPipelines, handle); return handle; }
        inline void                     Unregister(Handle<GraphicsPipeline> handle) { Unregister(m_graphicsPipelines, handle); }
        inline Handle<ComputePipeline>  Register(Handle<ComputePipeline> handle)    { Register(m_computePipelines, handle); return handle; }
        inline void                     Unregister(Handle<ComputePipeline> handle)  { Unregister(m_computePipelines, handle); }
        inline Handle<Sampler>          Register(Handle<Sampler> handle)            { Register(m_samplers, handle); return handle; }
        inline void                     Unregister(Handle<Sampler> handle)          { Unregister(m_samplers, handle); }

        // clang-format on

    private:
        template<typename T>
        using LiveResourceLookup = TL::UnorderedMap<T, Callstack>;

        template<typename T>
        inline T Register(LiveResourceLookup<T>& lookup, T resource)
        {
            RHI_ASSERT(lookup.find(resource) == lookup.end());
            lookup[resource] = CaptureCallstack(3);
            return resource;
        }

        template<typename T>
        inline void Unregister(LiveResourceLookup<T> lookup, T resource)
        {
            RHI_ASSERT(lookup.find(resource) != lookup.end());
            lookup.erase(resource);
        }

        template<typename T>
        inline TL::String ReportResourceStacktrace(LiveResourceLookup<T> lookup)
        {
            auto breakline = "\n=============================================================================\n";
            auto message = std::format("{}{} leak count {} \n", breakline, typeid(T).name(), lookup.size());

            for (auto [handle, stacktrace] : lookup)
            {
                auto stacktraceReport = ReportCallstack(stacktrace);
                message.append(std::format("{}\n", stacktraceReport));
            }

            message.append(std::format("{}", breakline));
            return TL::String{ message };
        }

    private:
        LiveResourceLookup<Handle<Image>> m_images;
        LiveResourceLookup<Handle<Buffer>> m_buffers;
        LiveResourceLookup<Handle<ImageView>> m_imageViews;
        LiveResourceLookup<Handle<BufferView>> m_bufferViews;
        LiveResourceLookup<Handle<BindGroupLayout>> m_bindGroupLayouts;
        LiveResourceLookup<Handle<BindGroup>> m_bindGroups;
        LiveResourceLookup<Handle<PipelineLayout>> m_pipelineLayouts;
        LiveResourceLookup<Handle<GraphicsPipeline>> m_graphicsPipelines;
        LiveResourceLookup<Handle<ComputePipeline>> m_computePipelines;
        LiveResourceLookup<Handle<Sampler>> m_samplers;
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

    void Context::Shutdown()
    {
        for (auto& commandQueue : m_deferCommandQueue)
        {
            for (auto it = commandQueue.rbegin(); it != commandQueue.rend(); it++)
            {
                it->callback();
            }
            commandQueue.clear();
        }

        if (m_resourceTracker->LiveResourcesCount())
        {
            DebugLogWarn(m_resourceTracker->ReportLiveResources(true));
        }

        delete m_resourceTracker;
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

        Flush();
    }

    Ptr<Swapchain> Context::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        RHI_ASSERT(createInfo.imageSize.width > 0 && createInfo.imageSize.height > 0);
        RHI_ASSERT(createInfo.imageCount >= Swapchain::MinImageCount);
        RHI_ASSERT(createInfo.imageCount <= Swapchain::MaxImageCount);
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

    Ptr<CommandPool> Context::CreateCommandPool(CommandPoolFlags flags)
    {
        ZoneScoped;

        return Internal_CreateCommandPool(flags);
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

        PushDeferCommand([this, handle]()
        {
            Internal_DestroyBindGroupLayout(handle);
        });
        m_resourceTracker->Unregister(handle);
    }

    Handle<BindGroup> Context::CreateBindGroup(Handle<BindGroupLayout> bindGroupLayoutHandle, uint32_t bindlessElementsCount)
    {
        ZoneScoped;

        auto handle = Internal_CreateBindGroup(bindGroupLayoutHandle, bindlessElementsCount);
        m_resourceTracker->Register(handle);
        return handle;
    }

    void Context::DestroyBindGroup(Handle<BindGroup> handle)
    {
        ZoneScoped;

        PushDeferCommand([this, handle]()
        {
            Internal_DestroyBindGroup(handle);
        });
        m_resourceTracker->Unregister(handle);
    }

    void Context::UpdateBindGroup(Handle<BindGroup> handle, TL::Span<const ResourceBinding> bindings)
    {
        ZoneScoped;

        Internal_UpdateBindGroup(handle, bindings);
    }

    Handle<PipelineLayout> Context::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        ZoneScoped;

        auto handle = Internal_CreatePipelineLayout(createInfo);
        m_resourceTracker->Register(handle);
        return handle;
    }

    void Context::DestroyPipelineLayout(Handle<PipelineLayout> handle)
    {
        ZoneScoped;

        PushDeferCommand([this, handle]()
        {
            Internal_DestroyPipelineLayout(handle);
        });
        m_resourceTracker->Unregister(handle);
    }

    Handle<GraphicsPipeline> Context::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        ZoneScoped;

        auto handle = Internal_CreateGraphicsPipeline(createInfo);
        m_resourceTracker->Register(handle);
        return handle;
    }

    void Context::DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)
    {
        ZoneScoped;

        PushDeferCommand([this, handle]()
        {
            Internal_DestroyGraphicsPipeline(handle);
        });
        m_resourceTracker->Unregister(handle);
    }

    Handle<ComputePipeline> Context::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        ZoneScoped;

        auto handle = Internal_CreateComputePipeline(createInfo);
        m_resourceTracker->Register(handle);
        return handle;
    }

    void Context::DestroyComputePipeline(Handle<ComputePipeline> handle)
    {
        ZoneScoped;

        PushDeferCommand([this, handle]()
        {
            Internal_DestroyComputePipeline(handle);
        });
        m_resourceTracker->Unregister(handle);
    }

    Handle<Sampler> Context::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        ZoneScoped;

        auto handle = Internal_CreateSampler(createInfo);
        m_resourceTracker->Register(handle);
        return handle;
    }

    void Context::DestroySampler(Handle<Sampler> handle)
    {
        ZoneScoped;

        PushDeferCommand([this, handle]()
        {
            Internal_DestroySampler(handle);
        });
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

        auto handle = Internal_CreateImage(createInfo);
        if (handle.IsSucess())
        {
            m_resourceTracker->Register(handle.GetValue());
        }

        return handle;
    }

    void Context::DestroyImage(Handle<Image> handle)
    {
        ZoneScoped;

        PushDeferCommand([this, handle]()
        {
            Internal_DestroyImage(handle);
        });
        m_resourceTracker->Unregister(handle);
    }

    Result<Handle<Buffer>> Context::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        ZoneScoped;

        RHI_ASSERT(createInfo.usageFlags != BufferUsage::None);
        RHI_ASSERT(createInfo.byteSize != 0);

        auto handle = Internal_CreateBuffer(createInfo);
        if (handle.IsSucess())
        {
            m_resourceTracker->Register(handle.GetValue());
        }

        return handle;
    }

    void Context::DestroyBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;

        PushDeferCommand([this, handle]()
        {
            Internal_DestroyBuffer(handle);
        });
        m_resourceTracker->Unregister(handle);
    }

    Handle<ImageView> Context::CreateImageView(const ImageViewCreateInfo& createInfo)
    {
        ZoneScoped;

        auto handle = Internal_CreateImageView(createInfo);
        m_resourceTracker->Register(handle);
        return handle;
    }

    void Context::DestroyImageView(Handle<ImageView> handle)
    {
        ZoneScoped;

        PushDeferCommand([this, handle]()
        {
            Internal_DestroyImageView(handle);
        });
        m_resourceTracker->Unregister(handle);
    }

    Handle<BufferView> Context::CreateBufferView(const BufferViewCreateInfo& createInfo)
    {
        ZoneScoped;

        auto handle = Internal_CreateBufferView(createInfo);
        m_resourceTracker->Register(handle);
        return handle;
    }

    void Context::DestroyBufferView(Handle<BufferView> handle)
    {
        ZoneScoped;

        PushDeferCommand([this, handle]()
        {
            Internal_DestroyBufferView(handle);
        });
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

    void Context::Flush()
    {
        uint32_t currentFrameIndex = m_frameIndex % 2 != 0;
        auto& commandQueue = m_deferCommandQueue[currentFrameIndex];
        for (auto it = commandQueue.rbegin(); it != commandQueue.rend(); it++)
        {
            it->callback();
        }
        commandQueue.clear();
        m_frameIndex++;
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

    void Context::PushDeferCommand(std::function<void()> command)
    {
        uint32_t currentFrameIndex = m_frameIndex % 2 == 0;
        m_deferCommandQueue[currentFrameIndex].push_back({ m_frameIndex, command });
    }

} // namespace RHI