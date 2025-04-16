#include "RHI/RenderGraph.hpp"

#include "RHI/Device.hpp"

#include <TL/Allocator/Allocator.hpp>
#include <TL/Allocator/Mimalloc.hpp>
#include <TL/Containers.hpp>

#include <tracy/Tracy.hpp>

namespace RHI
{


    inline static RHI::Access LoadStoreToAccess(LoadOperation loadOp, StoreOperation storeOp)
    {
        if (loadOp == LoadOperation::Load && storeOp == StoreOperation::Store)
        {
            return RHI::Access::ReadWrite;
        }
        else if (loadOp == LoadOperation::Load && storeOp != StoreOperation::Store)
        {
            return RHI::Access::Read;
        }
        else
        {
            return RHI::Access::Write;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    RenderGraphResource::RenderGraphResource(const char* name, Type type)
        : m_name(name)
        , m_first(nullptr)
        , m_last(nullptr)
        , m_type(type)
        , m_format(Format::Unknown)
        , m_handle(NullHandle)
        , m_usage({})
    {
        m_handle.asImage = {};
        m_usage.asImage  = {};
    }

    void RenderGraphResource::PushAccess(GraphTransition* access)
    {
        if (!m_last)
        {
            m_first = m_last = access;
        }
        else
        {
            access->prev = m_last;
            m_last->next = access;
            m_last       = access;
        }

        switch (access->resource->m_type)
        {
        case Type::Image:
            m_usage.asImage |= static_cast<ImageGraphTransition*>(access)->usage;
            break;
        case Type::Buffer:
            m_usage.asBuffer |= static_cast<BufferGraphTransition*>(access)->usage;
            break;
        }
    }

    RenderGraphImage::RenderGraphImage(const char* name, Handle<Image> image, Format format)
        : RenderGraphResource(name, Type::Image)
    {
        m_handle.asImage = image;
        m_format         = format;
    }

    RenderGraphImage::RenderGraphImage(const char* name, Format format)
        : RenderGraphResource(name, Type::Image)
    {
        m_format = format;
    }

    RenderGraphBuffer::RenderGraphBuffer(const char* name, Handle<Buffer> buffer)
        : RenderGraphResource(name, Type::Buffer)
    {
        m_handle.asBuffer = buffer;
    }

    RenderGraphBuffer::RenderGraphBuffer(const char* name)
        : RenderGraphResource(name, Type::Buffer)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    Pass::Pass(const PassCreateInfo& createInfo, TL::IAllocator* allocator)
        : m_allocator(allocator)
        , m_name(createInfo.name)
        , m_queueType(createInfo.queue)
        , m_size(createInfo.size)
        , m_onSetupCallback(createInfo.setupCallback)
        , m_onCompileCallback(createInfo.compileCallback)
        , m_onExecuteCallback(createInfo.executeCallback)
    {
    }

    Pass::~Pass()
    {
        // default
    }

    TL::Span<GraphTransition* const> Pass::GetTransitions() const
    {
        return m_transitions;
    }

    TL::Span<ImageGraphTransition* const> Pass::GetImageTransitions() const
    {
        return m_imageTransitions;
    }

    TL::Span<BufferGraphTransition* const> Pass::GetBufferTransitions() const
    {
        return m_bufferTransitions;
    }

    ColorAttachment GetColorAttachment(const ColorRGAttachment& attachment)
    {
        return ColorAttachment{
            .view        = attachment.view->GetImage(),
            .loadOp      = attachment.loadOp,
            .storeOp     = attachment.storeOp,
            .clearValue  = attachment.clearValue,
            .resolveMode = attachment.resolveMode,
            .resolveView = attachment.resolveView ? attachment.resolveView->GetImage() : NullHandle,
        };
    }

    DepthStencilAttachment GetDepthStencilAttachment(const DepthStencilRGAttachment& attachment)
    {
        return DepthStencilAttachment{
            .view           = attachment.view->GetImage(),
            .depthLoadOp    = attachment.depthLoadOp,
            .depthStoreOp   = attachment.depthStoreOp,
            .stencilLoadOp  = attachment.stencilLoadOp,
            .stencilStoreOp = attachment.stencilStoreOp,
            .clearValue     = attachment.clearValue,
        };
    }

    void Pass::Execute(CommandList& commandList)
    {
        PrepareBarriers();

        commandList.PushDebugMarker(m_name, {});

        // Prologue barriers
        auto prologueMemoryBarriers = GetMemoryBarriers(BarrierSlot_Prologue);
        auto prologueImageBarriers  = GetImageBarriers(BarrierSlot_Prologue);
        auto prologueBufferBarriers = GetBufferBarriers(BarrierSlot_Prologue);
        commandList.AddPipelineBarrier(prologueMemoryBarriers, prologueImageBarriers, prologueBufferBarriers);

        // Begin render pass if graphics queue
        if (m_queueType == QueueType::Graphics)
        {
            TL::Vector<ColorAttachment>          colorAttachments;
            TL::Optional<DepthStencilAttachment> depthStencilAttachment;

            for (auto attachment : m_renderPass.m_colorAttachments)
            {
                if (attachment.view != nullptr)
                    colorAttachments.push_back(GetColorAttachment(attachment));
            }

            if (m_renderPass.m_depthStencilAttachment.has_value())
            {
                depthStencilAttachment = GetDepthStencilAttachment(m_renderPass.m_depthStencilAttachment.value());
            }

            RenderPassBeginInfo beginInfo{
                .size                   = m_size,
                .offset                 = {},
                .colorAttachments       = colorAttachments,
                .depthStencilAttachment = depthStencilAttachment,
            };
            commandList.BeginRenderPass(beginInfo);
        }

        // Execute pass callback
        m_onExecuteCallback(commandList);

        // End render pass if graphics queue
        if (m_queueType == QueueType::Graphics)
        {
            commandList.EndRenderPass();
        }

        // Epilogue barriers
        auto epilogueMemoryBarriers = GetMemoryBarriers(BarrierSlot_Epilogue);
        auto epilogueImageBarriers  = GetImageBarriers(BarrierSlot_Epilogue);
        auto epilogueBufferBarriers = GetBufferBarriers(BarrierSlot_Epilogue);
        commandList.AddPipelineBarrier(epilogueMemoryBarriers, epilogueImageBarriers, epilogueBufferBarriers);

        commandList.PopDebugMarker();
    }

    void Pass::UseResource(RenderGraphImage& resource, ImageSubresourceRange subresourceRange, ImageUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access)
    {
        auto transition              = m_allocator->Allocate<ImageGraphTransition>();
        transition->pass             = this;
        transition->prev             = resource.GetLastAccess();
        transition->next             = nullptr;
        transition->resource         = &resource;
        transition->usage            = usage;
        transition->stage            = stage;
        transition->access           = access;
        transition->subresourceRange = subresourceRange;
        resource.PushAccess(transition);
        m_imageTransitions.push_back(transition);
    }

    void Pass::UseResource(RenderGraphBuffer& resource, BufferSubregion subregion, BufferUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access)
    {
        auto transition       = m_allocator->Allocate<BufferGraphTransition>();
        transition->pass      = this;
        transition->prev      = resource.GetLastAccess();
        transition->next      = nullptr;
        transition->resource  = &resource;
        transition->usage     = usage;
        transition->stage     = stage;
        transition->access    = access;
        transition->subregion = subregion;
        resource.PushAccess(transition);
        m_bufferTransitions.push_back(transition);
    }

    void Pass::PresentSwapchain(RenderGraphImage& resource)
    {
        auto transition              = m_allocator->Allocate<ImageGraphTransition>();
        transition->pass             = this;
        transition->prev             = resource.GetLastAccess();
        transition->next             = nullptr;
        transition->resource         = &resource;
        transition->usage            = ImageUsage::Present;
        transition->stage            = PipelineStage::ColorAttachmentOutput;
        transition->access           = Access::ReadWrite;
        transition->subresourceRange = {};
        resource.PushAccess(transition);
        m_imageTransitions.push_back(transition);

        {
            auto& imageBarrier        = m_imageBarriers[BarrierSlot_Epilogue].emplace_back();
            imageBarrier.image        = resource.GetImage();
            imageBarrier.srcState     = {transition->usage, transition->stage, transition->access};
            imageBarrier.dstState     = {transition->usage, transition->stage, transition->access};
            imageBarrier.subresources = {};
        }
    }

    void Pass::AddRenderTarget(const ColorRGAttachment& attachment)
    {
        m_renderPass.m_colorAttachments.push_back(attachment);
        UseResource(*attachment.view, {}, ImageUsage::Color, PipelineStage::ColorAttachmentOutput, Access::ReadWrite);
    }

    void Pass::AddRenderTarget(const DepthStencilRGAttachment& attachment)
    {
        m_renderPass.m_depthStencilAttachment = attachment;
        UseResource(*attachment.view, {}, ImageUsage::DepthStencil, PipelineStage::EarlyFragmentTests | PipelineStage::LateFragmentTests, Access::ReadWrite);
    }

    TL::Span<const BarrierInfo> Pass::GetMemoryBarriers(BarrierSlot slot) const
    {
        return m_memoryBarriers[slot];
    }

    TL::Span<const ImageBarrierInfo> Pass::GetImageBarriers(BarrierSlot slot) const
    {
        return m_imageBarriers[slot];
    }

    TL::Span<const BufferBarrierInfo> Pass::GetBufferBarriers(BarrierSlot slot) const
    {
        return m_bufferBarriers[slot];
    }

    void Pass::PrepareBarriers()
    {
        // Prepare image barriers
        for (auto imageTransition : m_imageTransitions)
        {
            auto  image               = (RenderGraphImage*)imageTransition->resource;
            auto& imageBarrier        = m_imageBarriers[BarrierSlot_Prologue].emplace_back();
            imageBarrier.image        = image->GetImage();
            imageBarrier.srcState     = {imageTransition->usage, imageTransition->stage, imageTransition->access};
            imageBarrier.dstState     = {imageTransition->usage, imageTransition->stage, imageTransition->access};
            imageBarrier.subresources = imageTransition->subresourceRange;
        }

        // Prepare buffer barriers
        for (auto bufferTransition : m_bufferTransitions)
        {
            auto buffer = (RenderGraphBuffer*)bufferTransition->resource;

            auto& bufferBarrier     = m_bufferBarriers[BarrierSlot_Prologue].emplace_back();
            bufferBarrier.buffer    = buffer->GetBuffer();
            bufferBarrier.srcState  = {bufferTransition->usage, bufferTransition->stage, bufferTransition->access};
            bufferBarrier.dstState  = {bufferTransition->usage, bufferTransition->stage, bufferTransition->access};
            bufferBarrier.subregion = bufferTransition->subregion;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    struct GraphNode
    {
        Pass*                                  m_pass;
        QueueType                              m_queueType;
        TL::Vector<GraphNode*, TL::IAllocator> m_consumers;
        uint32_t                               m_ssis[AsyncQueuesCount];
        uint32_t                               m_depth;
    };

    using Graph = TL::Vector<GraphNode*, TL::IAllocator>;

    // inline static void TopologicalSort(Graph& graph, GraphNode* node, TL::Vector<GraphNode*, TL::IAllocator>& sorted)
    // {
    //     for (auto consumer : node->m_consumers)
    //     {
    //         TopologicalSort(graph, consumer, sorted);
    //     }6
    //     sorted.push_back(node);
    // }

    // inline static void BuildDepdenencyLevelsPerQueue(Graph& graph, TL::Span<TL::Vector<RenderGraphExecuteGroup>> groups)
    // {
    //     for (auto node : graph)
    //     {
    //         for (auto consumer : node->m_consumers)
    //         {
    //             for (uint32_t i = 0; i < AsyncQueuesCount; i++)
    //             {
    //                 // node->m_ssis[i] = std::max(node->m_ssis[i], consumer->m_ssis[i] + 1);
    //                 // update depth
    //                 if (node->m_queueType == consumer->m_queueType)
    //                 {
    //                     node->m_depth = std::max(node->m_depth, consumer->m_depth + 1);
    //                 }
    //                 else
    //                 {
    //                     node->m_depth = std::max(node->m_depth, consumer->m_depth);
    //                 }
    //             }
    //         }
    //     }
    // }

    // inline static void RecordPassBarriers(IDevice& device, CompiledPass& pass)
    // {
    //     for (const auto& resourceTransition : pass.GetRenderGraphResourceTransitions())
    //     {
    //         auto [srcStageMask, srcAccessMask, srcLayout, srcQfi] = GetBarrierStage(resourceTransition->prev);
    //         auto [dstStageMask, dstAccessMask, dstLayout, dstQfi] = GetBarrierStage(resourceTransition);
    //         if (resourceTransition->resource->GetType() == RenderGraphResource::Type::Image)
    //         {
    //             auto imageTransition = (RenderGraphImage*)resourceTransition->resource;
    //             auto image           = device.m_imageOwner.Get(imageTransition->GetImage());
    //             pass.PushPassBarrier(
    //                 BarrierSlot::Prilogue,
    //                 {
    //                     .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    //                     .pNext               = nullptr,
    //                     .srcStageMask        = srcStageMask,
    //                     .srcAccessMask       = srcAccessMask,
    //                     .dstStageMask        = dstStageMask,
    //                     .dstAccessMask       = dstAccessMask,
    //                     .oldLayout           = srcLayout,
    //                     .newLayout           = dstLayout,
    //                     .srcQueueFamilyIndex = srcQfi == dstQfi ? VK_QUEUE_FAMILY_IGNORED : srcQfi,
    //                     .dstQueueFamilyIndex = srcQfi == dstQfi ? VK_QUEUE_FAMILY_IGNORED : dstQfi,
    //                     .image               = image->handle,
    //                     .subresourceRange    = GetAccessedSubresourceRange(*resourceTransition),
    //                 });
    //         }
    //         else
    //         {
    //             auto bufferTransition = (RenderGraphBuffer*)resourceTransition->resource;
    //             auto buffer           = device.m_bufferOwner.Get(bufferTransition->GetBuffer());
    //             pass.PushPassBarrier(
    //                 BarrierSlot::Prilogue,
    //                 {
    //                     .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
    //                     .pNext               = nullptr,
    //                     .srcStageMask        = srcStageMask,
    //                     .srcAccessMask       = srcAccessMask,
    //                     .dstStageMask        = dstStageMask,
    //                     .dstAccessMask       = dstAccessMask,
    //                     .srcQueueFamilyIndex = srcQfi == dstQfi ? VK_QUEUE_FAMILY_IGNORED : srcQfi,
    //                     .dstQueueFamilyIndex = srcQfi == dstQfi ? VK_QUEUE_FAMILY_IGNORED : dstQfi,
    //                     .buffer              = buffer->handle,
    //                     .offset              = resourceTransition->asBuffer.subregion.offset,
    //                     .size                = resourceTransition->asBuffer.subregion.size,
    //                 });
    //         }
    //     }
    // }

    RenderGraphImage* RenderGraph::ImportSwapchain(const char* name, Swapchain& swapchain, Format format)
    {
        ZoneScoped;

        auto* image       = m_allocator->Construct<RenderGraphImage>(name, swapchain.GetImage(), format);
        image->isImported = true;

        m_graphImages.push_back(image);
        m_graphResourcesLookup[name]                = image;
        m_graphImportedSwapchainsLookup[&swapchain] = image;
        return image;
    }

    RenderGraphImage* RenderGraph::ImportImage(const char* name, Handle<Image> image, Format format)
    {
        ZoneScoped;

        auto* importedImage       = m_allocator->Construct<RenderGraphImage>(name, image, format);
        importedImage->isImported = true;

        m_graphImages.push_back(importedImage);
        m_graphImportedImagesLookup[image] = importedImage;
        m_graphResourcesLookup[name]       = importedImage;
        return importedImage;
    }

    RenderGraphBuffer* RenderGraph::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        ZoneScoped;

        auto* importedBuffer       = m_allocator->Construct<RenderGraphBuffer>(name, buffer);
        importedBuffer->isImported = true;

        m_graphBuffers.push_back(importedBuffer);
        m_graphImportedBuffersLookup[buffer] = importedBuffer;
        m_graphResourcesLookup[name]         = importedBuffer;
        return importedBuffer;
    }

    RenderGraphImage* RenderGraph::CreateImage(const ImageCreateInfo& createInfo)
    {
        ZoneScoped;

        auto* image = m_allocator->Construct<RenderGraphImage>(createInfo.name, createInfo.format);
        m_graphImages.push_back(image);
        m_graphResourcesLookup[createInfo.name] = image;
        m_graphTransientImagesLookup[image]     = createInfo;
        return image;
    }

    RenderGraphBuffer* RenderGraph::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        ZoneScoped;

        auto* buffer = m_allocator->Construct<RenderGraphBuffer>(createInfo.name);
        m_graphBuffers.push_back(buffer);
        m_graphResourcesLookup[createInfo.name] = buffer;
        m_graphTransientBuffersLookup[buffer]   = createInfo;
        return buffer;
    }

    void RenderGraph::DestroyImage(RenderGraphImage* image)
    {
        ZoneScoped;
        if (image->isImported == false)
        {
            if (auto handle = image->GetImage())
            {
                m_device->DestroyImage(handle);
            }
        }
        m_allocator->Destruct(image);
    }

    void RenderGraph::DestroyBuffer(RenderGraphBuffer* buffer)
    {
        ZoneScoped;
        if (buffer->isImported == false)
        {
            if (auto handle = buffer->GetBuffer())
            {
                m_device->DestroyBuffer(handle);
            }
        }
        m_allocator->Destruct(buffer);
    }

    Pass* RenderGraph::AddPass(const PassCreateInfo& createInfo)
    {
        ZoneScoped;
        auto* pass = new Pass(createInfo, &m_tempAllocator);
        m_graphPasses.push_back(pass);
        return pass;
    }

    void RenderGraph::UseImage(Pass& pass, RenderGraphImage* image, ImageUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access)
    {
        ZoneScoped;
        pass.UseResource(*image, {}, usage, stage, access);
    }

    void RenderGraph::UseBuffer(Pass& pass, RenderGraphBuffer* buffer, BufferUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access)
    {
        ZoneScoped;
        pass.UseResource(*buffer, {}, usage, stage, access);
    }

    void RenderGraph::UseColorAttachment(Pass& pass, const ColorRGAttachment& attachment)
    {
        ZoneScoped;

        ExtendResourceUsage(*attachment.view, ImageUsage::Color);
        pass.AddRenderTarget(attachment);
        UseImage(pass, attachment.view, ImageUsage::Color, PipelineStage::ColorAttachmentOutput, Access::ReadWrite);

        if (attachment.resolveView && attachment.resolveMode != ResolveMode::None)
        {
            ExtendResourceUsage(*attachment.resolveView, ImageUsage::Resolve);
            UseImage(pass, attachment.resolveView, ImageUsage::Resolve, PipelineStage::Transfer, LoadStoreToAccess(attachment.loadOp, attachment.storeOp));
        }
    }

    void RenderGraph::UseDepthStencilAttachment(Pass& pass, const DepthStencilRGAttachment& attachment)
    {
        FormatInfo formatInfo = GetFormatInfo(attachment.view->GetFormat());

        // TODO: handle stencil
        // auto       usage      = GetImageUsage(formatInfo);
        // auto usage = attachment.view->GetImageUsage();

        ExtendResourceUsage(*attachment.view, ImageUsage::Depth);
        // pass. = attachment;
        UseImage(pass, attachment.view, ImageUsage::Depth, PipelineStage::EarlyFragmentTests, LoadStoreToAccess(attachment.depthLoadOp, attachment.depthStoreOp) | LoadStoreToAccess(attachment.stencilLoadOp, attachment.stencilStoreOp));
    }

    void RenderGraph::BeginFrame(ImageSize2D frameSize)
    {
        ZoneScoped;

        m_frameIndex++;
        m_graphPasses.clear();
        m_tempAllocator.Collect();

        m_graphPasses.clear();
        for (auto& group : m_orderedPassGroups)
            group.clear();

        for (auto graphImage : m_graphImages)
        {
            graphImage->m_first = nullptr;
            graphImage->m_last  = nullptr;
        }
        for (auto graphBuffer : m_graphBuffers)
        {
            graphBuffer->m_first = nullptr;
            graphBuffer->m_last  = nullptr;
        }
        for (auto [swapchain, graphResource] : m_graphImportedSwapchainsLookup)
        {
            graphResource->m_first = nullptr;
            graphResource->m_last  = nullptr;
        }

        if (m_frameSize != frameSize)
        {
            m_frameSize = frameSize;
            m_state     = GraphState::Invalid;
        }
    }

    void RenderGraph::EndFrame()
    {
        ZoneScoped;

        static constexpr uint32_t MaxFramesInFlight = 2;
        static uint32_t           currentFrameIndex = 0;

        for (const auto& pass : m_graphPasses)
        {
            pass->m_onSetupCallback(*this, *pass);
        }

        if (m_state == GraphState::Invalid)
        {
            CleanupResources();
            Compile();
        }

        auto& group = m_orderedPassGroups[(uint32_t)QueueType::Graphics].emplace_back(m_tempAllocator);
        for (auto pass : m_graphPasses)
        {
            group.AddPass(*pass);
            pass->m_group = &group;
        }

        for (auto [swapchain, graphResource] : m_graphImportedSwapchainsLookup)
        {
            auto firstPass  = graphResource->GetFirstPass();
            auto lastPass   = graphResource->GetLastPass();
            auto firstGroup = firstPass->GetExecuteGroup();
            auto lastGroup  = lastPass->GetExecuteGroup();

            graphResource->m_handle.asImage = swapchain->GetImage();

            firstGroup->WaitForSwapchain(*swapchain, PipelineStage::TopOfPipe);
            lastPass->PresentSwapchain(*graphResource);
            lastGroup->SignalSwapchainPresent(*swapchain, PipelineStage::BottomOfPipe);
        }

        OnGraphExecutionBegin();
        for (auto& currentGroup : m_orderedPassGroups[(uint32_t)QueueType::Graphics])
        {
            [[maybe_unused]] auto newTimelineValue = ExecutePassGroup(currentGroup, QueueType::Graphics);
        }
        for (auto& currentGroup : m_orderedPassGroups[(uint32_t)QueueType::Compute])
        {
            [[maybe_unused]] auto newTimelineValue = ExecutePassGroup(currentGroup, QueueType::Compute);
        }
        for (auto& currentGroup : m_orderedPassGroups[(uint32_t)QueueType::Transfer])
        {
            [[maybe_unused]] auto newTimelineValue = ExecutePassGroup(currentGroup, QueueType::Transfer);
        }
        OnGraphExecutionEnd();

        for (auto [swapchain, _] : m_graphImportedSwapchainsLookup)
        {
            [[maybe_unused]] auto res = swapchain->Present();
        }
        currentFrameIndex = (currentFrameIndex + 1) % MaxFramesInFlight;
    }

    void RenderGraph::Compile()
    {
        // for (const auto& pass : m_graphPasses)
        // {
        //     pass->m_onSetupCallback(*this, *pass);
        // }

        InitializeTransientResources();

        for (const auto& pass : m_graphPasses)
        {
            if (pass->m_onCompileCallback)
            {
                pass->m_onCompileCallback(*this, *pass);
            }
        }

        m_state = GraphState::Compiled;
    }

    void RenderGraph::CleanupResources()
    {
        for (auto transientImage : m_graphTransientImagesLookup)
        {
            if (auto image = transientImage.first->m_handle.asImage)
                m_device->DestroyImage(image);
        }
        for (auto transientBuffer : m_graphTransientBuffersLookup)
        {
            if (auto buffer = transientBuffer.first->m_handle.asBuffer)
                m_device->DestroyBuffer(buffer);
        }
    }

    void RenderGraph::InitializeTransientResources()
    {
        for (auto graphImage : m_graphImages)
        {
            if (m_graphTransientImagesLookup.find(graphImage) == m_graphTransientImagesLookup.end())
                continue;

            ImageCreateInfo imageCI      = m_graphTransientImagesLookup[graphImage];
            imageCI.usageFlags           = graphImage->GetImageUsage();
            graphImage->m_handle.asImage = m_device->CreateImage(imageCI).GetValue();
        }
        for (auto graphBuffer : m_graphBuffers)
        {
            if (m_graphTransientBuffersLookup.find(graphBuffer) == m_graphTransientBuffersLookup.end())
                continue;

            BufferCreateInfo bufferCI      = m_graphTransientBuffersLookup[graphBuffer];
            bufferCI.usageFlags            = graphBuffer->GetBufferUsage();
            graphBuffer->m_handle.asBuffer = m_device->CreateBuffer(bufferCI).GetValue();
        }
    }

    void RenderGraph::OnGraphExecutionBegin()
    {
    }

    void RenderGraph::OnGraphExecutionEnd()
    {
    }

    uint64_t RenderGraph::ExecutePassGroup(const RenderGraphExecuteGroup& group, QueueType queueType)
    {
        auto commandList = m_device->CreateCommandList({.queueType = queueType});

        commandList->Begin();
        for (auto pass : group.GetPassList())
        {
            pass->Execute(*commandList);
        }
        commandList->End();

        QueueSubmitInfo queueSubmitInfo{
            .queueType            = queueType,
            .commandLists         = {commandList},
            .signalStage          = PipelineStage::BottomOfPipe,
            .m_swapchainToAcquire = group.GetSwapchainToWait().swapchain,
            .m_swapchainToSignal  = group.GetSwapchainToSignal().swapchain,
        };

        return m_device->QueueSubmit(queueSubmitInfo);
    }

} // namespace RHI