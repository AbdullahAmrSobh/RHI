#include "RenderGraph.hpp"

#include "CommandList.hpp"
#include "Device.hpp"

namespace RHI::WebGPU
{
    IRenderGraph::IRenderGraph()  = default;
    IRenderGraph::~IRenderGraph() = default;

    ResultCode IRenderGraph::Init(IDevice* device, const RenderGraphCreateInfo& createInfo)
    {
        m_device = device;
        return ResultCode::Success;
    }

    void IRenderGraph::Shutdown()
    {
    }

    Pass* IRenderGraph::CreatePass(const PassCreateInfo& createInfo)
    {
        return m_tempAllocator.Construct<Pass>(createInfo, &m_tempAllocator);
    }

    void IRenderGraph::OnGraphExecutionBegin()
    {
    }

    void IRenderGraph::OnGraphExecutionEnd()
    {
    }

    uint64_t IRenderGraph::ExecutePassGroup(const RenderGraphExecuteGroup& group, QueueType queueType)
    {
        auto device = (IDevice*)m_device;

        auto commandList = (ICommandList*)device->CreateCommandList({.queueType = queueType});
        commandList->Begin();
        for (auto pass : group.GetPassList())
        {
            commandList->DebugMarkerPush(pass->GetName(), {});
            // if (pass->GetQueueType() == QueueType::Graphics)
            // {
                commandList->BeginRenderPass(*pass);
            // }
            ExecutePassCallback(*pass, *commandList);
            // if (pass->GetQueueType() == QueueType::Graphics)
            // {
                commandList->EndRenderPass();
            // }
            commandList->DebugMarkerPop();
        }
        commandList->End();

        device->ExecuteCommandList(commandList);

        return 0;
    }

} // namespace RHI::WebGPU