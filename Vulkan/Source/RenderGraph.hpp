#pragma once

#include <RHI/RenderGraph.hpp>

namespace RHI::Vulkan
{
    class IDevice;

    class IRenderGraph final : public RenderGraph
    {
    public:
        IRenderGraph();
        ~IRenderGraph();

        ResultCode Init(IDevice* device);
        void       Shutdown();

        void OnBeginPassExecute(Pass& pass, CommandList& commandList) override;
        void OnEndPassExecute(Pass& pass, CommandList& commandList) override;
    };

} // namespace RHI::Vulkan