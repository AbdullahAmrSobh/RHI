#pragma once
#include "RHI/Common.hpp"
#include "RHI/Handle.hpp"
#include "RHI/HandleTypes.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Attachment.hpp"

namespace RHI
{

class FrameGraphBuilder
{
public:
    Handle<ImageView> ImportImage(std::string name, const ImageAttachmentImportInfo& importInfo);

    Handle<BufferView> ImportBuffer(std ::string name, const BufferAttachmentImportInfo& importInfo);

    Handle<ImageView> CreateTransientImageAttachment(std::string name, const ImageCreateInfo& createInfo);

    Handle<BufferView> CreateTransientBufferAttachment(std::string name, const BufferCreateInfo& createInfo);

    Handle<ImageView> UseImageAttachment(std::string name, const ImageAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access);

    Handle<BufferView> UseBufferAttachment(std::string name, const BufferAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access);

    void ExecuteAfter(std::string name);

    void ExecuteBefore(std::string name);

    Handle<ImageView> UseRenderTarget(std::string name, const ImageAttachmentUseInfo& useInfo)
    {
        return UseImageAttachment(name, useInfo, AttachmentUsage::RenderTarget, AttachmentAccess::Write);
    }

    Handle<ImageView> UseDepthTarget(std::string name, const ImageAttachmentUseInfo& useInfo)
    {
        return UseImageAttachment(name, useInfo, AttachmentUsage::Depth, AttachmentAccess::Write);
    }

    Handle<ImageView> UseStencilTarget(std::string name, const ImageAttachmentUseInfo& useInfo)
    {
        return UseImageAttachment(name, useInfo, AttachmentUsage::Stencil, AttachmentAccess::Write);
    }

    Handle<ImageView> UseDepthStencilTarget(std::string name, const ImageAttachmentUseInfo& useInfo)
    {
        return UseImageAttachment(name, useInfo, AttachmentUsage::DepthStencil, AttachmentAccess::Write);
    }

    Handle<BufferView> UseBufferCopySrc(std::string name, const BufferAttachmentUseInfo& useInfo)
    {
        return UseBufferAttachment(name, useInfo, AttachmentUsage::Copy, AttachmentAccess::Read);
    }

    Handle<BufferView> UseBufferCopyDst(std::string name, const BufferAttachmentUseInfo& useInfo)
    {
        return UseBufferAttachment(name, useInfo, AttachmentUsage::Copy, AttachmentAccess::Write);
    }

    Handle<ImageView> UseImageCopyDst(std::string name, const ImageAttachmentUseInfo& useInfo)
    {
        return UseImageAttachment(name, useInfo, AttachmentUsage::Copy, AttachmentAccess::Write);
    }

    Handle<ImageView> UseImageShaderInput(std::string name, const ImageAttachmentUseInfo& useInfo)
    {
        return UseImageAttachment(name, useInfo, AttachmentUsage::ShaderResource, AttachmentAccess::Write);
    }

    Handle<BufferView> UseBufferShaderInput(std::string name, const BufferAttachmentUseInfo& useInfo)
    {
        return UseBufferAttachment(name, useInfo, AttachmentUsage::ShaderResource, AttachmentAccess::Write);
    }

    Handle<ImageView> UseRWImageShaderInput(std::string name, const ImageAttachmentUseInfo& useInfo)
    {
        return UseImageAttachment(name, useInfo, AttachmentUsage::ShaderResource, AttachmentAccess::ReadWrite);
    }

    Handle<BufferView> UseRWBufferShaderInput(std::string name, const BufferAttachmentUseInfo& useInfo)
    {
        return UseBufferAttachment(name, useInfo, AttachmentUsage::ShaderResource, AttachmentAccess::ReadWrite);
    }
};

}  // namespace RHI