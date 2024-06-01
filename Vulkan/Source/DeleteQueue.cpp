#include "DeleteQueue.hpp"

namespace RHI::Vulkan
{
    DeleteQueue::~DeleteQueue()
    {
        for (auto queue : m_deleteQueue)
        {
            RHI_ASSERT(queue.empty());
        }
    }

    void DeleteQueue::Destroy(uint32_t frameIndex, std::function<void()> callback)
    {
        auto frameId = frameIndex % MAX_FRAMES_IN_FLIGHT_COUNT;
        m_deleteQueue[frameId].push_back(callback);
    }

    void DeleteQueue::Flush(uint32_t frameIndex)
    {
        auto frameId = frameIndex % MAX_FRAMES_IN_FLIGHT_COUNT;
        for (auto callback : m_deleteQueue[frameId])
        {
            callback();
        }
        m_deleteQueue[frameId].clear();
    }
} // namespace RHI::Vulkan