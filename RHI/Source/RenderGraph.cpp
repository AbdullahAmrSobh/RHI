#include "RHI/RenderGraph.hpp"
#include "RHI/Swapchain.hpp"

#include <tracy/Tracy.hpp>

namespace RHI
{
    RenderGraph::RenderGraph(Context* context)
        : m_context(context)
    {
    }

    Handle<Pass> RenderGraph::CreatePass(const PassCreateInfo& createInfo)
    {
        Pass pass{};
        pass.renderTargetSize.width = 1600;
        pass.renderTargetSize.height = 1200;
        pass.name = createInfo.name;
        pass.queueType = createInfo.queueType;
        auto handle = m_passOwner.Emplace(std::move(pass));
        m_passes.push_back(handle);
        return handle;
    }

    Handle<ImageAttachment> RenderGraph::CreateImage(const ImageCreateInfo& createInfo)
    {
        ImageAttachment imageAttachment{};
        Handle<ImageAttachment> attachmentHandle = m_imageAttachmentOwner.Emplace(std::move(imageAttachment));

        ImageAttachmentList graphAttachment{};
        graphAttachment.renderGraph = this;
        graphAttachment.name = createInfo.name;
        graphAttachment.lifetime = AttachmentLifetime::Transient;
        graphAttachment.referenceCount = 0;
        graphAttachment.info = createInfo;
        graphAttachment.begin = attachmentHandle;
        graphAttachment.end = attachmentHandle;
        auto newAttachment = m_imageAttachmentOwner.Get(attachmentHandle);
        newAttachment->list = m_graphImageAttachmentOwner.Emplace(std::move(graphAttachment));
        m_graphImageAttachments.push_back(newAttachment->list);
        return attachmentHandle;
    }

    Handle<BufferAttachment> RenderGraph::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        BufferAttachment bufferAttachment{};
        Handle<BufferAttachment> attachmentHandle = m_bufferAttachmentOwner.Emplace(std::move(bufferAttachment));

