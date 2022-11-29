#pragma once
#include "RHI/FrameGraphAttachmentsRegistry.hpp"

namespace RHI
{

Expected<ImageAttachmentReference> IAttachmentsRegistry::CreateTransientImageAttachment(const ImageFrameAttachmentDesc& description)
{
    return Unexpected(EResultCode::Fail);
}

Expected<BufferAttachmentReference> IAttachmentsRegistry::CreateTransientBufferAttachment(const BufferFrameAttachmentDesc& description)
{
    return Unexpected(EResultCode::Fail);
}

ImageAttachmentReference IAttachmentsRegistry::FindImageReference(const std::string& name) const
{
    return ImageAttachmentReference::Null();
}

BufferAttachmentReference IAttachmentsRegistry::FindBufferReference(const std::string& name) const
{
    return BufferAttachmentReference::Null();
}

SwapchainAttachmentReference IAttachmentsRegistry::FindSwapchainReference(const std::string& name) const
{
    return SwapchainAttachmentReference::Null(); 
}

SwapchainAttachmentReference IAttachmentsRegistry::ImportSwapchain(std::string name, Unique<ISwapchain>& swapchain)
{
    return SwapchainAttachmentReference::Null();
}

ImageFrameAttachment* IAttachmentsRegistry::GetImageFrameAttachment(ImageAttachmentReference reference)
{
    return nullptr;
}

BufferFrameAttachment* IAttachmentsRegistry::GetBufferFrameAttachment(BufferAttachmentReference reference)
{
    return nullptr;
}


} // namespace RHI