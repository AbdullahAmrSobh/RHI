#pragma once

#include <RHI/PipelineAccess.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

#include <TL/Containers/Vector.hpp>

#include <TL/Span.hpp>

#include <atomic>

namespace RHI::Vulkan
{
    class IDevice;
    class ICommandList;

    class IQueue
    {
    public:
        IQueue();
        ~IQueue();

        VkResult Init(IDevice* device, const char* debugName, uint32_t familyIndex, uint32_t queueIndex);
        void     Shutdown();

        inline VkQueue GetHandle() const { return m_queue; }

        inline VkSemaphore GetTimelineHandle() const { return m_timelineSemaphore; }

        inline uint32_t GetFamilyIndex() const { return m_familyIndex; }

        inline uint64_t GetTimelineValue() const { return m_timelineValue; }

        void Wait() const;
        bool Wait(uint64_t timelineValue, uint64_t duration = UINT64_MAX);

        void AddWaitSemaphore(VkSemaphore semaphore, uint64_t value, VkPipelineStageFlags2 stageMask);

        inline void AddWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags2 stageMask) { AddWaitSemaphore(semaphore, 0, stageMask); }

        void AddSignalSemaphore(VkSemaphore semaphore, uint64_t value, VkPipelineStageFlags2 stageMask);

        inline void AddSignalSemaphore(VkSemaphore semaphore, VkPipelineStageFlags2 stageMask) { AddSignalSemaphore(semaphore, 0, stageMask); }

        void     BeginLabel(const char* name);
        void     EndLabel();
        uint64_t Submit(TL::Span<ICommandList* const> commandLists, VkPipelineStageFlags2 signalStage);

    private:
        IDevice*                          m_device;
        VkQueue                           m_queue;
        uint32_t                          m_familyIndex;
        VkSemaphore                       m_timelineSemaphore;
        std::atomic_uint64_t              m_timelineValue;
        // Flushed on submit
        TL::Vector<VkSemaphoreSubmitInfo> m_waitSemaphores;
        TL::Vector<VkSemaphoreSubmitInfo> m_signalSemaphores;
    };
} // namespace RHI::Vulkan