        BufferAttachmentList graphAttachment{};
        graphAttachment.renderGraph = this;
        graphAttachment.name = createInfo.name;
        graphAttachment.lifetime = AttachmentLifetime::Transient;
        graphAttachment.referenceCount = 0;
        graphAttachment.info = createInfo;
        graphAttachment.begin = attachmentHandle;
        graphAttachment.end = attachmentHandle;
        auto newAttachment = m_bufferAttachmentOwner.Get(attachmentHandle);
        newAttachment->list = m_graphBufferAttachmentOwner.Emplace(std::move(graphAttachment));
        m_graphBufferAttachments.push_back(newAttachment->list);
        return attachmentHandle;
    }

    Handle<ImageAttachment> RenderGraph::ImportSwapchain(const char* name, Swapchain& swapchain)
    {
        ImageAttachment imageAttachment{};
        Handle<ImageAttachment> attachmentHandle = m_imageAttachmentOwner.Emplace(std::move(imageAttachment));

        ImageAttachmentList graphAttachment{};
        graphAttachment.renderGraph = this;
        graphAttachment.name = name;
        graphAttachment.lifetime = AttachmentLifetime::Persistent;
        graphAttachment.referenceCount = 0;
        graphAttachment.swapchain = &swapchain;
        graphAttachment.begin = attachmentHandle;
        graphAttachment.end = attachmentHandle;
        auto newAttachment = m_imageAttachmentOwner.Get(attachmentHandle);
        newAttachment->list = m_graphImageAttachmentOwner.Emplace(std::move(graphAttachment));
        m_graphImageAttachments.push_back(newAttachment->list);
        return attachmentHandle;
    }

    Handle<ImageAttachment> RenderGraph::ImportImage(const char* name, Handle<Image> image)
    {
        ImageAttachment imageAttachment{};
        Handle<ImageAttachment> attachmentHandle = m_imageAttachmentOwner.Emplace(std::move(imageAttachment));

        ImageAttachmentList graphAttachment{};
        graphAttachment.renderGraph = this;
        graphAttachment.name = name;
        graphAttachment.lifetime = AttachmentLifetime::Persistent;
        graphAttachment.referenceCount = 0;
        graphAttachment.handle = image;
        graphAttachment.begin = attachmentHandle;
        graphAttachment.end = attachmentHandle;
        auto newAttachment = m_imageAttachmentOwner.Get(attachmentHandle);
        newAttachment->list = m_graphImageAttachmentOwner.Emplace(std::move(graphAttachment));
        m_graphImageAttachments.push_back(newAttachment->list);
        return attachmentHandle;
    }

    Handle<BufferAttachment> RenderGraph::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        BufferAttachment bufferAttachment{};
        Handle<BufferAttachment> attachmentHandle = m_bufferAttachmentOwner.Emplace(std::move(bufferAttachment));

        BufferAttachmentList graphAttachment{};
        graphAttachment.renderGraph = this;
        graphAttachment.name = name;
        graphAttachment.lifetime = AttachmentLifetime::Persistent;
        graphAttachment.referenceCount = 0;
        graphAttachment.handle = buffer;
        graphAttachment.begin = attachmentHandle;
        graphAttachment.end = attachmentHandle;
        auto newAttachment = m_bufferAttachmentOwner.Get(attachmentHandle);
        newAttachment->list = m_graphBufferAttachmentOwner.Emplace(std::move(graphAttachment));
        m_graphBufferAttachments.push_back(newAttachment->list);
        return attachmentHandle;
    }

    Handle<ImageAttachment> RenderGraph::UseImage(Handle<Pass> pass, Handle<ImageAttachment> attachmentHandle, const ImageAttachmentUseInfo& useInfo)
    {
        auto attachment = m_imageAttachmentOwner.Get(attachmentHandle);

        Handle<ImageAttachment> newAttachmentHandle;
        if (attachment->pass == NullHandle)
        {
            attachment->pass = pass;
            attachment->useInfo = useInfo;
            newAttachmentHandle = attachmentHandle;
        }
        else
        {
            ImageAttachment newAttachment{};
            newAttachment.pass = pass;
            newAttachment.useInfo = useInfo;
            newAttachment.prev = attachmentHandle;
            newAttachment.list = attachment->list;
            newAttachmentHandle = attachment->next = m_imageAttachmentOwner.Emplace(std::move(newAttachment));

            auto graphAttachment = m_graphImageAttachmentOwner.Get(attachment->list);
            graphAttachment->end = attachment->next;
        }

        auto _pass = m_passOwner.Get(pass);
        if (useInfo.usage == ImageUsage::Color)
        {
            _pass->colorAttachments.push_back(newAttachmentHandle);
        }
        else if (useInfo.usage == ImageUsage::Depth || useInfo.usage == ImageUsage::DepthStencil)
        {
            _pass->depthStencilAttachment = newAttachmentHandle;
        }
        _pass->imageAttachments.push_back(newAttachmentHandle);

        return newAttachmentHandle;
    }

    Handle<BufferAttachment> RenderGraph::UseBuffer(Handle<Pass> pass, Handle<BufferAttachment> attachmentHandle, const BufferAttachmentUseInfo& useInfo)
    {
        auto attachment = m_bufferAttachmentOwner.Get(attachmentHandle);
        if (attachment->pass == NullHandle)
        {
            attachment->pass = pass;
            attachment->useInfo = useInfo;
            return attachmentHandle;
        }
        else
        {
            BufferAttachment newAttachment{};
            newAttachment.pass = pass;
            newAttachment.useInfo = useInfo;
            newAttachment.prev = attachmentHandle;
            newAttachment.list = attachment->list;
            attachment->next = m_bufferAttachmentOwner.Emplace(std::move(newAttachment));

            auto graphAttachment = m_graphBufferAttachmentOwner.Get(attachment->list);
            graphAttachment->end = attachment->next;
            return attachment->next;
        }
    }

    void RenderGraph::SubmitCommands(Handle<Pass> passHandle, TL::Span<const CommandList* const> commandLists)
    {
        auto pass = m_passOwner.Get(passHandle);
        pass->commandList = TL::Vector<const CommandList*>(commandLists.begin(), commandLists.end());
    }

    Handle<Image> RenderGraph::GetImage(Handle<ImageAttachment> attachmentHandle)
    {
        auto attachment = GetAttachmentList(attachmentHandle);
        if (attachment->swapchain)
        {
            return attachment->swapchain->GetImage();
        }
        return attachment->handle;
    }

    Handle<Buffer> RenderGraph::GetBuffer(Handle<BufferAttachment> attachment)
    {
        return GetAttachmentList(attachment)->handle;
    }

    Handle<ImageView> RenderGraph::GetImageView(Handle<ImageAttachment> attachmentHandle)
    {
        auto attachment = m_imageAttachmentOwner.Get(attachmentHandle);
        if (auto swapchain = GetSwapchain(attachmentHandle))
        {
            ImageViewCreateInfo info{};
            info.image = GetImage(attachmentHandle);
            info.components = attachment->useInfo.componentMapping;
            info.subresource = attachment->useInfo.subresourceRange;
            return swapchain->GetImageView(info);
        }
        return attachment->view;
    }

    Handle<BufferView> RenderGraph::GetBufferView(Handle<BufferAttachment> attachment)
    {
        return m_bufferAttachmentOwner.Get(attachment)->view;
    }

    Swapchain* RenderGraph::GetSwapchain(Handle<ImageAttachment> attachment)
    {
        return GetAttachmentList(attachment)->swapchain;
    }

} // namespace RHI