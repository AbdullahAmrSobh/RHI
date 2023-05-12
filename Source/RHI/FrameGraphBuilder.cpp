#include "RHI/Pch.hpp"

#include "RHI/Common.hpp"

#include "RHI/FrameGraphBuilder.hpp"

namespace RHI
{
// todo
void FrameGraphBuilder::Begin()
{
    m_renderpasses.clear();
}

void FrameGraphBuilder::End()
{
}

void FrameGraphBuilder::BeginPass(IRenderPass& renderpass)
{
    m_renderpasses.push_back(&renderpass);
}

void FrameGraphBuilder::EndPass()
{
}

void FrameGraphBuilder::UseImageAttachment(std::string                   attachmentName,
                                           ImageViewDesc                 imageViewDesc,
                                           AttachmentLoadStoreOperations loadStoreOperations,
                                           AttachmentUsage               usage,
                                           AttachmentAccess              access)
{
    IRenderPass&     renderpass = *m_renderpasses.back();
    ImageAttachment& attachment = *m_registry->FindImageAttachment(attachmentName);
    assert(&attachment != nullptr);

    UsedImageAttachment* attachmentInstance =
        attachment.Use(UsedImageAttachment(renderpass, attachment, imageViewDesc, loadStoreOperations, usage, access));
    renderpass.m_usedImageAttachments.push_back(attachmentInstance);

    if (attachment.GetSwapchain())
    {
        renderpass.m_usedSwapchainAttachment = attachmentInstance;
    }
}

}  // namespace RHI