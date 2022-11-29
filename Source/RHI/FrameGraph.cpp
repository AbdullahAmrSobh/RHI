#include "RHI/FrameGraph.hpp"
#include "RHI/FrameGraphPass.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{

EResultCode IFrameGraph::BeginFrame()
{
    // Swapbuffers for all imported attachments
    for (ISwapchain* swapchain : m_attachmentsRegistry->m_swapchains)
    {
        swapchain->SwapBuffers();
    }

    return EResultCode::Success;
}

EResultCode IFrameGraph::EndFrame()
{
    return EResultCode::Success;
}

EResultCode IFrameGraph::ImportPassProducer(IPassProducer& producer, std::string name, EPassType type)
{
    m_producers.push_back(&producer);

    auto expectedPass = this->CreatePass(std::move(name), type);
    
    if(!expectedPass.has_value())
    {
        return expectedPass.error();
    }
    
    producer.m_pass = std::move(*expectedPass);

    return EResultCode::Success;
}

} // namespace RHI