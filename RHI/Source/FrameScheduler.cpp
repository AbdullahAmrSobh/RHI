#include "RHI/FrameScheduler.hpp"

#include "RHI/Context.hpp"

namespace RHI
{
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

    ImagePassAttachment* Pass::ImportSwapchainImageResource(Swapchain* swapchain, const ImageAttachmentUseInfo& useInfo)
    {
        m_swapchain = swapchain;
        return UseAttachment(swapchain->GetImage(), useInfo);
    }

    ImagePassAttachment* Pass::ImportImageResource(Handle<Image> image, const ImageAttachmentUseInfo& useInfo)
    {
        return UseAttachment(image, useInfo);
    }

    BufferPassAttachment* Pass::ImportBufferResource(Handle<Buffer> buffer, const BufferAttachmentUseInfo& useInfo)
    {
        return UseAttachment(buffer, useInfo);
    }

    ImagePassAttachment* Pass::CreateTransientImageResource(const ImageCreateInfo& createInfo, const ImageAttachmentUseInfo& useInfo)
    {
        auto image = m_scheduler->m_transientAttachmentAllocator->CreateTransientImage(createInfo);
        return UseAttachment(image, useInfo);
    }

    BufferPassAttachment* Pass::CreateTransientBufferResource(const BufferCreateInfo& createInfo, const BufferAttachmentUseInfo& useInfo)
    {
        auto buffer = m_scheduler->m_transientAttachmentAllocator->CreateTransientBuffer(createInfo);
        return UseAttachment(buffer, useInfo);
    }

    ImagePassAttachment* Pass::UseImageResource(ImagePassAttachment* attachment, const ImageAttachmentUseInfo& useInfo)
    {
        return UseAttachment(attachment->resourceHandle, useInfo);
    }

    BufferPassAttachment* Pass::UseBufferResource(BufferPassAttachment* attachment, const BufferAttachmentUseInfo& useInfo)
    {
        return UseAttachment(attachment->resourceHandle, useInfo);
    }

    ImagePassAttachment* Pass::UseAttachment(Handle<Image> handle, const ImageAttachmentUseInfo& useInfo)
    {
        auto image = m_context->AccessImage(handle);

        ImagePassAttachment& passAttachment = m_imagePassAttachments.emplace_back(ImagePassAttachment{});
        passAttachment.resourceHandle = handle;
        passAttachment.pass = this;
        passAttachment.info = useInfo;
        passAttachment.next = nullptr;
        passAttachment.prev = image->lastUse;

        if (image->lifetime == RHI::Lifetime::Persistent && image->swapchain == nullptr)
        {
            passAttachment.viewHandle = m_context->CreateImageView(handle, passAttachment.info);
        }

        if (image->firstUse == nullptr)
        {
            image->firstUse = &passAttachment;
            image->lastUse = &passAttachment;
        }
        else
        {
            image->lastUse->next = &passAttachment;
            image->lastUse = image->lastUse->next;
        }

        return &passAttachment;
    }

    BufferPassAttachment* Pass::UseAttachment(Handle<Buffer> handle, const BufferAttachmentUseInfo& useInfo)
    {
        auto buffer = m_context->AccessBuffer(handle);

        BufferPassAttachment& passAttachment = m_bufferPassAttachment.emplace_back(BufferPassAttachment{});
        passAttachment.resourceHandle = handle;
        passAttachment.pass = this;
        passAttachment.info = useInfo;
        passAttachment.next = nullptr;
        passAttachment.prev = buffer->lastUse;

        // if (buffer->lifetime == RHI::Lifetime::Persistent)
        // {
        //     // passAttachment.view = m_context->CreateBufferView(buffer->handle, passAttachment.info);
        // }
        if (buffer->firstUse == nullptr)
        {
            buffer->firstUse = &passAttachment;
            buffer->lastUse = &passAttachment;
        }
        else
        {
            buffer->lastUse->next = &passAttachment;
            buffer->lastUse = buffer->lastUse->next;
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

    void FrameScheduler::Compile()
    {
        CompileTransientAttachments();

        CompileAttachmentsViews();

        m_state = FrameGraphState::Ready;
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
                auto image = m_context->AccessImage(passAttachment.resourceHandle);

                if (image->lifetime != Lifetime::Transient)
                    continue;
                else if (passAttachment.prev != nullptr)
                    continue;

                m_transientAttachmentAllocator->Activate(image);
            }

            for (auto& passAttachment : pass->m_bufferPassAttachment)
            {
                auto buffer = m_context->AccessBuffer(passAttachment.resourceHandle);

                if (buffer->lifetime != Lifetime::Transient)
                    continue;
                else if (passAttachment.prev != nullptr)
                    continue;

                m_transientAttachmentAllocator->Activate(buffer);
            }

            for (auto& passAttachment : pass->m_imagePassAttachments)
            {
                auto image = m_context->AccessImage(passAttachment.resourceHandle);

                if (image->lifetime != Lifetime::Transient)
                    continue;
                else if (passAttachment.next != nullptr)
                    continue;

                m_transientAttachmentAllocator->Deactivate(image);
            }

            for (auto& passAttachment : pass->m_bufferPassAttachment)
            {
                auto buffer = m_context->AccessBuffer(passAttachment.resourceHandle);

                if (buffer->lifetime != Lifetime::Transient)
                    continue;
                else if (passAttachment.next != nullptr)
                    continue;

                m_transientAttachmentAllocator->Deactivate(buffer);
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
                auto image = m_context->AccessImage(passAttachment.resourceHandle);

                if (auto swapchain = image->swapchain)
                {
                    // attachment->handle = swapchain->GetImage();
                }

                if (passAttachment.viewHandle)
                    continue;

                if (auto it = m_imageViewsLut.find(passAttachment.resourceHandle); it != m_imageViewsLut.end())
                {
                    passAttachment.viewHandle = it->second;
                }
                else
                {
                    passAttachment.viewHandle = m_context->CreateImageView(passAttachment.resourceHandle, passAttachment.info);
                    m_imageViewsLut.insert({ passAttachment.resourceHandle, passAttachment.viewHandle });
                }
            }

            for (auto& passAttachment : pass->m_bufferPassAttachment)
            {
                // auto attachment = passAttachment.attachment;

                if (passAttachment.viewHandle)
                    continue;

                if (auto it = m_bufferViewLut.find(passAttachment.resourceHandle); it != m_bufferViewLut.end())
                {
                    passAttachment.viewHandle = it->second;
                }
                else
                {
                    // passAttachment.view = m_context->CreateBufferView(passAttachment.attachment->handle, passAttachment.info);
                    // m_bufferViewLut.insert({ attachment->handle, passAttachment.view });
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
    }

} // namespace RHI