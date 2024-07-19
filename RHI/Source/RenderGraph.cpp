#include "RHI/RenderGraph.hpp"
#include "RHI/Swapchain.hpp"
#include "RHI/Context.hpp"

#include <tracy/Tracy.hpp>

namespace RHI
{
    RenderGraph::RenderGraph(Context* context)
    {
        m_context = context;
    }

    Handle<Pass> RenderGraph::CreatePass(const PassCreateInfo& createInfo)
    {
        Pass pass{};
        pass.m_name = createInfo.name;
        auto handle = m_passPool.Emplace(std::move(pass));
        return m_passes.emplace_back(handle);
    }

    void RenderGraph::PassResize(Handle<Pass> _pass, ImageSize2D size)
    {
        auto pass = m_passPool.Get(_pass);
        pass->m_renderTargetSize = size;
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
        return handle;
    }

    Handle<BufferAttachment> RenderGraph::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        BufferAttachment attachment{};
        attachment.name = createInfo.name;
        attachment.info = createInfo;
        auto handle = m_bufferAttachmentPool.Emplace(std::move(attachment));
        return handle;
    }

    void RenderGraph::PassUseImage(Handle<Pass> _pass, Handle<ImageAttachment> _attachment, const ImageViewInfo& viewInfo, ImageUsage usage, Flags<ShaderStage> stages, Access access)
    {
        auto pass = m_passPool.Get(_pass);
        auto attachment = m_imageAttachmentPool.Get(_attachment);

        ImageViewCreateInfo createInfo{};
        createInfo.image = NullHandle;
        createInfo.viewType = viewInfo.type;
        createInfo.components = viewInfo.swizzle;
        createInfo.subresource = viewInfo.subresources;

        auto it = attachment->list.try_emplace(_pass, new ImagePassAttachment());
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

    void RenderGraph::PassUseBuffer(Handle<Pass> _pass, Handle<BufferAttachment> _attachment, const BufferViewInfo& viewInfo, BufferUsage usage, Flags<ShaderStage> stages, Access access)
    {
        auto pass = m_passPool.Get(_pass);
        auto attachment = m_bufferAttachmentPool.Get(_attachment);

        BufferViewCreateInfo createInfo{};
        createInfo.buffer = NullHandle;
        createInfo.subregion = viewInfo.subregion;
        createInfo.format = viewInfo.format;

        attachment->list.emplace(_pass, new BufferPassAttachment());
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
            RHI_ASSERT(info.size != ImageSize3D{});
        }

        auto key = HashCombine(HashAny(info), std::hash<TL::String>{}(attachment->name));
        if (auto image = m_imagesLRU.find(key); image != m_imagesLRU.end())
        {
            return image->second;
        }
        return m_imagesLRU[key] = m_context->CreateImage(info).GetValue();
    }

    Handle<Buffer> RenderGraph::GetBuffer(Handle<BufferAttachment> _attachment) const
    {
        auto attachment = m_bufferAttachmentPool.Get(_attachment);
        if (attachment->resource)
        {
            return attachment->resource;
        }

        auto info = attachment->info;
        auto key = HashCombine(HashAny(info), std::hash<TL::String>{}(attachment->name));

        if (auto buffer = m_buffersLRU.find(key); buffer != m_buffersLRU.end())
        {
            return buffer->second;
        }
        return m_buffersLRU[key] = m_context->CreateBuffer(info).GetValue();
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
        auto key = HashCombine(HashAny(createInfo), std::hash<TL::String>{}(attachment->name));

        if (auto imageView = m_imageViewsLRU.find(key); imageView != m_imageViewsLRU.end())
        {
            return imageView->second;
        }

        return m_imageViewsLRU[key] = m_context->CreateImageView(createInfo);
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
        auto key = HashCombine(HashAny(createInfo), std::hash<TL::String>{}(attachment->name));

        if (auto bufferView = m_bufferViewsLRU.find(key); bufferView != m_bufferViewsLRU.end())
        {
            return bufferView->second;
        }
        return m_bufferViewsLRU[key] = m_context->CreateBufferView(createInfo);
    }

    void RenderGraph::Submit(Handle<Pass> _pass, TL::Span<CommandList*> commandList, [[maybe_unused]] Fence* signalFence)
    {
        auto pass = m_passPool.Get(_pass);
        pass->m_commandLists.clear();
        pass->m_commandLists.insert(pass->m_commandLists.end(), commandList.begin(), commandList.end());
    }

    void RenderGraph::Compile()
    {
        // should be no op for now
    }

    void RenderGraph::Cleanup()
    {
        // todo
    }
} // namespace RHI