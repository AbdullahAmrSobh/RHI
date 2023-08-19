#include "RHI/FrameScheduler.hpp"

namespace RHI
{

void FrameScheduler::Begin()
{
}

void FrameScheduler::End()
{
}

void FrameScheduler::AddPass(PassProducer& passProducer)
{
}

void FrameScheduler::BeignPass(Pass* pass)
{
}

void FrameScheduler::EndPass()
{
}

void FrameScheduler::AddPassDependency(Pass* consumer, Pass* producer)
{
}

Fence& FrameScheduler::AddSignalFence()
{
    return *((Fence*)(nullptr));
}

void FrameScheduler::ImportImage(std::string name, const ImageAttachmentImportInfo& importInfo, AttachmentUsage usage, AttachmentAccess access)
{
}

void FrameScheduler::ImportBuffer(std::string name, const BufferAttachmentImportInfo& importInfo, AttachmentUsage usage, AttachmentAccess access)
{
}

void FrameScheduler::CreateTransientImageAttachment(std::string name, const ImageCreateInfo& createInfo, AttachmentUsage usage, AttachmentAccess access)
{
}

void FrameScheduler::CreateTransientBufferAttachment(std::string name, const BufferCreateInfo& createInfo, AttachmentUsage usage, AttachmentAccess access)
{
}

void FrameScheduler::UseImageAttachment(std::string name, const ImageAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access)
{
}

void FrameScheduler::UseBufferAttachment(std::string name, const BufferAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access)
{
}

}  // namespace RHI