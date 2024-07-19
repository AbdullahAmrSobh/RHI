#include "Queue.hpp"

#include "Context.hpp"
#include "VulkanFunctions.hpp"
#include "CommandList.hpp"

namespace RHI::Vulkan
{
    Queue::Queue(VkDevice device, uint32_t familyIndex)
        : m_queue(VK_NULL_HANDLE)
        , m_familyIndex(familyIndex)
    {
        vkGetDeviceQueue(device, familyIndex, 0, &m_queue);
    }

    void Queue::BeginLabel(IContext* context, const char* name, ColorValue<float> color)
    {
        if (auto fn = context->m_fnTable->m_vkQueueBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT labelInfo{};
            labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            labelInfo.pNext = nullptr;
            labelInfo.pLabelName = name;
            labelInfo.color[0] = color.r;
            labelInfo.color[1] = color.g;
            labelInfo.color[2] = color.b;
            labelInfo.color[3] = color.a;
            fn(m_queue, &labelInfo);
        }
    }

    void Queue::EndLabel(IContext* context)
    {
        if (auto fn = context->m_fnTable->m_vkQueueEndDebugUtilsLabelEXT)
        {
            fn(m_queue);
        }
    }

    void Queue::Submit([[maybe_unused]] IContext* context, SubmitInfo submitGroups, IFence* fence)
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

        vkDeviceWaitIdle(context->m_device);
    }
} // namespace RHI::Vulkan