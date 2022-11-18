#include "RHI/FrameGraph.hpp"
#include "RHI/Common.hpp"

namespace RHI
{

EResultCode IFrameGraph::BeginFrame()
{
    for (auto& swapchain : m_swapchains)
    {
        swapchain->SwapBuffers();
    }

    return this->BeginFrame();
}

EResultCode IFrameGraph::EndFrame()
{
    this->EndFrame();
}

EResultCode IFrameGraph::SubmitPass(IPassProducer& producer)
{
    return this->SubmitPass(producer);
}

EResultCode IFrameGraph::RegisterPass(IPassProducer& producer)
{
    auto result = CreatePass(producer.m_name, producer.m_passType);

    if (result.has_value())
    {
        producer.m_pass = std::move(result.value());
    }
    else
    {
        return result.error();
    }

    return EResultCode::Success;
}

} // namespace RHI