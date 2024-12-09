#include "RHI/RenderGraph.hpp"

#include "RHI/Device.hpp"
#include "RHI/RenderGraphPass.hpp"
#include "RHI/RenderGraphResources.hpp"

#include <TL/Allocator/Allocator.hpp>
#include <TL/Allocator/Mimalloc.hpp>
#include <TL/Containers.hpp>

#include <tracy/Tracy.hpp>

namespace RHI
{
    RenderGraphImage* RenderGraph::ImportSwapchain(const char* name, Swapchain& swapchain, Format format)
    {
        ZoneScoped;

        auto* image = m_allocator->Construct<RenderGraphImage>(name, swapchain.GetImage(), format);
        m_graphImages.push_back(image);
        m_graphResourcesLookup[name]                = image;
        m_graphImportedSwapchainsLookup[&swapchain] = image;
        return image;
    }

    RenderGraphImage* RenderGraph::ImportImage(const char* name, Handle<Image> image, Format format)
    {
        ZoneScoped;

        auto* importedImage = m_allocator->Construct<RenderGraphImage>(name, image, format);
        m_graphImages.push_back(importedImage);
        m_graphImportedImagesLookup[image] = importedImage;
        m_graphResourcesLookup[name]       = importedImage;
        return importedImage;
    }

    RenderGraphBuffer* RenderGraph::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        ZoneScoped;

        auto* importedBuffer = m_allocator->Construct<RenderGraphBuffer>(name, buffer);
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

    Pass* RenderGraph::AddPass(const PassCreateInfo& createInfo)
    {
        ZoneScoped;

        auto* pass = m_tempAllocator.Construct<Pass>(createInfo, &m_tempAllocator);
        m_graphPasses.push_back(pass);
        pass->m_onSetupCallback(*this, *pass);
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

    void RenderGraph::UseRenderTarget(Pass& pass, const RenderTargetInfo& renderTargetInfo)
    {
        ZoneScoped;

        FormatInfo formatInfo = GetFormatInfo(renderTargetInfo.attachment->GetFormat());
        ExpandRenderGraphResourceUsage(*renderTargetInfo.attachment, GetImageUsage(formatInfo));

        if (formatInfo.hasDepth || formatInfo.hasStencil)
            pass.m_depthStencilAttachment = renderTargetInfo;
        else
            pass.m_colorAttachments.push_back(renderTargetInfo);

        /// @todo: convert load ops to access
        UseImage(pass, renderTargetInfo.attachment, ImageUsage::Color, PipelineStage::ColorAttachmentOutput, Access::ReadWrite);
        if (renderTargetInfo.resolveAttachment && renderTargetInfo.resolveMode != ResolveMode::None)
        {
            ExpandRenderGraphResourceUsage(*renderTargetInfo.attachment, ImageUsage::Resolve);
            UseImage(pass, renderTargetInfo.resolveAttachment, ImageUsage::Color, PipelineStage::Transfer, Access::Write);
        }
    }

    void RenderGraph::BeginFrame()
    {
        ZoneScoped;

        m_frameIndex++;
        m_graphPasses.clear();
        m_tempAllocator.Collect();
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
    }

    void RenderGraph::EndFrame()
    {
        ZoneScoped;

        static bool isInit = false;
        if (isInit == false)
        {
            Compile();
            isInit = true;
        }

        for (auto [swapchain, graphImage] : m_graphImportedSwapchainsLookup)
        {
            graphImage->m_handle.asImage  = swapchain->GetImage();
            auto presentAccess            = m_tempAllocator.Allocate<RenderGraphResourceTransition>();
            presentAccess->pass           = nullptr;
            presentAccess->next           = nullptr;
            presentAccess->prev           = graphImage->GetLastAccess();
            presentAccess->resource       = graphImage;
            presentAccess->asImage.usage  = ImageUsage::_SwapchainPresent;
            presentAccess->asImage.stage  = PipelineStage::BottomOfPipe;
            presentAccess->asImage.access = Access::None;
            graphImage->PushAccess(presentAccess);
        }

        auto [swapchain, resource] = *m_graphImportedSwapchainsLookup.begin();

        {
            // For debug and testing only
            auto& group = m_orderedPassGroups[(int)QueueType::Graphics];
            group.push_back({
                .passList                = m_graphPasses,
                .asyncQueuesDependencies = {},
                .swapchainToAcquire      = swapchain,
                .swapchainToRelease      = swapchain,
            });
        }

        OnGraphExecutionBegin();
        for (auto& group : m_orderedPassGroups[(uint32_t)QueueType::Graphics])
        {
            ExecutePassGroup(group, QueueType::Graphics);
        }
        for (auto& group : m_orderedPassGroups[(uint32_t)QueueType::Compute])
        {
            ExecutePassGroup(group, QueueType::Compute);
        }
        for (auto& group : m_orderedPassGroups[(uint32_t)QueueType::Transfer])
        {
            ExecutePassGroup(group, QueueType::Transfer);
        }
        OnGraphExecutionEnd();
    }

    void RenderGraph::Compile()
    {
        InitializeTransientResources();

        for (const auto& pass : m_graphPasses)
        {
            if (pass->m_onCompileCallback)
            {
                pass->m_onCompileCallback(*this, *pass);
            }
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