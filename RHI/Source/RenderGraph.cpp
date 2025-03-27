#include "RHI/RenderGraph.hpp"

#include "RHI/Device.hpp"
#include "RHI/RenderGraphExecuteGroup.hpp"
#include "RHI/RenderGraphPass.hpp"
#include "RHI/RenderGraphResources.hpp"

#include <TL/Allocator/Allocator.hpp>
#include <TL/Allocator/Mimalloc.hpp>
#include <TL/Containers.hpp>

#include <tracy/Tracy.hpp>

namespace RHI
{
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
        auto* pass = CreatePass(createInfo);
        m_graphPasses.push_back(pass);
        return pass;
    }

    void RenderGraph::UseImage(Pass& pass, RenderGraphImage* image, ImageUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access)
    {
        ZoneScoped;
        pass.AddTransition(m_tempAllocator, *image, usage, stage, access, {});
    }

    void RenderGraph::UseBuffer(Pass& pass, RenderGraphBuffer* buffer, BufferUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access)
    {
        ZoneScoped;
        pass.AddTransition(m_tempAllocator, *buffer, usage, stage, access, {});
    }

    void RenderGraph::UseColorAttachment(Pass& pass, const ColorRGAttachment& attachment)
    {
        ZoneScoped;

        ExtendResourceUsage(*attachment.view, ImageUsage::Color);
        pass.m_colorAttachments.push_back(attachment);
        UseImage(pass, attachment.view, ImageUsage::Color, PipelineStage::ColorAttachmentOutput, Access::ReadWrite);

        if (attachment.resolveView && attachment.resolveMode != ResolveMode::None)
        {
            ExtendResourceUsage(*attachment.resolveView, ImageUsage::Resolve);
            UseImage(pass, attachment.resolveView, ImageUsage::Resolve, PipelineStage::Transfer, LoadStoreToAccess(attachment.loadOp, attachment.storeOp));
        }
    }

    void RenderGraph::UseDepthStencilAttachment(Pass& pass, const DepthStencilRGAttachment& attachment)
    {
        // TL_ASSERT(pass.m_depthStencilAttachment.has_value() == false);

        FormatInfo formatInfo = GetFormatInfo(attachment.view->GetFormat());

        // TODO: handle stencil
        // auto       usage      = GetImageUsage(formatInfo);
        // auto usage = attachment.view->GetImageUsage();

        ExtendResourceUsage(*attachment.view, ImageUsage::Depth);
        pass.m_depthStencilAttachment = attachment;
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
            lastPass->AddSwapchainPresentBarrier(*m_device, *swapchain, *graphResource->GetLastAccess());
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

    ColorAttachment RenderGraph::GetColorAttachment(const ColorRGAttachment& attachment) const
    {
        return ColorAttachment{
            .view        = attachment.view->GetImage(),
            .loadOp      = attachment.loadOp,
            .storeOp     = attachment.storeOp,
            .clearValue  = attachment.clearValue,
            .resolveMode = attachment.resolveMode,
            .resolveView = attachment.resolveView->GetImage(),
        };
    }

    DepthStencilAttachment RenderGraph::GetDepthStencilAttachment(const DepthStencilRGAttachment& attachment) const
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

} // namespace RHI