#pragma once

#include <RHI/RenderGraph.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;
    class ICompiledPass;
    class ICommandList;

    class IRenderGraph final : public RenderGraph
    {
    public:
        IRenderGraph();
        ~IRenderGraph();

        ResultCode Init(IDevice* device);
        void       Shutdown();

        Pass*    CreatePass(const PassCreateInfo& createInfo) override;
        void     OnGraphExecutionBegin() override;
        void     OnGraphExecutionEnd() override;
        uint64_t ExecutePassGroup(const RenderGraphExecuteGroup& group, QueueType queueType) override;

    private:
        uint64_t m_queueTimelineFrameOffsets[AsyncQueuesCount];

        // Frames in flight management
        static constexpr uint32_t FramesInFlightCount = 3;
        uint64_t                  m_framesInFlightTimelineValue[FramesInFlightCount];
        uint32_t                  m_currentFrameIndex;
    };

} // namespace RHI::Vulkan