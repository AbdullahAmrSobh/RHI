#include "RHI/Attachments.hpp"
#include "RHI/Context.hpp"

namespace RHI
{
    void Attachment::Insert(PassAttachment* passAttachment)
    {
        m_referenceCount++;

        if (m_firstPassAttachment == nullptr && m_lastPassAttachment == nullptr)
        {
            m_firstPassAttachment = passAttachment;
            m_lastPassAttachment = passAttachment;
        }
        else
        {
            m_lastPassAttachment->m_next = passAttachment;
            passAttachment->m_prev = m_lastPassAttachment;
            m_lastPassAttachment = passAttachment;
        }

        if (m_lifetime == Lifetime::Transient)
        {
            if (m_type == Type::Image)
            {
                auto imagePassAttachment = (ImagePassAttachment*)passAttachment;
                m_asImage.info.usageFlags |= imagePassAttachment->m_usage;
                m_asImage.info.mipLevels = std::max(m_asImage.info.mipLevels, imagePassAttachment->m_viewInfo.subresource.mipLevelCount);
                m_asImage.info.arrayCount = std::max(m_asImage.info.arrayCount, imagePassAttachment->m_viewInfo.subresource.arrayCount);
            }
            else if (m_type == Type::Buffer)
            {
                auto bufferPassAttachment = (BufferPassAttachment*)passAttachment;
                m_asBuffer.info.usageFlags |= bufferPassAttachment->m_usage;
            }
            else
            {
                RHI_UNREACHABLE();
            }
        }
    }

