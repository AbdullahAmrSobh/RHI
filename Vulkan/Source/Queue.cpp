#include "Queue.hpp"

#include <tracy/Tracy.hpp>

#include "CommandList.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"

namespace RHI::Vulkan
{
    IQueue::IQueue()  = default;
    IQueue::~IQueue() = default;

    ResultCode IQueue::Init(IDevice* device, uint32_t familyIndex, uint32_t queueIndex)
    {
        m_device = device;

        // Retrieve the queue handle.
        vkGetDeviceQueue(device->m_device, familyIndex, queueIndex, &m_queue);
        m_familyIndex = familyIndex;

        // Create the timeline semaphore.
        VkSemaphoreTypeCreateInfo timelineCreateInfo = {
            .sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext         = nullptr,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue  = 0,
        };

        VkSemaphoreCreateInfo semaphoreInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &timelineCreateInfo,
        };

        if (vkCreateSemaphore(device->m_device, &semaphoreInfo, nullptr, &m_timeline) != VK_SUCCESS)
        {
            return ResultCode::ErrorUnknown;
        }

        m_timelineValue.store(0);
        return ResultCode::Success;
    }

    void IQueue::Shutdown()
    {
        vkDestroySemaphore(m_device->m_device, m_timeline, nullptr);
    }

    void IQueue::BeginLabel(const char* name, const float color[4])
    {
        if (auto fn = m_device->m_pfn.m_vkQueueBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT label = {
                .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pNext      = nullptr,
                .pLabelName = name,
                .color      = {color[0], color[1], color[2], color[3]},
            };
            fn(m_queue, &label);
        }
    }

    void IQueue::EndLabel()
    {
        if (auto fn = m_device->m_pfn.m_vkQueueEndDebugUtilsLabelEXT)
        {
            fn(m_queue);
        }
    }

    uint64_t IQueue::GetTimelineSemaphoreValue() const
    {
        uint64_t value;
        vkGetSemaphoreCounterValue(m_device->m_device, m_timeline, &value);
        return value;
    }

    uint64_t IQueue::GetTimelineSemaphorePendingValue() const
    {
        return m_timelineValue;
    }

    uint64_t IQueue::Submit(const QueueSubmitInfo& submitInfo)
    {
        uint64_t nextValue = m_timelineValue.fetch_add(1) + 1;

        VkSemaphoreSubmitInfo waitSemaphoreInfos[AsyncQueuesCount + 1];
        uint32_t              waitCount = 0;

        if (auto semaphore = submitInfo.binaryWaitSemaphore.semaphore; semaphore != VK_NULL_HANDLE)
        {
            waitSemaphoreInfos[waitCount++] = {
                .sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .pNext       = nullptr,
                .semaphore   = semaphore,
                .value       = 0,
                .stageMask   = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                .deviceIndex = 0,
            };
        }
        for (uint32_t i = 0; i < AsyncQueuesCount; ++i)
        {
            if (submitInfo.timelineWaitSemaphores[i].semaphore)
                waitSemaphoreInfos[waitCount++] = submitInfo.timelineWaitSemaphores[i];
        }

        VkSemaphoreSubmitInfo signalSemaphoreInfos[2];
        uint32_t              signalCount = 1;
        signalSemaphoreInfos[0]           = {
                      .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                      .semaphore = m_timeline,
                      .value     = m_timelineValue,
                      .stageMask = submitInfo.timelineSignalStages,
        };
        if (auto semaphore = submitInfo.binarySignalSemaphore.semaphore; semaphore != VK_NULL_HANDLE)
        {
            signalSemaphoreInfos[waitCount++] = submitInfo.binarySignalSemaphore;
        }

        VkCommandBufferSubmitInfo commandBuffersSubmitInfos[32]  = {};
        uint32_t                  commandBuffersSubmitInfosCount = 0;
        for (auto commandList : submitInfo.commandLists)
        {
            commandBuffersSubmitInfos[commandBuffersSubmitInfosCount++] = {
                .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = commandList->GetHandle(),
            };
        }

        // Prepare the submission info.
        VkSubmitInfo2 submitInfo2 = {
            .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext                    = nullptr,
            .waitSemaphoreInfoCount   = waitCount,
            .pWaitSemaphoreInfos      = waitSemaphoreInfos,
            .commandBufferInfoCount   = commandBuffersSubmitInfosCount,
            .pCommandBufferInfos      = commandBuffersSubmitInfos,
            .signalSemaphoreInfoCount = signalCount,
            .pSignalSemaphoreInfos    = signalSemaphoreInfos,
        };

        auto result = vkQueueSubmit2(m_queue, 1, &submitInfo2, VK_NULL_HANDLE);
        Validate(result);
        return nextValue;
    }

} // namespace RHI::Vulkan