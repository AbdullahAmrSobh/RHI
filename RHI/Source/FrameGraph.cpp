#include "RHI/FrameGraph.hpp"

#include "RHI/Context.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{
    //////////////////////////////////////////////////////////////////////////////////////////
    /// AttachmentsRegistry
    //////////////////////////////////////////////////////////////////////////////////////////

    void AttachmentsRegistry::Reset()
    {
        for (auto attachment : m_attachments)
        {
            attachment->Reset();
        }
    }

    ImageAttachment* AttachmentsRegistry::ImportSwapchainImageAttachment(const char* name, Swapchain* swapchain)
    {
        auto& attachment = m_imageAttachments.emplace_back(std::make_unique<ImageAttachment>(name, swapchain));

        m_swapchainAttachments.push_back(attachment.get());
        m_attachments.push_back(attachment.get());

        return attachment.get();
    }

    ImageAttachment* AttachmentsRegistry::ImportImageAttachment(const char* name, Handle<Image> handle)
    {
        auto& attachment = m_imageAttachments.emplace_back(std::make_unique<ImageAttachment>(name, handle));

        m_importedImageAttachments.push_back(attachment.get());
        m_attachments.push_back(attachment.get());

        return attachment.get();
    }

    BufferAttachment* AttachmentsRegistry::ImportBufferAttachment(const char* name, Handle<Buffer> handle)
    {
        auto& attachment = m_bufferAttachments.emplace_back(std::make_unique<BufferAttachment>(name, handle));

        m_importedBufferAttachments.push_back(attachment.get());
        m_attachments.push_back(attachment.get());

        return attachment.get();
    }

    ImageAttachment* AttachmentsRegistry::CreateTransientImageAttachment(const char* name, const ImageCreateInfo& createInfo)
    {
        auto& attachment = m_imageAttachments.emplace_back(std::make_unique<ImageAttachment>(name, createInfo));

        m_transientImageAttachments.push_back(attachment.get());
        m_attachments.push_back(attachment.get());

        return attachment.get();
    }

    BufferAttachment* AttachmentsRegistry::CreateTransientBufferAttachment(const char* name, const BufferCreateInfo& createInfo)
    {
        auto& attachment = m_bufferAttachments.emplace_back(std::make_unique<BufferAttachment>(name, createInfo));

        m_transientBufferAttachments.push_back(attachment.get());
        m_attachments.push_back(attachment.get());

        return attachment.get();
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Pass
    //////////////////////////////////////////////////////////////////////////////////////////

    void Pass::Begin()
    {
    }

    void Pass::End()
    {
    }

    void Pass::ExecuteAfter(Pass& pass)
    {
        m_producers.push_back(&pass);
        pass.m_consumers.push_back(&pass);
    }

    void Pass::ExecuteBefore(Pass& pass)
    {
        m_consumers.push_back(&pass);
        pass.m_producers.push_back(&pass);
    }

    ImagePassAttachment* Pass::ImportSwapchainImageResource(const char* name, Swapchain* swapchain, const ImageAttachmentUseInfo& useInfo)
    {
        m_swapchain     = swapchain;
        auto attachemnt = m_scheduler->m_attachmentsRegistry->ImportSwapchainImageAttachment(name, swapchain);
        return UseAttachment(attachemnt, useInfo);
    }

    ImagePassAttachment* Pass::ImportImageResource(const char* name, Handle<Image> image, const ImageAttachmentUseInfo& useInfo)
    {
        auto attachemnt = m_scheduler->m_attachmentsRegistry->ImportImageAttachment(name, image);
        return UseAttachment(attachemnt, useInfo);
    }

    BufferPassAttachment* Pass::ImportBufferResource(const char* name, Handle<Buffer> buffer, const BufferAttachmentUseInfo& useInfo)
    {
        auto attachemnt = m_scheduler->m_attachmentsRegistry->ImportBufferAttachment(name, buffer);
        return UseAttachment(attachemnt, useInfo);
    }

    ImagePassAttachment* Pass::CreateTransientImageResource(const char* name, const ImageCreateInfo& createInfo, const ImageAttachmentUseInfo& useInfo)
    {
        auto attachemnt = m_scheduler->m_attachmentsRegistry->CreateTransientImageAttachment(name, createInfo);
        return UseAttachment(attachemnt, useInfo);
    }

    BufferPassAttachment* Pass::CreateTransientBufferResource(const char* name, const BufferCreateInfo& createInfo, const BufferAttachmentUseInfo& useInfo)
    {
        auto attachemnt = m_scheduler->m_attachmentsRegistry->CreateTransientBufferAttachment(name, createInfo);
        return UseAttachment(attachemnt, useInfo);
    }

    ImagePassAttachment* Pass::UseImageResource(ImagePassAttachment* attachment, const ImageAttachmentUseInfo& useInfo)
    {
        return UseAttachment(attachment->attachment, useInfo);
    }

    BufferPassAttachment* Pass::UseBufferResource(BufferPassAttachment* attachment, const BufferAttachmentUseInfo& useInfo)
    {
        return UseAttachment(attachment->attachment, useInfo);
    }

    ImagePassAttachment* Pass::UseAttachment(ImageAttachment*& attachment, const ImageAttachmentUseInfo& useInfo)
    {
        if (m_scheduler->m_state != FrameScheduler::Invalid)
        {
            m_scheduler->Reset();
        }

        ImagePassAttachment& passAttachment = m_imagePassAttachments.emplace_back(ImagePassAttachment{});
        passAttachment.attachment           = attachment;
        passAttachment.pass                 = this;
        passAttachment.info                 = useInfo;
        passAttachment.next                 = nullptr;
        passAttachment.prev                 = attachment->lastUse;

        if (attachment->firstUse == nullptr)
        {
            attachment->firstUse = &passAttachment;
            attachment->lastUse  = &passAttachment;
        }
        else
        {
            attachment->lastUse->next = &passAttachment;
            attachment->lastUse       = attachment->lastUse->next;
        }

        return &passAttachment;
    }

    BufferPassAttachment* Pass::UseAttachment(BufferAttachment*& attachment, const BufferAttachmentUseInfo& useInfo)
    {
        if (m_scheduler->m_state != FrameScheduler::Invalid)
        {
            m_scheduler->Reset();
        }

        BufferPassAttachment& passAttachment = m_bufferPassAttachment.emplace_back(BufferPassAttachment{});
        passAttachment.attachment            = attachment;
        passAttachment.pass                  = this;
        passAttachment.info                  = useInfo;
        passAttachment.next                  = nullptr;
        passAttachment.prev                  = attachment->lastUse;

        if (attachment->firstUse == nullptr)
        {
            attachment->firstUse = &passAttachment;
            attachment->lastUse  = &passAttachment;
        }
        else
        {
            attachment->lastUse->next = &passAttachment;
            attachment->lastUse       = attachment->lastUse->next;
        }
        return &passAttachment;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// FrameScheduler
    //////////////////////////////////////////////////////////////////////////////////////////

    void FrameScheduler::Begin()
    {
        Compile();

        OnFrameEnd();

        // prepare command lists
        {
            uint32_t frameIndex = 0;

            if (m_attachmentsRegistry->m_swapchainAttachments.empty() == false)
            {
                frameIndex = m_attachmentsRegistry->m_swapchainAttachments.front()->swapchain->GetCurrentImageIndex();
            }

            for (auto& pass : m_passList)
            {
                pass->m_commandList = GetCommandList(frameIndex);
            }
        }
    }

    void FrameScheduler::End()
    {
        Execute();
    }

    void FrameScheduler::Submit(Pass& pass)
    {
        m_passList.push_back(&pass);
    }

    void FrameScheduler::Reset()
    {
        m_attachmentsRegistry->Reset();

        m_state = FrameGraphState::Invalid;
    }

    void FrameScheduler::Compile()
    {
        TopologicalSort();

        CompileTransientAttachments();

        CompileAttachmentsViews();

        m_state = FrameGraphState::Ready;
    }

    void FrameScheduler::TopologicalSort()
    {
        if (m_state == FrameGraphState::Ready)
        {
            return;
        }
    }

    void FrameScheduler::CompileTransientAttachments()
    {
        if (m_state == FrameGraphState::Ready)
        {
            return;
        }

        m_transientAttachmentAllocator->Begin();

        for (auto pass : m_passList)
        {
            for (auto& passAttachment : pass->m_imagePassAttachments)
            {
                if (passAttachment.attachment->lifetime != AttachmentLifetime::Transient)
                    continue;
                else if (passAttachment.prev != nullptr)
                    continue;

                auto attachment = passAttachment.attachment;
                m_transientAttachmentAllocator->Allocate(attachment);
            }

            for (auto& passAttachment : pass->m_bufferPassAttachment)
            {
                if (passAttachment.attachment->lifetime != AttachmentLifetime::Transient)
                    continue;
                else if (passAttachment.prev != nullptr)
                    continue;

                auto attachment = passAttachment.attachment;
                m_transientAttachmentAllocator->Allocate(attachment);
            }

            for (auto& passAttachment : pass->m_imagePassAttachments)
            {
                if (passAttachment.attachment->lifetime != AttachmentLifetime::Transient)
                    continue;
                else if (passAttachment.next != nullptr)
                    continue;

                auto attachment = passAttachment.attachment;
                m_transientAttachmentAllocator->Free(attachment);
            }

            for (auto& passAttachment : pass->m_bufferPassAttachment)
            {
                if (passAttachment.attachment->lifetime != AttachmentLifetime::Transient)
                    continue;
                else if (passAttachment.next != nullptr)
                    continue;

                auto attachment = passAttachment.attachment;
                m_transientAttachmentAllocator->Free(attachment);
            }
        }

        m_transientAttachmentAllocator->End();
    }

    void FrameScheduler::CompileAttachmentsViews()
    {
        for (auto pass : m_passList)
        {
            for (auto& passAttachment : pass->m_imagePassAttachments)
            {
                auto attachment = passAttachment.attachment;

                if (auto swapchain = attachment->swapchain)
                {
                    attachment->handle = swapchain->GetImage();
                }

                if (auto it = m_imageViewsLut.find(attachment->handle); it != m_imageViewsLut.end())
                {
                    passAttachment.view = it->second;
                }
                else
                {
                    passAttachment.view = m_context->CreateImageView(passAttachment.attachment->handle, passAttachment.info);
                    m_imageViewsLut.insert({ attachment->handle, passAttachment.view });
                }
            }

            for (auto& passAttachment : pass->m_bufferPassAttachment)
            {
                auto attachment = passAttachment.attachment;

                if (auto it = m_bufferViewLut.find(attachment->handle); it != m_bufferViewLut.end())
                {
                    passAttachment.view = it->second;
                }
                else
                {
                    passAttachment.view = m_context->CreateBufferView(passAttachment.attachment->handle, passAttachment.info);
                    m_bufferViewLut.insert({ attachment->handle, passAttachment.view });
                }
            }
        }
    }

    void FrameScheduler::Execute()
    {
        for (auto pass : m_passList)
        {
            ExecutePass(*pass);
        }

        for (auto attachment : m_attachmentsRegistry->m_swapchainAttachments)
        {
            auto swapchain = attachment->swapchain;
            auto result    = swapchain->Present(*attachment->lastUse->pass);
            RHI_ASSERT(result == ResultCode::Success);
        }
    }

} // namespace RHI