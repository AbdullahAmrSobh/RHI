#pragma once

#include "Resources.hpp"
#include "CommandList.hpp"

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    struct SubmitInfo
    {
        TL::Span<const VkSemaphoreSubmitInfoKHR> waitSemaphores;
        TL::Span<const VkSemaphoreSubmitInfoKHR> signalSemaphores;
        TL::Span<ICommandList* const> commandLists;
    };

    class Queue
    {
    public:
        Queue() = default;

        Queue(VkDevice device, uint32_t familyIndex)
            : m_queue(VK_NULL_HANDLE)
            , m_familyIndex(familyIndex)
        {
            vkGetDeviceQueue(device, familyIndex, 0, &m_queue);
        }

        void Submit(SubmitInfo submitGroups, IFence* fence)
        {
            TL::Vector<VkCommandBufferSubmitInfo> commandBufferSubmitInfos;
            for (const auto& commandList : submitGroups.commandLists)
            {
                VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
                commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
                commandBufferSubmitInfo.commandBuffer = commandList->m_commandBuffer;
                commandBufferSubmitInfos.push_back(commandBufferSubmitInfo);
            }

            VkSubmitInfo2 info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
            info.pNext = nullptr;
            info.flags = 0;
            info.waitSemaphoreInfoCount = (uint32_t)submitGroups.waitSemaphores.size();
            info.pWaitSemaphoreInfos = submitGroups.waitSemaphores.data();
            info.commandBufferInfoCount = (uint32_t)commandBufferSubmitInfos.size();
            info.pCommandBufferInfos = commandBufferSubmitInfos.data();
            info.signalSemaphoreInfoCount = (uint32_t)submitGroups.signalSemaphores.size();
            info.pSignalSemaphoreInfos = submitGroups.signalSemaphores.data();
            vkQueueSubmit2(m_queue, 1, &info, fence ? fence->UseFence() : VK_NULL_HANDLE);
        }

        VkQueue GetHandle() const { return m_queue; }

        uint32_t GetFamilyIndex() const { return m_familyIndex; }

    private:
        VkQueue m_queue;
        uint32_t m_familyIndex;
    };
} // namespace RHI::Vulkan