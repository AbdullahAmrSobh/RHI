#pragma once

#include <RHI/QueueType.hpp>
#include <RHI/Common/Span.hpp>
#include <RHI/Resources.hpp>

#include <cstdint>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;
    class IFence;
    class ICommandList;

    class SyncGroup
    {
    public:
        void AddImage(TL::Span<const Handle<Image>> images);

        void AddBuffer(TL::Span<const Handle<Buffer>> buffers);

        TL::Vector<VkSemaphoreSubmitInfo> GetSemaphoreSubmitInfos() const;

    private:
        TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags> semaphores;
    };

    class Scheduler
    {
    public:
        Scheduler(IContext* context);
        ~Scheduler();

        static SyncGroup MergeSyncGroups(TL::Span<const SyncGroup> syncGroups);

        void SetBufferedFramesInFlights(uint32_t count) const;
        uint32_t GetCurrentFrameIndex() const;

        void FrameBegin();
        void FrameEnd();

        void QueueWaitIdle(QueueType queueType);

        void QueueSubmit(
            QueueType queueType,
            TL::Span<ICommandList* const> commandLists,
            const SyncGroup& waitGroup,
            const SyncGroup& signalGroup,
            IFence* fence
        );
    };
} // namespace RHI::Vulkan