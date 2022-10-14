#include "RHI/FrameGraph.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Swapchain.hpp"


namespace RHI
{
const RenderTargetLayout& IPass::GetRenderTargetLayout() const
{
    RenderTargetLayout output;
    for (auto& passAttachment : m_imagePassAttachments)
    {
        if (passAttachment->GetUsage() == EAttachmentUsage::RenderTarget)
        {
            output.colorFormats.push_back(passAttachment->GetDesc().format);
        }
    }

    if (m_pDepthStencilAttachment != nullptr)
    {
        output.depthStencilFormat = m_pDepthStencilAttachment->GetDesc().format;
    }
    
    return output;
}

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
}

EResultCode IFrameGraph::EndFrame()
{
    // for all swapchains in the graph
    // Enqueue present

    EndFrameInternal();
}

} // namespace RHI