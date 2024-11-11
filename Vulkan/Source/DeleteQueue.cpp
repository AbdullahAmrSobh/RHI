#include "DeleteQueue.hpp"
#include "Device.hpp"

#include <TL/Assert.hpp>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{

    DeleteQueue::DeleteQueue()  = default;
    DeleteQueue::~DeleteQueue() = default;

    ResultCode DeleteQueue::Init(IDevice* device)
    {
        m_device = device;
        return ResultCode::Success;
    }

    void DeleteQueue::Shutdown()
    {
        vkDeviceWaitIdle(m_device->m_device);
        DestroyQueued(true);
        TL_ASSERT(m_destructionQueue.empty());
    }

    void DeleteQueue::DestroyObject(DeleteFunc deleteFunc, uint64_t frameIndex)
    {
        m_destructionQueue.push_back({frameIndex, std::move(deleteFunc)});
    }

    void DeleteQueue::DestroyQueued(bool force)
    {
        uint64_t currentTimelineValue = GetTimelineGpuValue();

        // Iterate over the queue and execute deletions where frameIndex condition is met
        auto it = m_destructionQueue.begin();
        while (it != m_destructionQueue.end())
        {
            if (force || it->frameIndex <= currentTimelineValue)
            {
                it->deleteFunc(m_device);          // Invoke the deletion lambda
                it = m_destructionQueue.erase(it); // Remove after execution
            }
            else
            {
                ++it; // Move to the next item if the condition is not met
            }
        }
    }

    uint64_t DeleteQueue::GetTimelineGpuValue() const
    {
        uint64_t timelineValue = 0;
        vkGetSemaphoreCounterValue(m_device->m_device, m_device->GetTimelineSemaphore(), &timelineValue);
        return timelineValue;
    }
} // namespace RHI::Vulkan