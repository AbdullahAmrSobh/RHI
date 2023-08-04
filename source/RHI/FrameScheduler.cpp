#include "RHI/FrameScheduler.hpp"

#include "RHI/Debug.hpp"
#include "RHI/FrameGraphAttachments.hpp"
#include "RHI/Pass.hpp"
#include "RHI/PassInterface.hpp"
#include "RHI/Context.hpp"

namespace RHI
{

void FrameScheduler::FrameBegin()
{
    m_frameGraph->Begin();
}

void FrameScheduler::FrameEnd()
{
    m_frameGraph->End();

    // Build the graph
    for (PassInterface* passInterface : m_passesList)
    {
        m_frameGraph->m_currentPass = passInterface->m_pass.get();

        passInterface->SetupAttachments(*m_frameGraph);
    }

    // Compile all transient resources
    {
        TransientAllocatorBegin();

        for (RHI::Attachment* attachment : m_frameGraph->GetRegistry().GetTransientAttachments())
        {
            switch (attachment->type)
            {
                case RHI::AttachmentType::Image: attachment->image = AllocateTransientImage(attachment->imageInfo); break;
                case RHI::AttachmentType::Buffer: attachment->buffer = AllocateTransientBuffer(attachment->bufferInfo); break;
                default: RHI_UNREACHABLE(); break;
            }
        }

        TransientAllocatorEnd();
    }

    // Create all views for all attachments
    for (RHI::PassInterface* passProducer : m_passesList)
    {
        auto& pass = *passProducer->m_pass;

        for (RHI::PassAttachment* passAttachment : pass.GetImageAttachments())
        {
            auto image                = passAttachment->attachment->image;
            auto viewDesc             = passAttachment->imageInfo.viewInfo;
            passAttachment->imageView = CreateView(viewDesc);
        }

        for (RHI::PassAttachment* passAttachment : pass.GetBufferAttachments())
        {
            auto buffer                = passAttachment->attachment->buffer;
            auto viewDesc              = passAttachment->bufferInfo.viewInfo;
            passAttachment->bufferView = CreateView(viewDesc);
        }
    }

    // Build command lists
    for (PassInterface* passInterface : m_passesList)
    {
        auto&        pass        = *passInterface->m_pass;
        CommandList& commandList = PassExecuteBegin(pass);
        passInterface->BuildCommandList(commandList);
        PassExecuteEnd(pass);
    }
}

void FrameScheduler::Submit(PassInterface& pass)
{
    m_passesList.push_back(&pass);
}

}  // namespace RHI