#include "RHI/Pass.hpp"

#include "RHI/FrameScheduler.hpp"

namespace RHI
{
    
void Pass::Begin()
{
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

Handle<ImageView> Pass::ImportImageResource(Handle<Image> image, const ImageAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access)
{
    return {};
}

Handle<BufferView> Pass::ImportBufferResource(Handle<Buffer> buffer, const BufferAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access)
{
    return {};
}

Handle<ImageView> Pass::CreateTransientImageResource(const TransientImageCreateInfo& createInfo, AttachmentUsage usage, AttachmentAccess access)
{
    return {};
}

Handle<BufferView> Pass::CreateTransientBufferResource(const TransientBufferCreateInfo& createInfo, AttachmentUsage usage, AttachmentAccess access)
{
    return {};
}

Handle<ImageView> Pass::UseImageResource(Handle<ImageView> view, const ImageAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access)
{
    return {};
}

Handle<BufferView> Pass::UseBufferResource(Handle<BufferView> view, const BufferAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access)
{
    return {};
}

}  // namespace RHI