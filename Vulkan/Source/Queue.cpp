
#include "Queue.hpp"

#include <tracy/Tracy.hpp>

#include "CommandList.hpp"
#include "Common.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"

namespace RHI::Vulkan
{
    QueueSubmitInfo::QueueSubmitInfo(IDevice& device)
        : waitSemaphores(device.m_tempAllocator)
        , commandLists(device.m_tempAllocator)
        , signalSemaphores(device.m_tempAllocator)
    {
    }

    IQueue::IQueue()  = default;
    IQueue::~IQueue() = default;

    ResultCode IQueue::Init(IDevice* device, const char* debugName, uint32_t familyIndex, uint32_t queueIndex)
    {
        m_device = device;

        vkGetDeviceQueue(device->m_device, familyIndex, queueIndex, &m_queue);
        m_familyIndex = familyIndex;

        m_device->SetDebugName(m_queue, debugName);

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

        m_device->SetDebugName(m_timeline, std::format("{}-timeline-semaphore", debugName).c_str());
        m_timelineValue.store(0);

        return ResultCode::Success;
    }

    void IQueue::Shutdown()
    {
        if (m_timeline != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device->m_device, m_timeline, nullptr);
            m_timeline = VK_NULL_HANDLE;
        }
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

    uint64_t IQueue::GetTimelineValue() const
    {
        uint64_t value = 0;
        vkGetSemaphoreCounterValue(m_device->m_device, m_timeline, &value);
        return value;
    }

    uint64_t IQueue::GetTimelinePendingValue() const
    {
        return m_timelineValue.load();
    }

    uint64_t IQueue::Submit(QueueSubmitInfo& submitInfo)
    {
        submitInfo.AddSignalSemaphore(m_timeline, m_timelineValue + 1, submitInfo.signalStage);

        VkSubmitInfo2 submitInfo2 = {
            .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext                    = nullptr,
            .waitSemaphoreInfoCount   = static_cast<uint32_t>(submitInfo.waitSemaphores.size()),
            .pWaitSemaphoreInfos      = submitInfo.waitSemaphores.data(),
            .commandBufferInfoCount   = static_cast<uint32_t>(submitInfo.commandLists.size()),
            .pCommandBufferInfos      = submitInfo.commandLists.data(),
            .signalSemaphoreInfoCount = static_cast<uint32_t>(submitInfo.signalSemaphores.size()),
            .pSignalSemaphoreInfos    = submitInfo.signalSemaphores.data(),
        };
        auto result = vkQueueSubmit2(m_queue, 1, &submitInfo2, VK_NULL_HANDLE);
        Validate(result);
        return m_timelineValue.fetch_add(1);
    }

    bool IQueue::WaitTimeline(uint64_t timelineValue, uint64_t duration)
    {
        VkSemaphoreWaitInfo waitInfo = {
            .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .pNext          = nullptr,
            .flags          = 0,
            .semaphoreCount = 1,
            .pSemaphores    = &m_timeline,
            .pValues        = &timelineValue,
        };
        return vkWaitSemaphores(m_device->m_device, &waitInfo, duration) == VK_SUCCESS;
    }

    uint64_t IQueue::SignalTimeline(uint64_t timelineValue)
    {
        VkSemaphoreSignalInfo signalInfo = {
            .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
            .pNext     = nullptr,
            .semaphore = m_timeline,
            .value     = timelineValue,
        };
        auto result = vkSignalSemaphore(m_device->m_device, &signalInfo);
        Validate(result);
        return timelineValue;
    }
} // namespace RHI::Vulkan
