#include "RHI/Attachments.hpp"

namespace RHI
{

    //////////////////////////////////////////////////////////////////////////////////////////
    /// ImageAttachment
    //////////////////////////////////////////////////////////////////////////////////////////

    ImageAttachment::ImageAttachment(const char* name, Handle<Image> handle)
        : Attachment(name, Attachment::Lifetime::Persistent, Attachment::Type::Image)
        , swapchain(nullptr)
        , handle(handle)
        , info()
        , firstUse(nullptr)
        , lastUse(nullptr)
    {
    }

    ImageAttachment::ImageAttachment(const char* name, Swapchain* swapchain)
        : Attachment(name, Attachment::Lifetime::Persistent, Attachment::Type::Image)
        , swapchain(swapchain)
        , handle(swapchain->GetImage())
        , info()
        , firstUse(nullptr)
        , lastUse(nullptr)
    {
    }

    ImageAttachment::ImageAttachment(const char* name, const ImageCreateInfo& createInfo)
        : Attachment(name, Attachment::Lifetime::Transient, Attachment::Type::Image)
        , swapchain(nullptr)
        , handle()
        , info(createInfo)
        , firstUse(nullptr)
        , lastUse(nullptr)
    {
    }

    void ImageAttachment::PushPassAttachment(ImagePassAttachment* passAttachment)
    {
        if (firstUse && lastUse)
        {
            lastUse->next = passAttachment;
            lastUse = lastUse->next;
        }
        else
        {
            firstUse = passAttachment;
            lastUse = passAttachment;
        }
    }

    void ImageAttachment::Reset()
    {
        firstUse = nullptr;
        lastUse = nullptr;
    }

    Handle<Image> ImageAttachment::GetImage()
    {
        if (swapchain)
        {
            return swapchain->GetImage();
        }

        return handle;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// BufferAttachment
    //////////////////////////////////////////////////////////////////////////////////////////

    BufferAttachment::BufferAttachment(const char* name, Handle<Buffer> handle)
        : Attachment(name, Attachment::Lifetime::Persistent, Attachment::Type::Buffer)
        , handle(handle)
        , info()
        , firstUse(nullptr)
        , lastUse(nullptr)
    {
    }

    BufferAttachment::BufferAttachment(const char* name, const BufferCreateInfo& createInfo)
        : Attachment(name, Attachment::Lifetime::Transient, Attachment::Type::Buffer)
        , handle()
        , info(createInfo)
        , firstUse(nullptr)
        , lastUse(nullptr)
    {
    }

    void BufferAttachment::PushPassAttachment(BufferPassAttachment* passAttachment)
    {
        if (firstUse && lastUse)
        {
            lastUse->next = passAttachment;
            lastUse = lastUse->next;
        }
        else
        {
            firstUse = passAttachment;
            lastUse = passAttachment;
        }
    }

    void BufferAttachment::Reset()
    {
        firstUse = nullptr;
        lastUse = nullptr;
    }

    Handle<Image> BufferAttachment::GetBuffer()
    {
        return handle;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// AttachmentsRegistry
    //////////////////////////////////////////////////////////////////////////////////////////

    void AttachmentsRegistry::Reset()
    {
        m_imageAttachments.clear();
        m_bufferAttachments.clear();
        m_swapchainAttachments.clear();
    }

    ImageAttachment* AttachmentsRegistry::ImportSwapchainImage(const char* name, Swapchain* swapchain)
    {
        auto& attachment = m_imageAttachments[name] = std::make_unique<ImageAttachment>(name, swapchain);
        m_swapchainAttachments.push_back(name);
        return attachment.get();
    }

    ImageAttachment* AttachmentsRegistry::ImportImage(const char* name, Handle<Image> handle)
    {
        auto& attachment = m_imageAttachments[name] = std::make_unique<ImageAttachment>(name, handle);
        return attachment.get();
    }

    BufferAttachment* AttachmentsRegistry::ImportBuffer(const char* name, Handle<Buffer> handle)
    {
        auto& attachment = m_bufferAttachments[name] = std::make_unique<BufferAttachment>(name, handle);
        return attachment.get();
    }

    ImageAttachment* AttachmentsRegistry::CreateTransientImage(const char* name, const ImageCreateInfo& createInfo)
    {
        auto& attachment = m_imageAttachments[name] = std::make_unique<ImageAttachment>(name, createInfo);
        return attachment.get();
    }

    BufferAttachment* AttachmentsRegistry::CreateTransientBuffer(const char* name, const BufferCreateInfo& createInfo)
    {
        auto& attachment = m_bufferAttachments[name] = std::make_unique<BufferAttachment>(name, createInfo);
        return attachment.get();
    }

    ImageAttachment* AttachmentsRegistry::FindImage(AttachmentID id)
    {
        if (auto it = m_imageAttachments.find(id); it != m_imageAttachments.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    BufferAttachment* AttachmentsRegistry::FindBuffer(AttachmentID id)
    {
        if (auto it = m_bufferAttachments.find(id); it != m_bufferAttachments.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

}