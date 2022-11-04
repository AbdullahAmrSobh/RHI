#include "RHI/FrameGraph.hpp"
#include "RHI/Swapchain.hpp"
#include "RHI/Commands.hpp"

namespace RHI
{

const std::vector<const ImagePassAttachment*> IPass::GetImageAttachments() const
{
    std::vector<const ImagePassAttachment*> attachments;
    attachments.reserve(attachments.size());
    for (auto& attachment : m_imageAttachments)
    {
        attachments.push_back(attachment.get());
    }
    return attachments;
}

const std::vector<const BufferPassAttachment*> IPass::GetBufferAttachments() const
{
    std::vector<const BufferPassAttachment*> attachments;
    attachments.reserve(attachments.size());
    for (auto& attachment : m_bufferAttachments)
    {
        attachments.push_back(attachment.get());
    }
    return attachments;
}

const std::vector<const ImagePassAttachment*> IPass::GetSwapchainAttachemnts() const
{
    std::vector<const ImagePassAttachment*> attachments;
    attachments.reserve(attachments.size());
    for (auto& attachment : m_swapchainImages)
    {
        attachments.push_back(attachment.get());
    }
    return attachments;
}

EResultCode IFrameGraph::BeginFrame()
{
    for (auto& swapchain : m_swapchains)
    {
        EResultCode result = swapchain->SwapBuffers();
        if (result != EResultCode::Success)
        {
            return result;
        }
    }

    return this->BeginFrameInternal();
}

EResultCode IFrameGraph::EndFrame()
{
    return this->EndFrameInternal();
}

EResultCode IFrameGraph::Execute(IPassProducer& producer)
{
    if (!producer.m_pass->IsValid())
    {
        FrameGraphBuilder builder{this};
        BeginPass(*producer.m_pass);
        producer.Setup(builder);
        EndPass();
    }
        
    if (!producer.m_pass->IsCompiled())
    {
        EResultCode result = CompilePass(*producer.m_pass);
        if (result != EResultCode::Success)
        {
            return result;
        }
    }
    

    FrameGraphContext context;
    producer.Compile(context);
    
    m_currentIndex = m_currentIndex % m_maxIndex;
    producer.BuildCommandBuffer(*producer.m_pass->m_commandBuffers.at(m_currentIndex));
    
    return this->Submit(*producer.m_pass);
}

} // namespace RHI