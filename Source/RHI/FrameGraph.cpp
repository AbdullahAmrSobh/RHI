#include "RHI/FrameGraph.hpp"
#include "RHI/Common.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Swapchain.hpp"


namespace RHI
{
EResultCode IFrameGraph::BeginFrame()
{
    m_currentFrameNumber++;

    // for all passes
    // if pass is modifed
    // recompile
    // Topological Reoderer

    // for all passes change the current commandbuffer

    std::vector<ISwapchain*> pSwapchains;
    for (ISwapchain* pSwapchain : pSwapchains)
    {
        // If a swapchain needs to recreate, then do it here.
        pSwapchain->SwapBuffers();
    }

    BeginFrameInternal();

    return EResultCode::Success;
}

EResultCode IFrameGraph::EndFrame()
{
    // for all swapchains in the graph
    // Enqueue present

    EndFrameInternal();

    return EResultCode::Success;
}

} // namespace RHI