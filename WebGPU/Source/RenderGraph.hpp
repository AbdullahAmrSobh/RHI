#pragma once

#include <RHI/RenderGraph.hpp>

#include <webgpu/webgpu.h>

namespace RHI::WebGPU
{
    class IDevice;
    class ICompiledPass;
    class ICommandList;

    class IRenderGraph final : public RenderGraph
    {
    public:
        IRenderGraph();
        ~IRenderGraph();

        ResultCode Init(IDevice* device, const RenderGraphCreateInfo& createInfo);
        void       Shutdown();

        Pass*    CreatePass(const PassCreateInfo& createInfo) override;
        void     OnGraphExecutionBegin() override;
        void     OnGraphExecutionEnd() override;
        uint64_t ExecutePassGroup(const RenderGraphExecuteGroup& group, QueueType queueType) override;
    };
} // namespace RHI::WebGPU