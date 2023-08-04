#include "RHI/FrameGraphAttachments.hpp"

#include "RHI/Debug.hpp"
#include "RHI/FrameGraph.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

Attachment::Attachment(AttachmentName name, Image& image)
    : name(name)
    , lifetime(AttachmentLifetime::Persistent)
    , type(AttachmentType::Image)
    , image(&image)
{
}

Attachment::Attachment(AttachmentName name, Buffer& buffer)
    : name(name)
    , lifetime(AttachmentLifetime::Persistent)
    , type(AttachmentType::Buffer)
    , buffer(&buffer)
{
}

Attachment::Attachment(AttachmentName name, Swapchain& swapchain)
    : name(name)
    , lifetime(AttachmentLifetime::Persistent)
    , type(AttachmentType::Swapchain)
    , swapchain(&swapchain)
{
}

Attachment::Attachment(AttachmentName name, const ImageCreateInfo& info)
    : name(name)
    , lifetime(AttachmentLifetime::Transient)
    , type(AttachmentType::Image)
    , imageInfo(info)
{
}

Attachment::Attachment(AttachmentName name, const BufferCreateInfo& info)
    : name(name)
    , lifetime(AttachmentLifetime::Transient)
    , type(AttachmentType::Buffer)
    , bufferInfo(info)
{
}

PassAttachment* Attachment::Use(Pass& pass, AttachmentUsage usage, AttachmentAccess access, const ImageAttachmentUseInfo& useInfo)
{
    auto& it = useList.emplace_back(std::make_unique<PassAttachment>(this, &pass, usage, access, useInfo));
    return it.get();
}

PassAttachment* Attachment::Use(Pass& pass, AttachmentUsage usage, AttachmentAccess access, const BufferAttachmentUseInfo& useInfo)
{
    auto& it = useList.emplace_back(std::make_unique<PassAttachment>(this, &pass, usage, access, useInfo));
    return it.get();
}

PassAttachment::PassAttachment(Attachment*                   attachment,
                               Pass*                         pass,
                               AttachmentUsage               usage,
                               AttachmentAccess              access,
                               const ImageAttachmentUseInfo& useInfo)
    : attachment(attachment)
    , pass(pass)
    , usage(usage)
    , access(access)
    , nextUse(nullptr)
    , pervUse(nullptr)
    , imageView(nullptr)
    , imageInfo(useInfo)
{
    if (attachment->useList.size() > 0)
    {
        pervUse = attachment->useList.back().get();
    }
}

PassAttachment::PassAttachment(Attachment*                    attachment,
                               Pass*                          pass,
                               AttachmentUsage                usage,
                               AttachmentAccess               access,
                               const BufferAttachmentUseInfo& useInfo)
    : attachment(attachment)
    , pass(pass)
    , usage(usage)
    , access(access)
    , nextUse(nullptr)
    , pervUse(nullptr)
    , bufferView(nullptr)
    , bufferInfo(useInfo)
{
    if (attachment->useList.size() > 0)
    {
        pervUse = attachment->useList.back().get();
    }
}

void AttachmentsRegistry::Reset()
{
    m_attachmentsLookup.clear();
    m_imageAttachments.clear();
    m_bufferAttachments.clear();
    m_swapchainAttachments.clear();
}

void AttachmentsRegistry::ImportImage(std::string name, Image& image)
{
    RHI_ASSERT_MSG(m_attachmentsLookup.find(name) == m_attachmentsLookup.end(), "Attachments with the same name already exists");

    auto it            = m_attachmentsLookup.emplace(name, Attachment(name.data(), image));
    auto attachmentPtr = &(*it.first).second;

    m_imageAttachments.push_back(attachmentPtr);
}

void AttachmentsRegistry::ImportBuffer(std::string name, Buffer& buffer)
{
    RHI_ASSERT_MSG(m_attachmentsLookup.find(name) == m_attachmentsLookup.end(), "Attachments with the same name already exists");

    auto it            = m_attachmentsLookup.emplace(name, Attachment(name.data(), buffer));
    auto attachmentPtr = &(*it.first).second;

    m_bufferAttachments.push_back(attachmentPtr);
}

void AttachmentsRegistry::ImportSwapchain(std::string name, Swapchain& swapchain)
{
    RHI_ASSERT_MSG(m_attachmentsLookup.find(name) == m_attachmentsLookup.end(), "Attachments with the same name already exists");

    auto it            = m_attachmentsLookup.emplace(name, Attachment(name.data(), swapchain));
    auto attachmentPtr = &(*it.first).second;

    m_swapchainAttachments.push_back(attachmentPtr);
}

void AttachmentsRegistry::CreateTransientImageAttachment(std::string name, const ImageCreateInfo& createInfo)
{
    RHI_ASSERT_MSG(m_attachmentsLookup.find(name) == m_attachmentsLookup.end(), "Attachments with the same name already exists");

    auto it            = m_attachmentsLookup.emplace(name, Attachment(name.data(), createInfo));
    auto attachmentPtr = &(*it.first).second;

    m_imageAttachments.push_back(attachmentPtr);
    m_transientResources.push_back(attachmentPtr);
}

void AttachmentsRegistry::CreateTransientBufferAttachment(std::string name, const BufferCreateInfo& createInfo)
{
    RHI_ASSERT_MSG(m_attachmentsLookup.find(name) == m_attachmentsLookup.end(), "Attachments with the same name already exists");

    auto it            = m_attachmentsLookup.emplace(name, Attachment(name.data(), createInfo));
    auto attachmentPtr = &(*it.first).second;

    m_bufferAttachments.push_back(attachmentPtr);
    m_transientResources.push_back(attachmentPtr);
}

}  // namespace RHI