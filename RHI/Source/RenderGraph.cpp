#include "RHI/RenderGraph.hpp"
#include "RHI/Swapchain.hpp"
#include "RHI/Device.hpp"

#include <TL/Containers.hpp>

#include <tracy/Tracy.hpp>

namespace RHI
{
    RenderGraph::RenderGraph(Device* device)
        : m_arena()
    {
        m_device = device;
    }

    RenderGraph::~RenderGraph()
    {
        Cleanup();
        m_passPool.Clear();
        m_rgImagesPool.Clear();
        m_rgBufferPool.Clear();
    }

    Handle<Pass> RenderGraph::CreatePass(const PassCreateInfo& createInfo)
    {
        Pass pass{};
        pass.m_name = createInfo.name;
        auto handle = m_passPool.Emplace(std::move(pass));
        return m_passList.emplace_back(handle);
    }

    void RenderGraph::PassResize(Handle<Pass> pass, ImageSize2D size)
    {
        m_passPool.Get(pass)->Resize(size);
    }

    ImageSize2D RenderGraph::PassGetSize(Handle<Pass> pass) const
    {
        return m_passPool.Get(pass)->GetSize();
    }

    Handle<RGImage> RenderGraph::ImportSwapchain(const char* name, Swapchain& swapchain)
    {
        RGImage attachment{};
        attachment.name      = name;
        attachment.swapchain = &swapchain;
        auto handle          = m_rgImagesPool.Emplace(std::move(attachment));
        return handle;
    }

    Handle<RGImage> RenderGraph::ImportImage(const char* name, Handle<Image> image)
    {
        RGImage attachment{};
        attachment.name     = name;
        attachment.resource = image;
        auto handle         = m_rgImagesPool.Emplace(std::move(attachment));
        return handle;
    }

    Handle<RGBuffer> RenderGraph::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        RGBuffer attachment{};
        attachment.name     = name;
        attachment.resource = buffer;
        auto handle         = m_rgBufferPool.Emplace(std::move(attachment));
        return handle;
    }

    Handle<RGImage> RenderGraph::CreateImage(const ImageCreateInfo& createInfo)
    {
        RGImage attachment{};
        attachment.name     = createInfo.name;
        attachment.resource = m_device->CreateImage(createInfo).GetValue();
        auto handle         = m_rgImagesPool.Emplace(std::move(attachment));
        m_images.push_back(handle);
        m_transientImages.push_back(handle);
        return handle;
    }

    Handle<RGBuffer> RenderGraph::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        RGBuffer attachment{};
        attachment.name     = createInfo.name;
        attachment.resource = m_device->CreateBuffer(createInfo).GetValue();
        auto handle         = m_rgBufferPool.Emplace(std::move(attachment));
        m_buffers.push_back(handle);
        m_transientBuffers.push_back(handle);
        return handle;
    }

    void RenderGraph::PassUseImage(
        Handle<Pass> _pass, Handle<RGImage> _attachment, ImageUsage usage, TL::Flags<PipelineStage> stages, Access access)
    {
        auto pass       = m_passPool.Get(_pass);
        auto attachment = m_rgImagesPool.Get(_attachment);

        auto passAttachment            = m_arena.Allocate<RGImagePassAccess>();
        passAttachment->pass           = _pass;
        passAttachment->image          = _attachment;
        passAttachment->usage          = usage;
        passAttachment->pipelineAccess = access;
        passAttachment->pipelineStages = stages;
        passAttachment->next           = nullptr;
        passAttachment->prev           = attachment->last;

        if (attachment->first == nullptr)
        {
            attachment->first = passAttachment;
            attachment->last  = passAttachment;
        }
        else
        {
            attachment->last->next = passAttachment;
            attachment->last       = passAttachment;
        }

        pass->m_imageAttachments.push_back(passAttachment);
        if (usage & ImageUsage::DepthStencil)
        {
            pass->m_depthStencilAttachment = passAttachment;
        }
        else if (usage & ImageUsage::Color)
        {
            pass->m_colorAttachments.push_back(passAttachment);
        }
    }

    void RenderGraph::PassUseBuffer(
        Handle<Pass> _pass, Handle<RGBuffer> _attachment, BufferUsage usage, TL::Flags<PipelineStage> stages, Access access)
    {
        auto pass       = m_passPool.Get(_pass);
        auto attachment = m_rgBufferPool.Get(_attachment);

        auto passAttachment            = m_arena.Allocate<RGBufferPassAccess>();
        passAttachment->pass           = _pass;
        passAttachment->buffer         = _attachment;
        passAttachment->usage          = usage;
        passAttachment->pipelineAccess = access;
        passAttachment->pipelineStages = stages;
        passAttachment->next           = nullptr;
        passAttachment->prev           = attachment->last;

        if (attachment->first == nullptr)
        {
            attachment->first = passAttachment;
            attachment->last  = passAttachment;
        }
        else
        {
            attachment->last = passAttachment;
        }

        pass->m_bufferAttachments.push_back(passAttachment);
    }

    Handle<Image> RenderGraph::GetImage(Handle<RGImage> _attachment) const
    {
        auto attachment = m_rgImagesPool.Get(_attachment);
        if (attachment->swapchain) return attachment->swapchain->GetImage();
        else return attachment->resource;
    }

    Handle<Buffer> RenderGraph::GetBuffer(Handle<RGBuffer> _attachment) const
    {
        auto attachment = m_rgBufferPool.Get(_attachment);
        return attachment->resource;
    }

    void RenderGraph::Compile()
    {
        // should be no op for now
    }

    void RenderGraph::Cleanup()
    {
        for (auto attachmentHandle : m_transientImages)
        {
            auto resource = GetImage(attachmentHandle);
            m_device->DestroyImage(resource);
        }

        for (auto attachmentHandle : m_transientBuffers)
        {
            auto resource = GetBuffer(attachmentHandle);
            m_device->DestroyBuffer(resource);
        }
    }

} // namespace RHI