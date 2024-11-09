#pragma once
#include <cstdint>
#include <atomic>

#include <vulkan/vulkan.h>

#include <TL/Assert.hpp>

namespace RHI::Vulkan
{

    enum class TimelineStages : uint64_t
    {
        Submit  = 1,
        Draw    = 2,
        Present = 3,
        Max     = 4,
    };

    struct Timeline
    {
        VkSemaphore          m_semaphore;
        std::atomic_uint64_t m_frameIndex;

        uint32_t m_frameSubmitValue;

        uint32_t m_frameDrawValue;

        uint32_t m_framePresentValue;

        uint32_t m_frameMaxValue;

        VkResult Init(VkDevice device)
        {
            VkSemaphoreTypeCreateInfo typeCreateInfo{
                .sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
                .pNext         = nullptr,
                .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
                .initialValue  = m_frameIndex,
            };

            VkSemaphoreCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = &typeCreateInfo,
                .flags = 0,
            };

            VkResult result = vkCreateSemaphore(device, &createInfo, nullptr, &m_semaphore);
            return result;
        }

        void Shutdown(VkDevice device)
        {
            uint64_t timelineValue = m_frameIndex;

            VkSemaphoreWaitInfo waitInfo{
                .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
                .pNext          = nullptr,
                .flags          = 0,
                .semaphoreCount = 1,
                .pSemaphores    = &m_semaphore,
                .pValues        = &timelineValue,
            };

            vkWaitSemaphores(device, &waitInfo, UINT64_MAX);
            vkDestroySemaphore(device, m_semaphore, nullptr);
        }

        uint64_t GetTimelineStageValue(const TimelineStages stage) const
        {
            return (m_frameIndex * (uint64_t)TimelineStages::Max) + (uint64_t)stage;
        }

        // Signal the timeline from the host.
        void Signal(VkDevice device, const TimelineStages stage)
        {
            VkSemaphoreSignalInfo signalInfo{
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
                .pNext     = NULL,
                .semaphore = m_semaphore,
                .value     = GetTimelineStageValue(stage),
            };

            auto result = vkSignalSemaphoreKHR(device, &signalInfo);
            TL_ASSERT(result == VK_SUCCESS);
        }

        // Wait on the timeline from the host.
        void Wait(VkDevice device, const TimelineStages stage) const
        {
            const uint64_t waitValue = GetTimelineStageValue(stage);

            VkSemaphoreWaitInfo waitInfo{
                .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
                .pNext          = NULL,
                .flags          = 0,
                .semaphoreCount = 1,
                .pSemaphores    = &m_semaphore,
                .pValues        = &waitValue,
            };

            auto result = vkWaitSemaphoresKHR(device, &waitInfo, UINT64_MAX);
            TL_ASSERT(result == VK_SUCCESS);
        }

        // Sends the TimelineStages::Max signal for the current frame, then increments the frame counter
        void SignalNextFrame(VkDevice device)
        {
            VkSemaphoreSignalInfo signalInfo{
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
                .pNext     = NULL,
                .semaphore = m_semaphore,
                .value     = GetTimelineStageValue(TimelineStages::Max),
            };

            m_frameIndex++;

            auto result = vkSignalSemaphoreKHR(device, &signalInfo);
            TL_ASSERT(result == VK_SUCCESS);
        }

        // Waits for the timeline to reach TimelineStages::Max for the current frame
        void WaitNextFrame(VkDevice device) const
        {
            // TimelineStages::Max is used as it provides a boundary value between the stages of this frame and the next
            const uint64_t waitValue = (m_frameIndex + 1) * (uint64_t)TimelineStages::Max;

            VkSemaphoreWaitInfo waitInfo{
                .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
                .pNext          = NULL,
                .flags          = 0,
                .semaphoreCount = 1,
                .pSemaphores    = &m_semaphore,
                .pValues        = &waitValue,
            };

            auto result = vkWaitSemaphoresKHR(device, &waitInfo, UINT64_MAX);
            TL_ASSERT(result == VK_SUCCESS);
        }
    };

} // namespace RHI::Vulkan