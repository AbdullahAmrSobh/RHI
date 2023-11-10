#include "RHI/FrameGraph.hpp"
#include "RHI/Context.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{
    void Pass::Begin()
    {
        OnBegin();
    }

    void Pass::End()
    {
        OnEnd();
    }

    PassQueueState Pass::GetPassQueueState() const
    {
        return GetPassQueueStateInternal();
    }

    void Pass::ExecuteAfter(Pass& pass)
    {
        m_producers.push_back(&pass);
    }

    void Pass::ExecuteBefore(Pass& pass)
    {
        m_consumers.push_back(&pass);
    }

    ImagePassAttachment* Pass::ImportImageResource(const char* name, Handle<Image> image, const ImageAttachmentUseInfo& useInfo)
    {
        Attachment attachment{};
        attachment.name           = name;
        attachment.lifetime       = AttachmentLifetime::Persistent;
        attachment.type           = AttachmentType::Image;
        attachment.asImage.handle = image;
        auto attachmentHandle     = m_scheduler->m_attachments.Insert(attachment);

        return UseAttachment(attachmentHandle, useInfo);
    }

    BufferPassAttachment* Pass::ImportBufferResource(const char* name, Handle<Buffer> buffer, const BufferAttachmentUseInfo& useInfo)
    {
        Attachment attachment{};
        attachment.name            = name;
        attachment.lifetime        = AttachmentLifetime::Persistent;
        attachment.type            = AttachmentType::Buffer;
        attachment.asBuffer.handle = buffer;
        auto attachmentHandle      = m_scheduler->m_attachments.Insert(attachment);

        return UseAttachment(attachmentHandle, useInfo);
    }

    ImagePassAttachment* Pass::CreateTransientImageResource(const char* name, const ImageCreateInfo& createInfo, const ImageAttachmentUseInfo& useInfo)
    {
        Attachment attachment{};
        attachment.name           = name;
        attachment.lifetime       = AttachmentLifetime::Transient;
        attachment.type           = AttachmentType::Image;
        attachment.asImage.info   = createInfo;
        attachment.asImage.handle = m_scheduler->CreateTransientImageResource(createInfo);
        auto attachmentHandle     = m_scheduler->m_attachments.Insert(attachment);
        m_scheduler->m_transientAttachment.push_back(attachmentHandle);

        return UseAttachment(attachmentHandle, useInfo);
    }

    BufferPassAttachment* Pass::CreateTransientBufferResource(const char* name, const BufferCreateInfo& createInfo, const BufferAttachmentUseInfo& useInfo)
    {
        Attachment attachment{};
        attachment.name            = name;
        attachment.lifetime        = AttachmentLifetime::Transient;
        attachment.type            = AttachmentType::Buffer;
        attachment.asBuffer.info   = createInfo;
        attachment.asBuffer.handle = m_scheduler->CreateTransientBufferResource(createInfo);
        auto attachmentHandle      = m_scheduler->m_attachments.Insert(attachment);
        m_scheduler->m_transientAttachment.push_back(attachmentHandle);

        return UseAttachment(attachmentHandle, useInfo);
    }

    ImagePassAttachment* Pass::UseImageResource(ImagePassAttachment* view, const ImageAttachmentUseInfo& useInfo)
    {
        m_producers.push_back(view->pass);
        view->pass->m_consumers.push_back(this);
        return UseAttachment(view->attachment, useInfo);
    }

    BufferPassAttachment* Pass::UseBufferResource(BufferPassAttachment* view, const BufferAttachmentUseInfo& useInfo)
    {
        m_producers.push_back(view->pass);
        view->pass->m_consumers.push_back(this);
        return UseAttachment(view->attachment, useInfo);
    }

    ImagePassAttachment* Pass::UseAttachment(Handle<Attachment> attachmentHandle, const ImageAttachmentUseInfo& useInfo)
    {
        auto                attachment = m_scheduler->m_attachments.Get(attachmentHandle);

        ImagePassAttachment passAttachment{};
        passAttachment.attachment   = attachmentHandle;
        passAttachment.pass         = this;
        passAttachment.info         = useInfo;
        passAttachment.prev         = attachment->asImage.lastUse;
        passAttachment.next         = nullptr;
        attachment->asImage.lastUse = &m_imagePassAttachments.emplace_back(passAttachment);
        return attachment->asImage.lastUse;
    }

    BufferPassAttachment* Pass::UseAttachment(Handle<Attachment> attachmentHandle, const BufferAttachmentUseInfo& useInfo)
    {
        auto                 attachment = m_scheduler->m_attachments.Get(attachmentHandle);

        BufferPassAttachment passAttachment{};
        passAttachment.attachment    = attachmentHandle;
        passAttachment.pass          = this;
        passAttachment.info          = useInfo;
        passAttachment.prev          = attachment->asBuffer.lastUse;
        passAttachment.next          = nullptr;
        attachment->asBuffer.lastUse = &m_bufferPassAttachment.emplace_back(passAttachment);
        return attachment->asBuffer.lastUse;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Pass /////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////

    void FrameScheduler::Begin()
    {
        BeginInternal();

        CleanupTransientResources();
    }

    void FrameScheduler::End()
    {
        EndInternal();

        for (auto pass : m_passList)
        {
            ExecutePass(pass);
        }

        for (auto swapchain : m_swapchainsToPresent)
        {
            swapchain->Present();
        }
    }

    void FrameScheduler::Submit(Pass& pass)
    {
        m_passList.push_back(&pass);
    }

    void FrameScheduler::Compile()
    {
        TopologicalSort();

        InitTransientResources();

        CreateAttachmentViews();
    }

    Attachment* FrameScheduler::GetAttachment(Handle<Attachment> attachmentHandle)
    {
        return m_attachments.Get(attachmentHandle);
    }

    void FrameScheduler::TopologicalSort()
    {
    }

    void FrameScheduler::InitTransientResources()
    {
        for (auto& pass : m_passList)
        {
            for (auto& passAttachment : pass->m_imagePassAttachments)
            {
                if (passAttachment.prev == nullptr)
                {
                    Allocate(passAttachment.attachment);
                }
            }

            for (auto& passAttachment : pass->m_bufferPassAttachment)
            {
                if (passAttachment.prev == nullptr)
                {
                    Allocate(passAttachment.attachment);
                }
            }

            for (auto& passAttachment : pass->m_imagePassAttachments)
            {
                if (passAttachment.next == nullptr)
                {
                    Release(passAttachment.attachment);
                }
            }

            for (auto& passAttachments : pass->m_bufferPassAttachment)
            {
                if (passAttachments.next == nullptr)
                {
                    Release(passAttachments.attachment);
                }
            }
        }
    }

    void FrameScheduler::CreateAttachmentViews()
    {
        for (auto pass : m_passList)
        {
            for (auto& passAttachment : pass->m_imagePassAttachments)
            {
                auto attachment     = GetAttachment(passAttachment.attachment);
                passAttachment.view = CreateImageView(attachment, passAttachment.info);
            }

            for (auto& passAttachment : pass->m_bufferPassAttachment)
            {
                auto attachment     = GetAttachment(passAttachment.attachment);
                passAttachment.view = CreateBufferView(attachment, passAttachment.info);
            }
        }
    }

    void FrameScheduler::CleanupTransientResources()
    {
        for (auto pass : m_passList)
        {
            for (auto& passAttachment : pass->m_imagePassAttachments)
            {
                auto attachment = GetAttachment(passAttachment.attachment);
                if (attachment->lifetime == RHI::AttachmentLifetime::Transient)
                {
                    FreeImageView(passAttachment.view);
                }
            }

            for (auto& passAttachment : pass->m_bufferPassAttachment)
            {
                auto attachment = GetAttachment(passAttachment.attachment);
                if (attachment->lifetime == RHI::AttachmentLifetime::Transient)
                {
                    FreeBufferView(passAttachment.view);
                }
            }
        }

        for (auto handle : m_transientAttachment)
        {
            auto attachment = m_attachments.Get(handle);
            if (attachment->type == RHI::AttachmentType::Image)
            {
                FreeTransientImageResource(attachment->asImage.handle);
            }
            else if (attachment->type == RHI::AttachmentType::Buffer)
            {
                FreeTransientBufferResource(attachment->asBuffer.handle);
            }
        }
    }

} // namespace RHI