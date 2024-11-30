#include "Queue.hpp"

#include <tracy/Tracy.hpp>

#include "Barrier.hpp"
#include "CommandList.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"

namespace RHI::Vulkan
{

    IQueue::IQueue()  = default;
    IQueue::~IQueue() = default;

    ResultCode IQueue::Init(IDevice* device, uint32_t familyIndex, uint32_t queueIndex)
    {
        m_device      = device;
        m_queue       = VK_NULL_HANDLE;
        m_familyIndex = familyIndex;
        vkGetDeviceQueue(m_device->m_device, familyIndex, queueIndex, &m_queue);
        return ResultCode::Success;
    }

    void IQueue::Shutdown()
    {
        // Nothing to do here
    }

    void IQueue::BeginLabel(const char* name, float color[4])
    {
        if (auto fn = m_device->m_pfn.m_vkQueueBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT queueLabel{

                .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pNext      = nullptr,
                .pLabelName = name,
                .color      = {color[0], color[1], color[2], color[3]},
            };
            fn(m_queue, &queueLabel);
        }
    }

    void IQueue::EndLabel()
    {
        if (auto fn = m_device->m_pfn.m_vkQueueEndDebugUtilsLabelEXT)
        {
            fn(m_queue);
        }
    }

    uint64_t IQueue::Submit(const SubmitInfo& submitInfo)
    {
        ZoneScoped;

        auto swapchainToWait   = (ISwapchain*)submitInfo.swapchainToWait;
        auto swapchainToSignal = (ISwapchain*)submitInfo.swapchainToSignal;

        TL::Vector<VkCommandBufferSubmitInfo> commandBuffers;
        commandBuffers.reserve(submitInfo.commandLists.size());

        VkSemaphoreSubmitInfo waitSemaphoreInfo[2] = {
            {
                .sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .pNext       = nullptr,
                .semaphore   = m_device->GetTimelineSemaphore(),
                .value       = submitInfo.waitTimelineValue,
                .stageMask   = ConvertPipelineStageFlags(submitInfo.waitPipelineStage),
                .deviceIndex = 0,
            },
            {
                .sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .pNext       = nullptr,
                .semaphore   = swapchainToWait ? swapchainToWait->GetImageAcquiredSemaphore() : nullptr,
                .value       = 0,
                .stageMask   = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                .deviceIndex = 0,
            },
        };

        VkPipelineStageFlags2 commandBuffersSignalStages = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;

        for (CommandList* const _commandList : submitInfo.commandLists)
        {
            auto commandList = (ICommandList*)_commandList;
            commandBuffers.push_back({
                .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .pNext         = nullptr,
                .commandBuffer = commandList->GetHandle(),
                .deviceMask    = 0,
            });

            commandBuffersSignalStages |= commandList->GetPipelineStages();
        }

        VkSemaphoreSubmitInfo signalSemaphoreInfo[2] = {
            {
                .sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .pNext       = nullptr,
                .semaphore   = m_device->GetTimelineSemaphore(),
                .value       = m_device->GetPendingTimelineValue(),
                .stageMask   = commandBuffersSignalStages,
                .deviceIndex = 0,
            },
            {
                .sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .pNext       = nullptr,
                .semaphore   = swapchainToSignal ? swapchainToSignal->GetImagePresentSemaphore() : nullptr,
                .value       = 0,
                .stageMask   = commandBuffersSignalStages,
                .deviceIndex = 0,
            },
        };

        VkSubmitInfo2 submitInfo2{
            .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext                    = nullptr,
            .flags                    = 0,
            .waitSemaphoreInfoCount   = swapchainToWait ? 2u : 1u,
            .pWaitSemaphoreInfos      = waitSemaphoreInfo,
            .commandBufferInfoCount   = static_cast<uint32_t>(commandBuffers.size()),
            .pCommandBufferInfos      = commandBuffers.data(),
            .signalSemaphoreInfoCount = swapchainToSignal ? 2u : 1u,
            .pSignalSemaphoreInfos    = signalSemaphoreInfo,
        };

        auto result = vkQueueSubmit2(m_queue, 1, &submitInfo2, VK_NULL_HANDLE);
        TL_ASSERT(result == VK_SUCCESS);

        return m_device->GetTimelineValue();
    }
} // namespace RHI::Vulkan