#include "RHI/Pch.hpp"

#include "RHI/Common.hpp"

#include "RHI/FrameScheduler.hpp"

#include "RHI/Attachment.hpp"

namespace RHI
{

void IFrameScheduler::Begin()
{
    m_producers.clear();
    m_attachmentsRegistry->Reset();
    m_frameGraphBuilder->Begin();
}

void IFrameScheduler::End()
{
    m_frameGraphBuilder->End();

    for (ImageAttachment* attachment : m_attachmentsRegistry->GetImageAttachments())
    {
        for (UsedImageAttachment& usedAttachment : *attachment)
        {
            Shared<IImageView> view = m_cachedResourcesAllocator->GetImageView(attachment->GetResource(), usedAttachment.GetViewDesc());
            usedAttachment.SetView(view);
        }
    }

    for (RenderPassProducer* producer : m_producers)
    {
        IRenderPass& renderpass = *producer->GetRenderPass();

        ICommandBuffer& commandBuffer = BeginCommandBuffer(renderpass);
        producer->BuildCommandBuffer(commandBuffer);
        EndCommandBuffer();

        Submit(renderpass);
    }
}

void IFrameScheduler::Schedule(RenderPassProducer& producer)
{
    if (!producer.m_renderpass)
    {
        producer.m_renderpass = CreateRenderPass(producer.m_name);
    }

    m_frameGraphBuilder->BeginPass(*producer.m_renderpass);
    producer.SetupAttachments(*m_frameGraphBuilder);
    m_frameGraphBuilder->EndPass();

    m_producers.push_back(&producer);
}

}  // namespace RHI