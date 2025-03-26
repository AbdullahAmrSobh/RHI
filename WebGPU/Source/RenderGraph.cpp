#include "RenderGraph.hpp"

namespace RHI::WebGPU
{
    IRenderGraph::IRenderGraph()  = default;
    IRenderGraph::~IRenderGraph() = default;

    ResultCode IRenderGraph::Init(IDevice* device, const RenderGraphCreateInfo& createInfo)
    {
        return ResultCode::Success;
    }

    void IRenderGraph::Shutdown()
    {
    }

    Pass* IRenderGraph::CreatePass(const PassCreateInfo& createInfo)
    {
        return nullptr;
    }

    void IRenderGraph::OnGraphExecutionBegin()
    {
    }

    void IRenderGraph::OnGraphExecutionEnd()
    {
    }

    uint64_t IRenderGraph::ExecutePassGroup(const RenderGraphExecuteGroup& group, QueueType queueType)
    {
        return 0;
    }

} // namespace RHI::WebGPU