    void Attachment::Remove(PassAttachment* passAttachment)
    {
        m_referenceCount--;

        auto prev = passAttachment->m_prev;
        auto next = passAttachment->m_next;

        if (prev)
            prev->m_next = next;
        else
            m_firstPassAttachment = next;

        if (next)
            next->m_prev = prev;
        else
            m_lastPassAttachment = prev;

        if (m_lifetime == Lifetime::Transient)
        {
            if (m_type == Type::Image)
            {
                auto format = m_asImage.info.format;
                m_asImage.info = {};
                m_asImage.info.format = format;
            }
            else if (m_type == Type::Buffer)
            {
                m_asBuffer.info = {};
            }
            else
            {
                RHI_UNREACHABLE();
            }

            for (auto passAttachment2 = m_firstPassAttachment;
                 passAttachment2 != nullptr;
                 passAttachment2 = passAttachment2->GetNext())
            {
                if (m_type == Type::Image)
                {
                    auto imagePassAttachment = (ImagePassAttachment*)passAttachment2;
                    m_asImage.info.usageFlags |= imagePassAttachment->m_usage;
                    m_asImage.info.mipLevels = std::max(m_asImage.info.mipLevels, imagePassAttachment->m_viewInfo.subresource.mipLevelCount);
                    m_asImage.info.arrayCount = std::max(m_asImage.info.arrayCount, imagePassAttachment->m_viewInfo.subresource.arrayCount);
                }
                else if (m_type == Type::Buffer)
                {
                    auto bufferPassAttachment = (BufferPassAttachment*)passAttachment2;
                    m_asBuffer.info.usageFlags |= bufferPassAttachment->m_usage;
                }
                else
                {
                    RHI_UNREACHABLE();
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// AttachmentsPool
    ///////////////////////////////////////////////////////////////////////////

    AttachmentsPool::AttachmentsPool(Context* context)
        : m_context(context)
    {
    }

    void AttachmentsPool::InitPassAttachment(PassAttachment* passAttachment)
    {
        switch (passAttachment->GetAttachment()->m_type)
        {
        case Attachment::Type::Image:
            {
                auto imagePassAttachment = (ImagePassAttachment*)passAttachment;
                if (imagePassAttachment->GetAttachment()->m_swapchain)
                    return;

                imagePassAttachment->m_viewInfo.image = imagePassAttachment->GetAttachment()->GetHandle();
                imagePassAttachment->m_view = m_context->CreateImageView(imagePassAttachment->m_viewInfo);
                break;
            }
        case Attachment::Type::Buffer:
            {
                auto bufferPassAttachment = (BufferPassAttachment*)passAttachment;
                bufferPassAttachment->m_viewInfo.buffer = bufferPassAttachment->GetAttachment()->GetHandle();
                bufferPassAttachment->m_view = m_context->CreateBufferView(bufferPassAttachment->m_viewInfo);
                break;
            }
        default:
            {
                RHI_UNREACHABLE();
                break;
            }
        }
    }

    void AttachmentsPool::ShutdownPassAttachment(PassAttachment* passAttachment)
    {
        switch (passAttachment->GetAttachment()->m_type)
        {
        case Attachment::Type::Image:
            {
                auto imagePassAttachment = (ImagePassAttachment*)passAttachment;
                m_context->DestroyImageView(imagePassAttachment->m_view);
                break;
            }
        case Attachment::Type::Buffer:
            {
                auto bufferPassAttachment = (BufferPassAttachment*)passAttachment;
                m_context->DestroyBufferView(bufferPassAttachment->m_view);
                break;
            }
        default:
            {
                RHI_UNREACHABLE();
                break;
            }
        }
    }

    ImageAttachment* AttachmentsPool::CreateAttachment(const char* name, Swapchain* swapchain)
    {
        auto attachment = CreateAttachment(name, swapchain->GetImage());
        attachment->m_swapchain = swapchain;
        attachment->m_asImage.info.type = ImageType::Image2D;
        return attachment;
    }

    ImageAttachment* AttachmentsPool::CreateAttachment(const char* name, Handle<Image> handle)
    {
        auto attachment = (ImageAttachment*)m_attachmentsLut.insert(std::make_pair(name, CreatePtr<ImageAttachment>(name, handle))).first->second.get();
        m_attachments.push_back(attachment);
        m_imageAttachments.push_back(attachment);
        return attachment;
    }

    ImageAttachment* AttachmentsPool::CreateAttachment(const ImageCreateInfo& createInfo)
    {
        auto attachment = (ImageAttachment*)m_attachmentsLut.insert(std::make_pair(createInfo.debugName, CreatePtr<ImageAttachment>(createInfo.debugName, createInfo))).first->second.get();
        m_attachments.push_back(attachment);
        m_imageAttachments.push_back(attachment);
        m_transientAttachments.push_back(attachment);
        return attachment;
    }

    BufferAttachment* AttachmentsPool::CreateAttachment(const char* name, Handle<Buffer> handle)
    {
        auto attachment = (BufferAttachment*)m_attachmentsLut.insert(std::make_pair(name, CreatePtr<BufferAttachment>(name, handle))).first->second.get();
        m_attachments.push_back(attachment);
        m_bufferAttachments.push_back(attachment);
        return attachment;
    }

    BufferAttachment* AttachmentsPool::CreateAttachment(const BufferCreateInfo& createInfo)
    {
        auto attachment = (BufferAttachment*)m_attachmentsLut.insert(std::make_pair(createInfo.debugName, CreatePtr<BufferAttachment>(createInfo.debugName, createInfo.byteSize))).first->second.get();
        m_attachments.push_back(attachment);
        m_transientAttachments.push_back(attachment);
        m_bufferAttachments.push_back(attachment);
        return attachment;
    }

    void AttachmentsPool::DestroyAttachment(Attachment* attachment)
    {
        for (auto passAttachment = attachment->GetFirstPassAttachment(); passAttachment != nullptr; passAttachment = passAttachment->GetNext())
        {
            ShutdownPassAttachment(passAttachment);
        }

        switch (attachment->m_type)
        {
        case Attachment::Type::Image:
            {
                auto imageAttachment = (ImageAttachment*)attachment;
                m_context->DestroyImage(imageAttachment->GetHandle());
                break;
            }
        case Attachment::Type::Buffer:
            {
                auto bufferAttachment = (BufferAttachment*)attachment;
                m_context->DestroyBuffer(bufferAttachment->GetHandle());
                break;
            }
        default:
            {
                RHI_UNREACHABLE();
                break;
            }
        }
    }

    Handle<ImageView> AttachmentsPool::GetImageView(ImagePassAttachment* passAttachment)
    {
        passAttachment->m_viewInfo.image = passAttachment->GetAttachment()->GetHandle();

        if (auto swapchain = passAttachment->GetAttachment()->m_swapchain)
        {
            return swapchain->GetImageView(m_context, passAttachment->m_viewInfo);
        }

        return passAttachment->m_view;
    }

    Handle<BufferView> AttachmentsPool::GetBufferView(BufferPassAttachment* passAttachment)
    {
        return passAttachment->m_view;
    }

} // namespace RHI