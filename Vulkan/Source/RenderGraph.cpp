#include "RenderGraph.hpp"

#include <RHI/RenderGraphExecuteGroup.hpp>

#include <vulkan/vulkan.h>

#include <tracy/Tracy.hpp>

#include "CommandList.hpp"
#include "Device.hpp"
#include "RenderGraphPass.hpp"
#include "Swapchain.hpp"

namespace RHI::Vulkan
{

    inline static void RecordPassBarriers(IDevice& device, CompiledPass& pass)
    {
        for (const auto& resourceTransition : pass.GetRenderGraphResourceTransitions())
        {
            auto [srcStageMask, srcAccessMask, srcLayout, srcQfi] = GetBarrierStage(resourceTransition->prev);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQfi] = GetBarrierStage(resourceTransition);
            if (resourceTransition->resource->GetType() == RenderGraphResource::Type::Image)
            {
                auto imageTransition = (RenderGraphImage*)resourceTransition->resource;
                auto image           = device.m_imageOwner.Get(imageTransition->GetImage());
                pass.PushPassBarrier(
                    BarrierSlot::Prilogue,
                    {
                        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                        .pNext               = nullptr,
                        .srcStageMask        = srcStageMask,
                        .srcAccessMask       = srcAccessMask,
                        .dstStageMask        = dstStageMask,
                        .dstAccessMask       = dstAccessMask,
                        .oldLayout           = srcLayout,
                        .newLayout           = dstLayout,
                        .srcQueueFamilyIndex = srcQfi == dstQfi ? VK_QUEUE_FAMILY_IGNORED : srcQfi,
                        .dstQueueFamilyIndex = srcQfi == dstQfi ? VK_QUEUE_FAMILY_IGNORED : dstQfi,
                        .image               = image->handle,
                        .subresourceRange    = GetAccessedSubresourceRange(*resourceTransition),
                    });
            }
            else
            {
                auto bufferTransition = (RenderGraphBuffer*)resourceTransition->resource;
                auto buffer           = device.m_bufferOwner.Get(bufferTransition->GetBuffer());
                pass.PushPassBarrier(
                    BarrierSlot::Prilogue,
                    {
                        .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                        .pNext               = nullptr,
                        .srcStageMask        = srcStageMask,
                        .srcAccessMask       = srcAccessMask,
                        .dstStageMask        = dstStageMask,
                        .dstAccessMask       = dstAccessMask,
                        .srcQueueFamilyIndex = srcQfi == dstQfi ? VK_QUEUE_FAMILY_IGNORED : srcQfi,
                        .dstQueueFamilyIndex = srcQfi == dstQfi ? VK_QUEUE_FAMILY_IGNORED : dstQfi,
                        .buffer              = buffer->handle,
                        .offset              = resourceTransition->asBuffer.subregion.offset,
                        .size                = resourceTransition->asBuffer.subregion.size,
                    });
            }
        }
    }

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

    Pass* IRenderGraph::CreatePass(const PassCreateInfo& createInfo)
    {
        return m_tempAllocator.Construct<CompiledPass>(createInfo, &m_tempAllocator);
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
        for (auto _pass : passGroup.GetPassList())
        {
            auto pass = (CompiledPass*)_pass;
            RecordPassBarriers(*device, *pass);

            commandList->DebugMarkerPush(pass->GetName(), QueueTypeToColor(queueType));
            pass->EmitBarriers(*commandList, BarrierSlot::Prilogue);
            commandList->BeginRenderPass(*pass);
            ExecutePassCallback(*pass, *commandList);
            commandList->EndRenderPass();
            pass->EmitBarriers(*commandList, BarrierSlot::Epilogue);
            commandList->DebugMarkerPop();
        }
        commandList->End();

        QueueSubmitInfo queueSubmitInfo(*device);
        for (auto queueWaitType = QueueType::Graphics; queueWaitType < QueueType::Count; queueWaitType = (QueueType)((uint8_t)queueWaitType + 1))
        {
            auto waitInfo = passGroup.GetQueueWaitInfo(queueWaitType);
            if (waitInfo.timelineValue != 0 && queueWaitType != queueType)
            {
                auto& queueToWait = device->GetDeviceQueue(queueWaitType);
                queueSubmitInfo.AddWaitSemaphore(
                    queueToWait.GetTimelineHandle(),
                    m_queueTimelineFrameOffsets[(int)queueWaitType] + waitInfo.timelineValue,
                    ConvertPipelineStageFlags(waitInfo.waitStage));
            }
            break;
        }

        if (auto swapchainToWait = passGroup.GetSwapchainToWait(); swapchainToWait.swapchain != nullptr)
        {
            auto swapchain = (ISwapchain*)swapchainToWait.swapchain;
            queueSubmitInfo.AddWaitSemaphore(
                swapchain->GetImageAcquiredSemaphore(),
                0,
                ConvertPipelineStageFlags(swapchainToWait.stage));
        }

        if (auto swapchainToSignal = passGroup.GetSwapchainToSignal(); swapchainToSignal.swapchain != nullptr)
        {
            auto swapchain = (ISwapchain*)swapchainToSignal.swapchain;
            queueSubmitInfo.AddSignalSemaphore(
                swapchain->GetImagePresentSemaphore(),
                0,
                ConvertPipelineStageFlags(swapchainToSignal.stage));
        }

        queueSubmitInfo.AddCommandList(commandList->GetHandle());
        queueSubmitInfo.signalStage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;

        return m_queueTimelineFrameOffsets[(int)queueType] = queue.Submit(queueSubmitInfo);
    }
} // namespace RHI::Vulkan