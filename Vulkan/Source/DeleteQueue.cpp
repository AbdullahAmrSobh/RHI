#include "DeleteQueue.hpp"
#include "Context.hpp"

#include <TL/Assert.hpp>

namespace RHI::Vulkan
{
    DeleteQueue::DeleteQueue() = default;

    DeleteQueue::~DeleteQueue()
    {
        // TL_ASSERT(m_callbacks.empty());
    }

    void DeleteQueue::DeferCommand(std::function<void()> callback)
    {
        m_callbacks.push_back(callback);
    }

    void DeleteQueue::Flush()
    {
        for (auto fn : m_callbacks)
        {
            fn();
        }
        m_callbacks.clear();
    }

    void FrameExecuteContext::AdvanceFrame()
    {
        m_frame[m_frameIndex].m_deleteQueue.Flush();
        m_frameIndex = GetNextFrameIndex();
    }

    uint32_t FrameExecuteContext::GetFrameIndex() const
    {
        return m_frameIndex;
    }

    uint32_t FrameExecuteContext::GetNextFrameIndex() const
    {
        return (m_frameIndex + 1) % 2;
    }

    void FrameExecuteContext::DeferCommand(std::function<void()> callback)
    {
        auto& frame = CurrentFrame();
        frame.m_deleteQueue.DeferCommand(callback);
    }

    void FrameExecuteContext::DeferNextFrame(std::function<void()> callback)
    {
        auto& frame = m_frame[GetNextFrameIndex()];
        frame.m_deleteQueue.DeferCommand(callback);
    }

} // namespace RHI::Vulkan