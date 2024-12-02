#pragma once

#include "RHI/RenderGraph.hpp"
#include <RHI/Queue.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

#include <TL/Span.hpp>

namespace RHI::Vulkan
{
    class IDevice;
    class ICommandList;

    struct QueueSubmitInfo
    {
        VkSemaphoreSubmitInfo         timelineWaitSemaphores[AsyncQueuesCount] = {};
        VkSemaphoreSubmitInfo         binaryWaitSemaphore                      = {};
        VkSemaphoreSubmitInfo         binarySignalSemaphore                    = {};
        TL::Span<ICommandList* const> commandLists                             = {};
        VkPipelineStageFlags2         timelineSignalStages;
    };

    class IQueue
    {
    public:
        IQueue();
        ~IQueue();

        /// Initializes the queue with a given Vulkan device and queue creation info.
        ResultCode Init(IDevice* device, uint32_t familyIndex, uint32_t queueIndex);
        void       Shutdown();

        /// Returns the Vulkan queue handle.
        inline VkQueue GetHandle() const { return m_queue; }

        /// Returns the queue family index.
        inline uint32_t GetFamilyIndex() const { return m_familyIndex; }

        /// Returns the current semaphore timeline value (the last completed value).
        uint64_t GetTimelineSemaphoreValue() const;

        /// Returns the next timeline semaphore value to be signaled.
        uint64_t GetTimelineSemaphorePendingValue() const;

        /// Returns the Vulkan semaphore handle for the timeline.
        VkSemaphore GetTimelineSemaphoreHandle() const { return m_timeline; }

        /// Begins a debug label for the queue (for debugging tools like RenderDoc).
        void BeginLabel(const char* name, const float color[4]);

        /// Ends the debug label for the queue.
        void EndLabel();

        /// Submits work to the queue with synchronization information.
        /// @param submitInfo - Describes the semaphores, pipeline stages, and other synchronization details.
        /// @returns The next timeline semaphore value after the submission.
        uint64_t Submit(const QueueSubmitInfo& submitInfo);

    private:
        IDevice*              m_device               = nullptr;
        VkQueue               m_queue                = VK_NULL_HANDLE;
        uint32_t              m_familyIndex          = 0;
        VkSemaphore           m_timeline             = VK_NULL_HANDLE; // Handle to the timeline semaphore.
        std::atomic_uint64_t  m_timelineValue        = 0;              // Current timeline value.
        VkPipelineStageFlags2 m_timelineSignalStages = 0;              // Signal stages for timeline semaphore.
    };
} // namespace RHI::Vulkan