#include "DeleteQueue.hpp"
#include "Context.hpp"

namespace RHI::Vulkan
{
    DeleteQueue::DeleteQueue() = default;

    DeleteQueue::~DeleteQueue()
    {
        RHI_ASSERT(m_callbacks.empty());
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
        m_frameIndex = GetNextFrameIndex();
    }

    void FrameExecuteContext::SetFrameInFlightsCount(uint32_t count)
    {
        m_frameInFlightsCount = count;
        RHI_UNREACHABLE(); // should not use it
    }

    uint32_t FrameExecuteContext::GetFrameIndex() const
    {
        return m_frameIndex;
    }

    uint32_t FrameExecuteContext::GetNextFrameIndex() const
    {
        return (m_frameIndex + 1) % m_frameInFlightsCount;
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

    SemaphoreStage FrameExecuteContext::GetImageWaitSemaphore(Handle<Image> image)
    {
        auto& frame = CurrentFrame();
        if (auto it = frame.m_imageWaitSemaphores.find(image); it != frame.m_imageWaitSemaphores.end())
        {
            auto semaphore = it->second.semaphore;
            DeferNextFrame([this, semaphore]()
            {
                m_context->DestroySemaphore(semaphore);
            });
            return it->second;
        }
        return { VK_NULL_HANDLE, 0 };
    }

    SemaphoreStage FrameExecuteContext::GetImageSignalSemaphore(Handle<Image> image)
    {
        auto& frame = CurrentFrame();
        if (auto it = frame.m_imageSignalSemaphores.find(image); it != frame.m_imageSignalSemaphores.end())
        {
            auto semaphore = it->second.semaphore;
            DeferNextFrame([this, semaphore]()
            {
                m_context->DestroySemaphore(semaphore);
            });
            return it->second;
        }
        return { VK_NULL_HANDLE, 0 };
    }

    SemaphoreStage FrameExecuteContext::GetBufferWaitSemaphore(Handle<Buffer> buffer)
    {
        auto& frame = CurrentFrame();
        if (auto it = frame.m_bufferWaitSemaphores.find(buffer); it != frame.m_bufferWaitSemaphores.end())
        {
            auto semaphore = it->second.semaphore;
            DeferNextFrame([this, semaphore]()
            {
                m_context->DestroySemaphore(semaphore);
            });
            return it->second;
        }
        return { VK_NULL_HANDLE, 0 };
    }

    SemaphoreStage FrameExecuteContext::GetBufferSignalSemaphore(Handle<Buffer> buffer)
    {
        auto& frame = CurrentFrame();
        if (auto it = frame.m_bufferSignalSemaphores.find(buffer); it != frame.m_bufferSignalSemaphores.end())
        {
            auto semaphore = it->second.semaphore;
            DeferNextFrame([this, semaphore]()
            {
                m_context->DestroySemaphore(semaphore);
            });
            return it->second;
        }
        return { VK_NULL_HANDLE, 0 };
    }

    VkSemaphore FrameExecuteContext::AddImageWaitSemaphore(Handle<Image> image, VkPipelineStageFlags2 stages)
    {
        auto& frame = CurrentFrame();
        RHI_ASSERT(frame.m_imageSignalSemaphores.find(image) == frame.m_imageSignalSemaphores.end());
        auto semaphore = m_context->CreateSemaphore();
        frame.m_imageWaitSemaphores[image] = { semaphore, stages };
        return semaphore;
    }

    VkSemaphore FrameExecuteContext::AddImageSignalSemaphore(Handle<Image> image, VkPipelineStageFlags2 stages)
    {
        auto& frame = CurrentFrame();
        RHI_ASSERT(frame.m_imageWaitSemaphores.find(image) == frame.m_imageWaitSemaphores.end());
        auto semaphore = m_context->CreateSemaphore();
        frame.m_imageSignalSemaphores[image] = { semaphore, stages };
        return semaphore;
    }

    VkSemaphore FrameExecuteContext::AddBufferWaitSemaphore(Handle<Buffer> buffer, VkPipelineStageFlags2 stages)
    {
        auto& frame = CurrentFrame();
        RHI_ASSERT(frame.m_bufferSignalSemaphores.find(buffer) == frame.m_bufferWaitSemaphores.end());
        auto semaphore = m_context->CreateSemaphore();
        frame.m_bufferWaitSemaphores[buffer] = { semaphore, stages };
        return semaphore;
    }

    VkSemaphore FrameExecuteContext::AddBufferSignalSemaphore(Handle<Buffer> buffer, VkPipelineStageFlags2 stages)
    {
        auto& frame = CurrentFrame();
        RHI_ASSERT(frame.m_bufferWaitSemaphores.find(buffer) == frame.m_bufferSignalSemaphores.end());
        auto semaphore = m_context->CreateSemaphore();
        frame.m_bufferSignalSemaphores[buffer] = { semaphore, stages };
        return semaphore;
    }

} // namespace RHI::Vulkan