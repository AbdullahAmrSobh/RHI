#include "DeleteQueue.hpp"

#include <TL/Assert.hpp>

#include <tracy/Tracy.hpp>

#include "Device.hpp"

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
        m_device->WaitIdle();

        for (auto& deletion : m_destructionQueue)
        {
            if (deletion.deleteFunc)
                deletion.deleteFunc(m_device);
        }

        TL_ASSERT(m_destructionQueue.empty());
    }

    void DeleteQueue::Push(uint64_t frameIndex, DeleteFunc&& deleteFunc)
    {
        // std::lock_guard<std::mutex> lock(m_mutex);

        if (m_writeIndex < m_destructionQueue.size())
        {
            // Overwrite existing slot
            m_destructionQueue[m_writeIndex] = {frameIndex, std::move(deleteFunc)};
            m_writeIndex++;
        }
        else
        {
            // Grow the buffer
            m_destructionQueue.push_back({frameIndex, std::move(deleteFunc)});
            m_writeIndex++;
        }

        // Wrap around if needed
        if (m_writeIndex >= m_destructionQueue.capacity())
        {
            m_writeIndex = 0;
        }
    }

    void DeleteQueue::DestroyObjects()
    {
        m_completedFrameIndex = m_device->GetFrameIndex();
        // std::lock_guard<std::mutex> lock(m_mutex);

        uint32_t originalReadIndex = m_readIndex; // Store original read index for clearing check

        while (m_readIndex != m_writeIndex)
        {
            if (m_destructionQueue[m_readIndex].frameIndex <= m_completedFrameIndex)
            {
                m_destructionQueue[m_readIndex].deleteFunc(m_device);
                m_destructionQueue[m_readIndex].deleteFunc = nullptr;
                m_readIndex++;

                if (m_readIndex >= m_destructionQueue.capacity())
                {
                    m_readIndex = 0; // Wrap around
                }
            }
            else
            {
                break; // Stop if we've reached a frame that isn't complete yet
            }
        }

        // If we have deleted everything, just clear the vector.
        if (m_readIndex == originalReadIndex)
        {
            m_destructionQueue.clear();
            m_readIndex  = 0;
            m_writeIndex = 0;
        }
    }
} // namespace RHI::Vulkan