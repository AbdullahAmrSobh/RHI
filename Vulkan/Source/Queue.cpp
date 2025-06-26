
#include "Common.hpp"
#include "Queue.hpp"
#include "CommandList.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    // TODO: dissolve to frame ?

    IQueue::IQueue()  = default;
    IQueue::~IQueue() = default;

    VkResult IQueue::Init(IDevice* device, const char* debugName, uint32_t familyIndex, uint32_t queueIndex)
    {
        m_device = device;

        vkGetDeviceQueue(device->m_device, familyIndex, queueIndex, &m_queue);
        m_familyIndex = familyIndex;

        m_device->SetDebugName(m_queue, debugName);

        VkSemaphoreTypeCreateInfo timelineCreateInfo = {
            .sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext         = nullptr,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue  = 0,
        };
        VkSemaphoreCreateInfo semaphoreInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &timelineCreateInfo,
            .flags = 0,
        };

        VulkanResult result;
        result = vkCreateSemaphore(device->m_device, &semaphoreInfo, nullptr, &m_timelineSemaphore);
        VkResultTry(result);

        m_device->SetDebugName(m_timelineSemaphore, "{}-timeline-semaphore", debugName);
        m_timelineValue.store(0);

        return result;
    }

    void IQueue::Shutdown()
    {
        vkDestroySemaphore(m_device->m_device, m_timelineSemaphore, nullptr);
    }

    bool IQueue::WaitTimeline(uint64_t timelineValue, uint64_t duration)
    {
        ZoneScoped;
        VkSemaphoreWaitInfo waitInfo = {
            .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .pNext          = nullptr,
            .flags          = 0,
            .semaphoreCount = 1,
            .pSemaphores    = &m_timelineSemaphore,
            .pValues        = &timelineValue,
        };
        VulkanResult result = vkWaitSemaphores(m_device->m_device, &waitInfo, duration);
        return result;
    }

    void IQueue::WaitIdle() const
    {
        ZoneScoped;
        vkQueueWaitIdle(m_queue);
    }

    void IQueue::AddWaitSemaphore(VkSemaphore semaphore, uint64_t value, VkPipelineStageFlags2 stageMask)
    {
        ZoneScoped;
        m_waitSemaphores.push_back({
            .sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext       = nullptr,
            .semaphore   = semaphore,
            .value       = value,
            .stageMask   = stageMask,
            .deviceIndex = 0,
        });
    }

    void IQueue::AddSignalSemaphore(VkSemaphore semaphore, uint64_t value, VkPipelineStageFlags2 stageMask)
    {
        ZoneScoped;
        m_signalSemaphores.push_back({
            .sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext       = nullptr,
            .semaphore   = semaphore,
            .value       = value,
            .stageMask   = stageMask,
            .deviceIndex = 0,
        });
    }

    void IQueue::BeginLabel(const char* name)
    {
        if (auto fn = m_device->m_pfn.m_vkQueueBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT label = {
                .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pNext      = nullptr,
                .pLabelName = name,
                .color      = {},
            };
            fn(m_queue, &label);
        }
    }

    void IQueue::EndLabel()
    {
        if (auto fn = m_device->m_pfn.m_vkQueueEndDebugUtilsLabelEXT)
        {
            fn(m_queue);
        }
    }

    uint64_t IQueue::Submit(TL::Span<ICommandList* const> commandLists, VkPipelineStageFlags2 signalStage)
    {
        ZoneScoped;

        VkResult result;

        auto timelineValue = m_timelineValue++;
        AddSignalSemaphore(m_timelineSemaphore, timelineValue, signalStage);

        TL::Vector<VkCommandBufferSubmitInfo> commandBufferSubmitInfos{m_device->GetCurrentFrame()->GetAllocator()};
        for (auto commandList : commandLists)
        {
            commandBufferSubmitInfos.push_back({
                .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .pNext         = nullptr,
                .commandBuffer = commandList->GetHandle(),
                .deviceMask    = 0,
            });
        }

        VkSubmitInfo2 submitInfo2 = {
            .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext                    = nullptr,
            .flags                    = {},
            .waitSemaphoreInfoCount   = static_cast<uint32_t>(m_waitSemaphores.size()),
            .pWaitSemaphoreInfos      = m_waitSemaphores.data(),
            .commandBufferInfoCount   = static_cast<uint32_t>(commandBufferSubmitInfos.size()),
            .pCommandBufferInfos      = commandBufferSubmitInfos.data(),
            .signalSemaphoreInfoCount = static_cast<uint32_t>(m_signalSemaphores.size()),
            .pSignalSemaphoreInfos    = m_signalSemaphores.data(),
        };
        result = vkQueueSubmit2(m_queue, 1, &submitInfo2, VK_NULL_HANDLE);
        TL_ASSERT(result == VK_SUCCESS);

        m_waitSemaphores.clear();
        m_signalSemaphores.clear();

        return timelineValue;
    }

} // namespace RHI::Vulkan
