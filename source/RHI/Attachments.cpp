#pragma once
#include "RHI/Attachments.hpp"

namespace RHI
{

Attachment::Attachment(std::string name, const TransientImageCreateInfo createInfo);

Attachment::Attachment(std::string name, const TransientBufferCreateInfo createInfo);

Attachment::Attachment(std::string name, std::unique_ptr<Image> image)
    : m_name(std::move(name))
    , m_lifetime(AttachmentLifetime::Persistent)
    , m_type(ResourceType::Image)
    , m_asImage(std::move(image))
{
}

Attachment::Attachment(std::string name, std::unique_ptr<Buffer> buffer)
    : m_name(std::move(name))
    , m_lifetime(AttachmentLifetime::Persistent)
    , m_type(ResourceType::Buffer)
    , m_asBuffer(std::move(buffer))
{
}

const std::string& Attachment::GetName() const
{
    return m_name;
}

ResourceType Attachment::GetResourceType() const
{
    return m_type;
}

AttachmentLifetime Attachment::GetLifetimeType() const
{
    return m_lifetime;
}

bool Attachment::IsInitialized() const
{
    if (m_type == ResourceType::Image)
        return m_asImage != nullptr;
    else
        return m_asBuffer != nullptr;
}

ImageAttachment::ImageAttachment(std::string name, const TransientImageCreateInfo createInfo)
{
}

ImageAttachment::ImageAttachment(std::string name, std::unique_ptr<Image> image)
{
}

void ImageAttachment::SetImage(std::unique_ptr<Image> image)
{
}

const Image& ImageAttachment::GetImage() const
{
}

Image& ImageAttachment::GetImage()
{
}

const ImagePassAttachment* ImageAttachment::GetFirstUse() const
{
}

ImagePassAttachment* ImageAttachment::GetFirstUse()
{
}

const ImagePassAttachment* ImageAttachment::GetLastUse() const
{
}

ImagePassAttachment* ImageAttachment::GetLastUse()
{
}

const ImageCreateInfo& ImageAttachment::GetInfo() const
{
}

bool ImageAttachment::IsSwapchain() const
{
}

const Swapchain* ImageAttachment::GetSwapchain() const
{
}

Swapchain* ImageAttachment::GetSwapchain()
{
}

BufferAttachment::BufferAttachment(std::string name, const TransientBufferCreateInfo createInfo)
{
}

BufferAttachment::BufferAttachment(std::string name, std::unique_ptr<Buffer> buffer)
{
}

void BufferAttachment::SetBuffer(std::unique_ptr<Buffer> buffer)
{
}

const Buffer& BufferAttachment::GetBuffer() const
{
}

Buffer& BufferAttachment::GetBuffer()
{
}

const BufferPassAttachment* BufferAttachment::GetFirstUse() const
{
}

BufferPassAttachment* BufferAttachment::GetFirstUse()
{
}

const BufferPassAttachment* BufferAttachment::GetLastUse() const
{
}

BufferPassAttachment* BufferAttachment::GetLastUse()
{
}

const BufferCreateInfo& BufferAttachment::GetInfo() const
{
}

PassAttachment::PassAttachment()
{
}

bool PassAttachment::IsInitialized() const
{
}

ImagePassAttachment::ImagePassAttachment()
{
}

ImageAttachment& ImagePassAttachment::GetAttachment()
{
}

const ImagePassAttachment* ImagePassAttachment::GetPervUse() const
{
}

ImagePassAttachment* ImagePassAttachment::GetPervUse()
{
}

const ImagePassAttachment* ImagePassAttachment::GetNextUse() const
{
}

ImagePassAttachment* ImagePassAttachment::GetNextUse()
{
}

const ImageView& ImagePassAttachment::GetImagView() const
{
}

ImageView& ImagePassAttachment::GetImagView()
{
}

const ImageViewCreateInfo& ImagePassAttachment::GetImageInfo()
{
}

const Swapchain* ImagePassAttachment::GetSwapchain()
{
}

ImageAttachmentLoadStoreOperations ImagePassAttachment::GetLoadStoreoperations() const
{
}

BufferPassAttachment::BufferPassAttachment()
{
}

const BufferAttachment& BufferPassAttachment::GetAttachment() const
{
}

BufferAttachment& BufferPassAttachment::GetAttachment()
{
}

const BufferPassAttachment* BufferPassAttachment::GetPervUse() const
{
}

BufferPassAttachment* BufferPassAttachment::GetPervUse()
{
}

const BufferPassAttachment* BufferPassAttachment::GetNextUse() const
{
}

BufferPassAttachment* BufferPassAttachment::GetNextUse()
{
}

const BufferView& BufferPassAttachment::GetBuffeView() const
{
}

BufferView& BufferPassAttachment::GetBuffeView()
{
}

const BufferViewCreateInfo& BufferPassAttachment::GetBufferInfo()
{
}

AttachmentsRegistry::AttachmentsRegistry()
{
}

void AttachmentsRegistry::Reset()
{
}

void AttachmentsRegistry::ImportImage(std::string name, Image& image)
{
}

void AttachmentsRegistry::ImportBuffer(std::string name, Buffer& buffer)
{
}

void AttachmentsRegistry::ImportSwapchain(std::string name, Swapchain& swapchain)
{
}

void AttachmentsRegistry::CreateTransientImageAttachment(std::string name, const TransientImageCreateInfo& createInfo)
{
}

void AttachmentsRegistry::CreateTransientBufferAttachment(std::string name, const TransientBufferCreateInfo& createInfo)
{
}

const std::span<const Attachment*> AttachmentsRegistry::GetImportedAttachments() const
{
}

std::span<Attachment*> AttachmentsRegistry::GetImportedAttachments()
{
}

const std::span<const ImageAttachment*> AttachmentsRegistry::GetSwapchainAttachments() const
{
}

std::span<ImageAttachment*> AttachmentsRegistry::GetSwapchainAttachments()
{
}

const std::span<const Attachment*> AttachmentsRegistry::GetTransientAttachments() const
{
}

std::span<Attachment*> AttachmentsRegistry::GetTransientAttachments()
{
}

const std::span<const ImageAttachment*> AttachmentsRegistry::GetImageAttachments() const
{
}

std::span<ImageAttachment*> AttachmentsRegistry::GetImageAttachments()
{
}

const std::span<const BufferAttachment*> AttachmentsRegistry::GetBufferAttachments() const
{
}

std::span<BufferAttachment*> AttachmentsRegistry::GetBufferAttachments()
{
}

}  // namespace RHI