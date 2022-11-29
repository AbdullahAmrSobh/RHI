#pragma once
#include "RHI/FrameGraph.hpp"
#include "RHI/FrameGraphInterface.hpp"

namespace RHI
{

const IAttachmentsRegistry& FrameGraphBuilder::GetAttachmentsRegistry() const
{
    return m_pFrameGraph->GetAttachmentsRegistry();
}

EResultCode FrameGraphBuilder::UseImageAttachment(const ImagePassAttachmentDesc& description, EAttachmentUsage usage, EAttachmentAccess access)
{
    return EResultCode::Fail;
}

EResultCode FrameGraphBuilder::UseBufferAttachment(const BufferPassAttachmentDesc& description, EAttachmentUsage usage, EAttachmentAccess access)
{
    return EResultCode::Fail;
}

EResultCode FrameGraphBuilder::UseDepthStencilAttachment(const ImagePassAttachmentDesc& description, EAttachmentAccess access)
{
    return EResultCode::Fail;
}

EResultCode FrameGraphBuilder::ExecuteAfter(const IPass& pass)
{
    return EResultCode::Fail;
}

} // namespace RHI