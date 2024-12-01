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
        auto* image = m_allocator->Construct<RenderGraphImage>(name, swapchain.GetImage(), format);
        m_graphImages.push_back(image);
        m_graphResourcesLookup[name]                = image;
        m_graphImportedSwapchainsLookup[&swapchain] = image;
        return image;
    }

    RenderGraphImage* RenderGraph::ImportImage(const char* name, Handle<Image> image, Format format)
    {
        auto* importedImage = m_allocator->Construct<RenderGraphImage>(name, image, format);
        m_graphImages.push_back(importedImage);
        m_graphImportedImagesLookup[image] = importedImage;
        m_graphResourcesLookup[name]       = importedImage;
        return importedImage;
    }

    RenderGraphBuffer* RenderGraph::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        auto* importedBuffer = m_allocator->Construct<RenderGraphBuffer>(name, buffer);
        m_graphBuffers.push_back(importedBuffer);
        m_graphImportedBuffersLookup[buffer] = importedBuffer;
        m_graphResourcesLookup[name]         = importedBuffer;
        return importedBuffer;
    }

    RenderGraphImage* RenderGraph::CreateImage(const ImageCreateInfo& createInfo)
    {
        auto* image = m_allocator->Construct<RenderGraphImage>(createInfo.name, createInfo.format);
        m_graphImages.push_back(image);
        m_graphResourcesLookup[createInfo.name] = image;
        m_graphTransientImagesLookup[image]     = createInfo;
        return image;
    }

    RenderGraphBuffer* RenderGraph::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        auto* buffer = m_allocator->Construct<RenderGraphBuffer>(createInfo.name);
        m_graphBuffers.push_back(buffer);
        m_graphResourcesLookup[createInfo.name] = buffer;
        m_graphTransientBuffersLookup[buffer]   = createInfo;
        return buffer;
    }

    Pass* RenderGraph::AddPass(const PassCreateInfo& createInfo)
    {
        auto* pass = m_allocator->Construct<Pass>(createInfo);
        m_graphPasses.push_back(pass);
        pass->m_onSetupCallback(*this, *pass);
        return pass;
    }

    void RenderGraph::UseImage(Pass& pass, RenderGraphImage* image, ImageUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access)
    {
        auto* resourceAccess           = pass.AddResourceAccess(m_tempAllocator);
        resourceAccess->type           = RenderGraphResourceAccessType::Image;
        resourceAccess->asImage.image  = image;
        resourceAccess->asImage.usage  = usage;
        resourceAccess->asImage.stage  = stage;
        resourceAccess->asImage.access = access;
        image->m_usage.asImage |= usage;
        image->PushAccess(resourceAccess);
    }

    void RenderGraph::UseBuffer(Pass& pass, RenderGraphBuffer* buffer, BufferUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access)
    {
        auto* resourceAccess            = pass.AddResourceAccess(m_tempAllocator);
        resourceAccess->type            = RenderGraphResourceAccessType::Buffer;
        resourceAccess->asBuffer.buffer = buffer;
        resourceAccess->asBuffer.usage  = usage;
        resourceAccess->asBuffer.stage  = stage;
        resourceAccess->asBuffer.access = access;
        buffer->m_usage.asBuffer |= usage;
        buffer->PushAccess(resourceAccess);
    }

    void RenderGraph::UseRenderTarget(Pass& pass, const RenderTargetInfo& renderTargetInfo)
    {
        auto formatInfo = GetFormatInfo(renderTargetInfo.attachment->GetFormat());
        if (formatInfo.hasDepth || formatInfo.hasStencil)
        {
            renderTargetInfo.attachment->m_usage.asImage |= ImageUsage::Depth;
            pass.m_depthStencilAttachment = renderTargetInfo;
        }
        else
        {
            renderTargetInfo.attachment->m_usage.asImage |= ImageUsage::Color;
            pass.m_colorAttachments.push_back(renderTargetInfo);
        }

        auto* resourceAccess           = pass.AddResourceAccess(m_tempAllocator);
        resourceAccess->type           = RenderGraphResourceAccessType::RenderTarget;
        resourceAccess->asRenderTarget = renderTargetInfo;
        renderTargetInfo.attachment->PushAccess(resourceAccess);
        if (renderTargetInfo.resolveAttachment)
        {
            auto* resolveResourceAccess    = pass.AddResourceAccess(m_tempAllocator);
            resolveResourceAccess->type    = RenderGraphResourceAccessType::Resolve;
            resourceAccess->asRenderTarget = renderTargetInfo;
            renderTargetInfo.resolveAttachment->PushAccess(resolveResourceAccess);
            TL_UNREACHABLE();
        }
    }

    Handle<Image> RenderGraph::GetImage(RenderGraphImage image) const
    {
        return image.GetImage();
    }

    Handle<Buffer> RenderGraph::GetBuffer(RenderGraphBuffer buffer) const
    {
        return buffer.GetBuffer();
    }

    void RenderGraph::BeginFrame()
    {
        m_frameIndex++;
        m_graphPasses.clear();
        // m_tempAllocator.Collect();
    }

    void RenderGraph::EndFrame()
    {
        static bool isInit = false;
        if (isInit == false)
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
            for (const auto& pass : m_graphPasses)
            {
                pass->m_onCompileCallback(*this, *pass);
            }
            isInit = true;
        }

        // Execute each pass in the graph
        for (const auto& pass : m_graphPasses)
        {
            static uint64_t previousSubmitValue = 0; // TODO: remove

            auto commandList = m_device->CreateCommandList({
                .name      = pass->GetName(),
                .queueType = QueueType::Graphics,
            });

            OnBeginPassExecute(*pass, *commandList);
            pass->m_onExecuteCallback(*commandList);
            OnEndPassExecute(*pass, *commandList);

            auto swapchain = m_graphImportedSwapchainsLookup.begin()->first;

            previousSubmitValue = m_device->QueueSubmit({
                .waitTimelineValue = previousSubmitValue++,
                .waitPipelineStage = RHI::PipelineStage::TopOfPipe,
                .commandLists      = commandList,
                .swapchainToWait   = swapchain,
                .swapchainToSignal = swapchain,
            });

            [[maybe_unused]] auto res = swapchain->Present();
        }
    }

} // namespace RHI