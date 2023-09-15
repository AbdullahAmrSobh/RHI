#include "RHI/FrameScheduler.hpp"

#include "RHI/Swapchain.hpp"

#include "RHI/Profiler.hpp"

namespace RHI
{

void FrameScheduler::Begin()
{
    RHI_PROFILE_SCOPE;

    BeginInternal();
}

void FrameScheduler::End()
{
    RHI_PROFILE_SCOPE;

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
    RHI_PROFILE_SCOPE;

    Node node {};
    node.pass = m_passList.back();

    m_graphNodes.push_back(node);
    m_passList.push_back(&pass);
}

void FrameScheduler::Compile()
{
    RHI_PROFILE_SCOPE;

    CompileInternal();
}

}  // namespace RHI