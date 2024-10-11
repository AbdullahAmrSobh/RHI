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
        m_imageAttachmentPool.Clear();
        m_bufferAttachmentPool.Clear();
    }

    Handle<Pass> RenderGraph::CreatePass(const PassCreateInfo& createInfo)
    {
        Pass pass{};
        pass.m_name = createInfo.name;
        auto handle = m_passPool.Emplace(std::move(pass));
        return m_passes.emplace_back(handle);
    }

    void RenderGraph::PassResize(Handle<Pass> pass, ImageSize2D size)
    {
        m_passPool.Get(pass)->Resize(size);
    }

    ImageSize2D RenderGraph::GetPassSize(Handle<Pass> pass) const
    {
        return m_passPool.Get(pass)->GetSize();
    }

    Handle<ImageAttachment> RenderGraph::ImportSwapchain(const char* name, Swapchain& swapchain)
    {
        ImageAttachment attachment{};
        attachment.name = name;
        attachment.swapchain = &swapchain;
        auto handle = m_imageAttachmentPool.Emplace(std::move(attachment));
        return handle;
    }

    Handle<ImageAttachment> RenderGraph::ImportImage(const char* name, Handle<Image> image)
    {
        ImageAttachment attachment{};
        attachment.name = name;
        attachment.resource = image;
        attachment.isTransient = false;
        auto handle = m_imageAttachmentPool.Emplace(std::move(attachment));
        return handle;
    }

    Handle<BufferAttachment> RenderGraph::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        BufferAttachment attachment{};
        attachment.name = name;
        attachment.resource = buffer;
        auto handle = m_bufferAttachmentPool.Emplace(std::move(attachment));
        return handle;
    }

    Handle<ImageAttachment> RenderGraph::CreateImage(const ImageCreateInfo& createInfo)
    {
        ImageAttachment attachment{};
        attachment.name = createInfo.name;
        attachment.info = createInfo;
        attachment.isTransient = true;
        auto handle = m_imageAttachmentPool.Emplace(std::move(attachment));
        m_imageAttachments.push_back(handle);
        m_transientImageAttachments.push_back(handle);
        return handle;
    }

    Handle<BufferAttachment> RenderGraph::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        BufferAttachment attachment{};
        attachment.name = createInfo.name;
        attachment.info = createInfo;
        auto handle = m_bufferAttachmentPool.Emplace(std::move(attachment));
        m_bufferAttachments.push_back(handle);
        m_transientBufferAttachments.push_back(handle);
        return handle;
    }

    void RenderGraph::PassUseImage(Handle<Pass> _pass, Handle<ImageAttachment> _attachment, const ImageViewInfo& viewInfo, ImageUsage usage, TL::Flags<ShaderStage> stages, Access access)
    {
        auto pass = m_passPool.Get(_pass);
        auto attachment = m_imageAttachmentPool.Get(_attachment);

        ImageViewCreateInfo createInfo{};
        createInfo.image = NullHandle;
        createInfo.viewType = viewInfo.type;
        createInfo.components = viewInfo.swizzle;
        createInfo.subresource = viewInfo.subresources;

        auto it = attachment->list.emplace(_pass, m_arena.Allocate<ImagePassAttachment>());
        auto passAttachment = it.first->second;
        passAttachment->pass = _pass;
        passAttachment->attachment = _attachment;
        passAttachment->usage = usage;
        passAttachment->access = access;
        passAttachment->stages = stages;
        passAttachment->viewInfo = viewInfo;
        passAttachment->next = nullptr;
        passAttachment->prev = attachment->last;

        if (attachment->first == nullptr)
        {
            attachment->first = passAttachment;
            attachment->last = passAttachment;
        }
        else
        {
            attachment->last->next = passAttachment;
            attachment->last = passAttachment;
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

    void RenderGraph::PassUseBuffer(Handle<Pass> _pass, Handle<BufferAttachment> _attachment, const BufferViewInfo& viewInfo, BufferUsage usage, TL::Flags<ShaderStage> stages, Access access)
    {
        auto pass = m_passPool.Get(_pass);
        auto attachment = m_bufferAttachmentPool.Get(_attachment);

        BufferViewCreateInfo createInfo{};
        createInfo.buffer = NullHandle;
        createInfo.subregion = viewInfo.subregion;
        createInfo.format = viewInfo.format;

        attachment->list.emplace(_pass, m_arena.Allocate<BufferPassAttachment>());
        auto passAttachment = attachment->list[_pass];
        passAttachment->pass = _pass;
        passAttachment->attachment = _attachment;
        passAttachment->usage = usage;
        passAttachment->access = access;
        passAttachment->stages = stages;
        passAttachment->viewInfo = viewInfo;
        passAttachment->next = nullptr;
        passAttachment->prev = attachment->last;

        if (attachment->first == nullptr)
        {
            attachment->first = passAttachment;
            attachment->last = passAttachment;
        }
        else
        {
            attachment->last = passAttachment;
        }

        pass->m_bufferAttachments.push_back(passAttachment);
    }

    Handle<Image> RenderGraph::GetImage(Handle<ImageAttachment> _attachment) const
    {
        auto attachment = m_imageAttachmentPool.Get(_attachment);
        if (attachment->swapchain)
            return attachment->swapchain->GetImage();
        else if (attachment->resource)
            return attachment->resource;

        auto pass = m_passPool.Get(attachment->list.begin()->second->pass);
        auto size = pass->m_renderTargetSize;

        ImageCreateInfo info = attachment->info;
        if (attachment->info.usageFlags & (ImageUsage::Color | ImageUsage::DepthStencil))
        {
            info.size.width = size.width;
            info.size.height = size.height;
            info.size.depth = 1;
        }
        else
        {
            TL_ASSERT(info.size != ImageSize3D{});
        }

        auto key = TL::HashCombine(TL::HashAny(info), std::hash<TL::String>{}(attachment->name));
        if (auto image = m_imagesLRU.find(key); image != m_imagesLRU.end())
        {
            return image->second;
        }
        return m_imagesLRU[key] = m_device->CreateImage(info).GetValue();
    }

    Handle<Buffer> RenderGraph::GetBuffer(Handle<BufferAttachment> _attachment) const
    {
        auto attachment = m_bufferAttachmentPool.Get(_attachment);
        if (attachment->resource)
        {
            return attachment->resource;
        }

        auto info = attachment->info;
        auto key = TL::HashCombine(TL::HashAny(info), std::hash<TL::String>{}(attachment->name));

        if (auto buffer = m_buffersLRU.find(key); buffer != m_buffersLRU.end())
        {
            return buffer->second;
        }
        return m_buffersLRU[key] = m_device->CreateBuffer(info).GetValue();
    }

    Handle<ImageView> RenderGraph::PassGetImageView(Handle<Pass> _pass, Handle<ImageAttachment> _attachment) const
    {
        auto image = GetImage(_attachment);
        auto attachment = m_imageAttachmentPool.Get(_attachment);
        auto viewInfo = attachment->Find(_pass)->viewInfo;

        ImageViewCreateInfo createInfo{};
        createInfo.image = image;
        createInfo.viewType = viewInfo.type;
        createInfo.subresource = viewInfo.subresources;
        createInfo.components = viewInfo.swizzle;
        auto key = TL::HashCombine(TL::HashAny(createInfo), std::hash<TL::String>{}(attachment->name));

        if (auto imageView = m_imageViewsLRU.find(key); imageView != m_imageViewsLRU.end())
        {
            return imageView->second;
        }

        return m_imageViewsLRU[key] = m_device->CreateImageView(createInfo);
    }

    Handle<BufferView> RenderGraph::PassGetBufferView(Handle<Pass> _pass, Handle<BufferAttachment> _attachment) const
    {
        auto buffer = GetBuffer(_attachment);
        auto attachment = m_bufferAttachmentPool.Get(_attachment);
        auto viewInfo = attachment->Find(_pass)->viewInfo;

        BufferViewCreateInfo createInfo{};
        createInfo.buffer = buffer;
        createInfo.subregion = viewInfo.subregion;
        createInfo.format = viewInfo.format;
        auto key = TL::HashCombine(TL::HashAny(createInfo), std::hash<TL::String>{}(attachment->name));

        if (auto bufferView = m_bufferViewsLRU.find(key); bufferView != m_bufferViewsLRU.end())
        {
            return bufferView->second;
        }
        return m_bufferViewsLRU[key] = m_device->CreateBufferView(createInfo);
    }

    void RenderGraph::Compile()
    {
        // should be no op for now
    }

    void RenderGraph::Cleanup()
    {
        CleanupAttachmentViews();
        CleanupTransientAttachments();
    }

    void RenderGraph::CleanupAttachmentViews()
    {
        for (auto [_, viewHandle] : m_imageViewsLRU)
        {
            m_device->DestroyImageView(viewHandle);
        }

        for (auto [_, viewHandle] : m_bufferViewsLRU)
        {
            m_device->DestroyBufferView(viewHandle);
        }
    }

    void RenderGraph::CleanupTransientAttachments()
    {
        for (auto attachmentHandle : m_transientImageAttachments)
        {
            auto resource = GetImage(attachmentHandle);
            m_device->DestroyImage(resource);
        }

        for (auto attachmentHandle : m_transientBufferAttachments)
        {
            auto resource = GetBuffer(attachmentHandle);
            m_device->DestroyBuffer(resource);
        }
    }
} // namespace RHI