#include "Queue.hpp"

#include <tracy/Tracy.hpp>

#include "CommandList.hpp"
#include "Common.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"

namespace RHI::Vulkan
{
    IQueue::IQueue()  = default;
    IQueue::~IQueue() = default;

    ResultCode IQueue::Init(IDevice* device, const char* debugName, uint32_t familyIndex, uint32_t queueIndex)
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
        };

        if (vkCreateSemaphore(device->m_device, &semaphoreInfo, nullptr, &m_timeline) != VK_SUCCESS)
        {
            m_device->SetDebugName(m_queue, std::format("{}-timeline-semaphore", debugName).c_str());
            return ResultCode::ErrorUnknown;
        }

        m_timelineValue.store(0);
        return ResultCode::Success;
    }

    void IQueue::Shutdown()
    {
        vkDestroySemaphore(m_device->m_device, m_timeline, nullptr);
    }

    void IQueue::BeginLabel(const char* name, const float color[4])
    {
        if (auto fn = m_device->m_pfn.m_vkQueueBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT label = {
                .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pNext      = nullptr,
                .pLabelName = name,
                .color      = {color[0], color[1], color[2], color[3]},
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

    uint64_t IQueue::GetTimelineSemaphoreValue() const
    {
        uint64_t value;
        vkGetSemaphoreCounterValue(m_device->m_device, m_timeline, &value);
        return value;
    }

    uint64_t IQueue::GetTimelineSemaphorePendingValue() const
    {
        return m_timelineValue;
    }

    uint64_t IQueue::Submit(const QueueSubmitInfo& submitInfo)
    {
        uint64_t nextValue = m_timelineValue.fetch_add(1) + 1;

        VkSemaphoreSubmitInfo waitSemaphoreInfos[64]; // Assuming a reasonable limit
        uint32_t              waitCount = 0;
        for (const auto& semaphoreInfo : submitInfo.waitSemaphores)
        {
            waitSemaphoreInfos[waitCount++] = semaphoreInfo;
        }

        VkSemaphoreSubmitInfo signalSemaphoreInfos[64]; // Assuming a reasonable limit
        uint32_t              signalCount = 0;
        for (const auto& semaphoreInfo : submitInfo.signalSemaphores)
        {
            signalSemaphoreInfos[signalCount++] = semaphoreInfo;
        }

        signalSemaphoreInfos[signalCount++] = {
            .sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext       = nullptr,
            .semaphore   = m_timeline,
            .value       = m_timelineValue,
            .stageMask   = ConvertPipelineStageFlags(submitInfo.signalStage),
            .deviceIndex = 0,
        };

        // Prepare the command buffer infos
        VkCommandBufferSubmitInfo commandBufferSubmitInfos[64]; // Assuming a reasonable limit
        uint32_t                  commandBufferCount = 0;

        for (const auto* commandList : submitInfo.commandLists)
        {
            if (commandList != nullptr)
            {
                commandBufferSubmitInfos[commandBufferCount++] = {
                    .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                    .pNext         = nullptr,
                    .commandBuffer = commandList->GetHandle(),
                    .deviceMask    = 0,
                };
            }
        }

        VkSubmitInfo2 submitInfo2 = {
            .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext                    = nullptr,
            .waitSemaphoreInfoCount   = waitCount,
            .pWaitSemaphoreInfos      = waitSemaphoreInfos,
            .commandBufferInfoCount   = commandBufferCount,
            .pCommandBufferInfos      = commandBufferSubmitInfos,
            .signalSemaphoreInfoCount = signalCount,
            .pSignalSemaphoreInfos    = signalSemaphoreInfos,
        };

        auto result = vkQueueSubmit2(m_queue, 1, &submitInfo2, VK_NULL_HANDLE);
        Validate(result);

        return nextValue;
    }

} // namespace RHI::Vulkan