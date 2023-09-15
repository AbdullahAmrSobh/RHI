#include "RHI/Pass.hpp"

#include "RHI/FrameScheduler.hpp"

#include <RHI/Profiler.hpp>

namespace RHI
{

void Pass::Begin()
{
    RHI_PROFILE_SCOPE;
    
    /// Unuse all attachments
    m_waitPasses.clear();
    m_usedImages.clear();
    m_usedBuffers.clear();
}

void Pass::End()
{
    RHI_PROFILE_SCOPE;

}

void Pass::ExecuteAfter(Pass& pass)
{
    RHI_PROFILE_SCOPE;

    m_waitPasses.push_back(&pass);
}

void Pass::ExecuteBefore(Pass& pass)
{
    RHI_PROFILE_SCOPE;

    pass.m_waitPasses.push_back(this);
}

ImagePassAttachment Pass::ImportImageResource(std::string name, Handle<Image> image, const ImageAttachmentUseInfo& useInfo)
{
    RHI_PROFILE_SCOPE;

    return {};
}

BufferPassAttachment Pass::ImportBufferResource(std::string name, Handle<Buffer> buffer, const BufferAttachmentUseInfo& useInfo)
{
    RHI_PROFILE_SCOPE;

    return {};
}

ImagePassAttachment Pass::CreateTransientImageResource(std::string name, const ImageCreateInfo& createInfo, const ImageAttachmentUseInfo& useInfo)
{
    RHI_PROFILE_SCOPE;

    return {};
}

BufferPassAttachment Pass::CreateTransientBufferResource(std::string name, const BufferCreateInfo& createInfo, const BufferAttachmentUseInfo& useInfo)
{
    RHI_PROFILE_SCOPE;

    return {};
}

ImagePassAttachment Pass::UseImageResource(const ImagePassAttachment& view, const ImageAttachmentUseInfo& useInfo)
{   
    RHI_PROFILE_SCOPE;

    return {};
}

BufferPassAttachment Pass::UseBufferResource(const BufferPassAttachment& view, const BufferAttachmentUseInfo& useInfo)
{
    RHI_PROFILE_SCOPE;

    return {};
}

}  // namespace RHI