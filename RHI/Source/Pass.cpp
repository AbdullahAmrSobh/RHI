#include "RHI/Pass.hpp"

#include "RHI/FrameScheduler.hpp"

namespace RHI
{

void Pass::Begin()
{
    /// Unuse all attachments
    m_waitPasses.clear();
    m_usedImages.clear();
    m_usedBuffers.clear();
}

void Pass::End()
{
}

void Pass::ExecuteAfter(Pass& pass)
{
    m_waitPasses.push_back(&pass);
}

void Pass::ExecuteBefore(Pass& pass)
{
    pass.m_waitPasses.push_back(this);
}

ImagePassAttachment Pass::ImportImageResource(std::string name, Handle<Image> image, const ImageAttachmentUseInfo& useInfo)
{
    return {};
}

BufferPassAttachment Pass::ImportBufferResource(std::string name, Handle<Buffer> buffer, const BufferAttachmentUseInfo& useInfo)
{
    return {};
}

ImagePassAttachment Pass::CreateTransientImageResource(std::string name, const ImageCreateInfo& createInfo, const ImageAttachmentUseInfo& useInfo)
{
    return {};
}

BufferPassAttachment Pass::CreateTransientBufferResource(std::string name, const BufferCreateInfo& createInfo, const BufferAttachmentUseInfo& useInfo)
{
    return {};
}

ImagePassAttachment Pass::UseImageResource(const ImagePassAttachment& view, const ImageAttachmentUseInfo& useInfo)
{   
    return {};
}

BufferPassAttachment Pass::UseBufferResource(const BufferPassAttachment& view, const BufferAttachmentUseInfo& useInfo)
{
    return {};
}

}  // namespace RHI