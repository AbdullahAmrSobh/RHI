#include "RenderGraph.hpp"

#include <RHI/RenderGraphExecuteGroup.hpp>

#include <vulkan/vulkan.h>

#include <tracy/Tracy.hpp>

#include "CommandList.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"

namespace RHI::Vulkan
{
    IRenderGraph::IRenderGraph()  = default;
    IRenderGraph::~IRenderGraph() = default;

    ResultCode IRenderGraph::Init([[maybe_unused]] IDevice* device, const RenderGraphCreateInfo& createInfo)
    {
        this->m_device = device;
        return ResultCode::Success;
    }

    void IRenderGraph::Shutdown()
    {
    }

    void IRenderGraph::OnGraphExecutionBegin()
    {
        auto device = (IDevice*)m_device;

        auto& queue = device->GetDeviceQueue(QueueType::Graphics);

        m_currentFrameIndex             = m_currentFrameIndex % FramesInFlightCount;
        auto& currentFrameTimelineValue = m_framesInFlightTimelineValue[m_currentFrameIndex];

        // Wait for the graphics queue to catch up to the current frame
        auto currentTimelineValue = queue.GetTimelineValue();
        if (currentTimelineValue < currentFrameTimelineValue)
        {
            queue.WaitTimeline(currentFrameTimelineValue);
        }

        for (auto asyncQueue = QueueType::Graphics; asyncQueue < QueueType::Count; asyncQueue = (QueueType)((uint8_t)asyncQueue + 1))
        {
            m_queueTimelineFrameOffsets[(int)asyncQueue] = device->GetDeviceQueue(asyncQueue).GetTimelineValue() + 1;
            // TODO: remove once async queues are implemented
            break;
        }
    }

    void IRenderGraph::OnGraphExecutionEnd()
    {
        auto& currentFrameTimelineValue = m_framesInFlightTimelineValue[m_currentFrameIndex];
        currentFrameTimelineValue       = m_queueTimelineFrameOffsets[(int)QueueType::Graphics];
        auto device                     = (IDevice*)m_device;
        device->m_frameIndex++;
    }

    uint64_t IRenderGraph::ExecutePassGroup(const RenderGraphExecuteGroup& passGroup, QueueType queueType)
    {
        auto  device      = (IDevice*)m_device;
        auto& queue       = device->GetDeviceQueue(queueType);
        auto  commandList = (ICommandList*)m_device->CreateCommandList({.queueType = queueType});

        commandList->Begin();
        for (auto pass : passGroup.GetPassList())
        {
            pass->Execute(*commandList);
        }
        commandList->End();

        if (auto swapchainToWait = passGroup.GetSwapchainToWait(); swapchainToWait.swapchain != nullptr)
        {
            auto swapchain = (ISwapchain*)swapchainToWait.swapchain;
            queue.AddWaitSemaphore(swapchain->GetImageAcquiredSemaphore(), 0, ConvertPipelineStageFlags(swapchainToWait.stage));
        }

        if (auto swapchainToSignal = passGroup.GetSwapchainToSignal(); swapchainToSignal.swapchain != nullptr)
        {
            auto swapchain = (ISwapchain*)swapchainToSignal.swapchain;
            queue.AddSignalSemaphore(swapchain->GetImagePresentSemaphore(), 0, ConvertPipelineStageFlags(swapchainToSignal.stage));
        }

        // TODO: deduce the pipeline stage from the command lists ...
        return m_queueTimelineFrameOffsets[(int)queueType] = queue.Submit({commandList}, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);
    }
} // namespace RHI::Vulkan