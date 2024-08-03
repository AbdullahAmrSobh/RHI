#pragma once

#include <RHI/Resources.hpp>

#include <TL/Containers.hpp>

#include <vulkan/vulkan.h>

#include <functional>

namespace RHI::Vulkan
{
    class IContext;

    struct SemaphoreStage
    {
        VkSemaphore semaphore;
        VkPipelineStageFlags2 stages;
    };

    class DeleteQueue
    {
    public:
        DeleteQueue();
        ~DeleteQueue();

        void DeferCommand(std::function<void()> callback);

        void Flush();

    private:
        TL::Vector<std::function<void()>> m_callbacks;
    };

    class FrameExecuteContext
    {
    public:
        FrameExecuteContext(IContext* context)
            : m_context(context)
            , m_frameInFlightsCount(3)
        {
        }

        void AdvanceFrame();

        void SetFrameInFlightsCount(uint32_t count);

        uint32_t GetFrameIndex() const;

        uint32_t GetNextFrameIndex() const;

        void DeferCommand(std::function<void()> callback);
        void DeferNextFrame(std::function<void()> callback);

        SemaphoreStage GetImageWaitSemaphore(Handle<Image> image);

        SemaphoreStage GetImageSignalSemaphore(Handle<Image> image);

        SemaphoreStage GetBufferWaitSemaphore(Handle<Buffer> buffer);

        SemaphoreStage GetBufferSignalSemaphore(Handle<Buffer> buffer);

        VkSemaphore AddImageWaitSemaphore(Handle<Image> image, VkPipelineStageFlags2 stages);
        VkSemaphore AddImageSignalSemaphore(Handle<Image> image, VkPipelineStageFlags2 stages);
        VkSemaphore AddBufferWaitSemaphore(Handle<Buffer> buffer, VkPipelineStageFlags2 stages);
        VkSemaphore AddBufferSignalSemaphore(Handle<Buffer> buffer, VkPipelineStageFlags2 stages);

    // private:
        IContext* m_context;

        uint32_t m_frameInFlightsCount;
        uint32_t m_frameIndex;

        struct Data
        {
            TL::UnorderedMap<Handle<Image>, SemaphoreStage> m_imageWaitSemaphores;
            TL::UnorderedMap<Handle<Image>, SemaphoreStage> m_imageSignalSemaphores;
            TL::UnorderedMap<Handle<Buffer>, SemaphoreStage> m_bufferWaitSemaphores;
            TL::UnorderedMap<Handle<Buffer>, SemaphoreStage> m_bufferSignalSemaphores;
            DeleteQueue m_deleteQueue;
        } m_frame[4];

        const auto& CurrentFrame() const { return m_frame[m_frameIndex]; }

        auto& CurrentFrame() { return m_frame[m_frameIndex]; }
    };

} // namespace RHI::Vulkan