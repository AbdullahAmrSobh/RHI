#include "RHI/FrameScheduler.hpp"

#include "RHI/Swapchain.hpp"

namespace RHI
{

void FrameScheduler::Begin()
{
    BeginInternal();
}

void FrameScheduler::End()
{
    EndInternal();

    for (Swapchain* swapchain : m_swapchainsToPresent)
    {
        swapchain->Present();
    }

    // reset the graph state.
    m_graphNodes.clear();
    m_passList.clear();
    m_swapchainsToPresent.clear();
    m_imageAttachments.Reset();
    m_bufferAttachments.Reset();

}

void FrameScheduler::Submit(Pass& pass)
{
    Node node {};
    node.pass = m_passList.back();

    m_graphNodes.push_back(node);
    m_passList.push_back(&pass);
}

void FrameScheduler::Compile()
{
    CompileInternal();
}

}  // namespace RHI