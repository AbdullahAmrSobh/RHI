#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraph.hpp"
#include "RHI/FrameGraphAttachment.hpp"
#include "RHI/FrameGraphPass.hpp"

namespace RHI
{

class IFrameGraph;
class IPass;

// Interface for accessing Pass's attachments
class FrameGraphContext
{
public:
    inline Expected<ImagePassAttachment*> GetImagePassAttachment(std::string name) const
    {
        return Unexpected(EResultCode::Fail);
    }

    inline Expected<ImagePassAttachment*> GetImagePassAttachment(ImageAttachmentReference) const
    {
        return Unexpected(EResultCode::Fail);
    }

    inline Expected<BufferPassAttachment*> GetBufferPassAttachment(std::string name) const
    {
        return Unexpected(EResultCode::Fail);
    }

    inline Expected<BufferPassAttachment*> GetBufferPassAttachment(ImageAttachmentReference) const
    {
        return Unexpected(EResultCode::Fail);
    }
};

// Interface for building the framegraph.
class FrameGraphBuilder
{
public:
    FrameGraphBuilder(IFrameGraph* frameGraph);

    Expected<ImageAttachmentReference> FindAttachmentReference(std::string attachmentName) const
    {
        return Unexpected(EResultCode::Fail);
    }

    EResultCode UseImageAttachment(const ImagePassAttachmentDesc& attachmentDesc, EAttachmentUsage usage, EAttachmentAccess access)
    {
        return EResultCode::Fail;
    }

    EResultCode UseImageAttachments(const std::vector<ImagePassAttachmentDesc>& attachmentDesc, EAttachmentUsage usage, EAttachmentAccess access)
    {
        return EResultCode::Fail;
    }

    EResultCode UseRenderTargetAttachments(const std::vector<ImagePassAttachmentDesc>& attachmentDesc, EAttachmentAccess access)
    {
        return EResultCode::Fail;
    }

    EResultCode UseRenderTargetAttachment(const std::vector<ImagePassAttachmentDesc>& attachmentDesc, EAttachmentAccess access)
    {
        return EResultCode::Fail;
    }

    EResultCode UseDepthStencilAttachment(const ImagePassAttachmentDesc& attachmentDesc, EAttachmentAccess access)
    {
        return EResultCode::Fail;
    }

    EResultCode UseBufferAttachemnt(const BufferPassAttachment& attachmentDesc, EAttachmentUsage usage, EAttachmentAccess access)
    {
        return EResultCode::Fail;
    }

    EResultCode WaitForPass(const IPass& pass)
    {
        return EResultCode::Fail;
    }

private:
    IFrameGraph* m_pFrameGraph;
    IPass*       m_pass;
};

} // namespace RHI