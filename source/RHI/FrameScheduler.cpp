#include "RHI/FrameScheduler.hpp"

#include "RHI/FrameGraph.hpp"
#include "RHI/FrameGraphInterface.hpp"

namespace RHI
{

void FrameScheduler::FrameBegin()
{
    m_frameGraph->Begin();
}

void FrameScheduler::FrameEnd()
{
    m_frameGraph->End();
}

void FrameScheduler::Submit(Pass& pass)
{
    pass.SetupAttachments(*m_frameGraph);

    m_passes.push_back(&pass);
}

}  // namespace RHI