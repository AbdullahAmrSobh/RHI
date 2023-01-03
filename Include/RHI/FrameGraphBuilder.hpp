#pragma once
#include "RHI/AttachmentsRegistry.hpp"
#include "RHI/RenderPass.hpp"

namespace RHI
{
class FrameGraphBuilder
{
public:
    friend class IFrameScheduler;

    FrameGraphBuilder(AttachmentsRegistry& registry)
        : m_registry(&registry)
    {
    }

private:
    void Begin();
    void End();

    void BeginPass(IRenderPass& renderpass);
    void EndPass();

public:
    void UseImageAttachment(std::string                   attachmentName,
                            ImageViewDesc                 attachmentViewDesc,
                            AttachmentLoadStoreOperations loadStoreOperations = AttachmentLoadStoreOperations(),
                            AttachmentUsage               usage               = AttachmentUsage::RenderTarget,
                            AttachmentAccess              access              = AttachmentAccess::Write);

private:
    AttachmentsRegistry* m_registry;

    std::vector<IRenderPass*> m_renderpasses;
};
}  // namespace RHI