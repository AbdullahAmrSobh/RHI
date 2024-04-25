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
} // namespace RHI