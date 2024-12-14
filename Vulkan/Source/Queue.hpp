#pragma once

#include <RHI/PipelineAccess.hpp>
#include <RHI/Queue.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

#include <TL/Containers.hpp>
#include <TL/Memory.hpp>
#include <TL/Span.hpp>

namespace RHI::Vulkan
{
    class IDevice;
    class ICommandList;

    /// @brief Structure to provide information for submitting commands to a queue.
    ///
    /// This structure includes semaphores for synchronization and the command lists
    /// to be executed on the queue.
    struct QueueSubmitInfo
    {
        QueueSubmitInfo(IDevice& device);

        // Getters for iterating over the lists

        TL::Span<const VkSemaphoreSubmitInfo> GetWaitSemaphores() const { return waitSemaphores; }

        TL::Span<const VkCommandBufferSubmitInfo> GetCommandLists() const { return commandLists; }

        TL::Span<const VkSemaphoreSubmitInfo> GetSignalSemaphores() const { return signalSemaphores; }

        void AddWaitSemaphore(VkSemaphore semaphore, uint64_t value, VkPipelineStageFlags2 stageMask)
        {
            if (semaphore == VK_NULL_HANDLE) return;
            waitSemaphores.push_back({.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .semaphore = semaphore, .value = value, .stageMask = stageMask});
        }

        void AddCommandList(VkCommandBuffer commandList)
        {
            if (commandList == VK_NULL_HANDLE) return;
            commandLists.push_back({.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .commandBuffer = commandList});
        }

        void AddSignalSemaphore(VkSemaphore semaphore, uint64_t value, VkPipelineStageFlags2 stageMask)
        {
            if (semaphore == VK_NULL_HANDLE) return;
            signalSemaphores.push_back({.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .semaphore = semaphore, .value = value, .stageMask = stageMask});
        }

        TL::Vector<VkSemaphoreSubmitInfo, TL::IAllocator>     waitSemaphores;
        TL::Vector<VkCommandBufferSubmitInfo, TL::IAllocator> commandLists;
        TL::Vector<VkSemaphoreSubmitInfo, TL::IAllocator>     signalSemaphores;
        VkPipelineStageFlags2                                 signalStage = 0;
    };

    /// @brief Represents a Vulkan queue used for command submission.
    ///
    /// The IQueue class abstracts a Vulkan queue, allowing submission of commands,
    /// synchronization using timeline semaphores, and debug labeling for GPU markers.
    class IQueue
    {
    public:
        IQueue();
        ~IQueue();

        // disable copy/move semantics
        IQueue(const IQueue&)            = delete;
        IQueue(IQueue&&)                 = delete;
        IQueue& operator=(const IQueue&) = delete;
        IQueue& operator=(IQueue&&)      = delete;

        /// @brief Initializes the queue with the specified device and indices.
        /// @param device The Vulkan device.
        /// @param debugName The debug name for the queue.
        /// @param familyIndex The queue family index.
        /// @param queueIndex The queue index within the family.
        /// @return ResultCode indicating success or failure.
        ResultCode Init(IDevice* device, const char* debugName, uint32_t familyIndex, uint32_t queueIndex);

        /// @brief Shuts down the queue and releases resources.
        void Shutdown();

        /// @brief Retrieves the Vulkan queue handle.
        /// @return The Vulkan queue handle.
        VkQueue GetHandle() const { return m_queue; }

        /// @brief Retrieves the timeline semaphore handle.
        /// @return The Vulkan timeline semaphore handle.
        VkSemaphore GetTimelineHandle() const { return m_timeline; }

        /// @brief Retrieves the queue family index.
        /// @return The family index of the queue.
        uint32_t GetFamilyIndex() const { return m_familyIndex; }

        /// @brief Begins a debug label region on the queue.
        /// @param name The name of the debug label.
        /// @param color The color of the debug label.
        void BeginLabel(const char* name, const float color[4]);

        /// @brief Ends the current debug label region on the queue.
        void EndLabel();

        /// @brief Submits commands and synchronization primitives to the queue.
        /// @param submitInfo Information about the commands and synchronization primitives.
        /// @return The timeline value after submission.
        uint64_t Submit(QueueSubmitInfo& submitInfo);

        /// @brief Retrieves the current timeline semaphore value.
        /// @return The current timeline semaphore value.
        uint64_t GetTimelineValue() const;

        /// @brief Retrieves the pending timeline semaphore value.
        /// @return The pending timeline semaphore value.
        uint64_t GetTimelinePendingValue() const;

        /// @brief Waits for the timeline semaphore to reach a specified value.
        /// @param timelineValue The timeline value to wait for.
        /// @param duration The maximum time to wait (default: UINT64_MAX).
        /// @return The true if the wait is completed without timing-out.
        bool WaitTimeline(uint64_t timelineValue, uint64_t duration = UINT64_MAX);

        /// @brief Signals the timeline semaphore with a specified value.
        /// @param timelineValue The value to signal.
        /// @return The signaled timeline value.
        uint64_t SignalTimeline(uint64_t timelineValue);

        operator bool() const { return m_queue != VK_NULL_HANDLE; }

    private:
        IDevice*             m_device        = nullptr;        ///< Pointer to the Vulkan device.
        VkQueue              m_queue         = VK_NULL_HANDLE; ///< Vulkan queue handle.
        uint32_t             m_familyIndex   = 0;              ///< Queue family index.
        VkSemaphore          m_timeline      = VK_NULL_HANDLE; ///< Timeline semaphore handle.
        std::atomic_uint64_t m_timelineValue = 0;              ///< Atomic value for the timeline semaphore.
    };
} // namespace RHI::Vulkan
