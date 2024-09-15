#include "Queue.hpp"

#include "Common.hpp"
#include "Context.hpp"
#include "CommandList.hpp"

namespace RHI::Vulkan
{
    IQueue::IQueue(IContext* context, uint32_t familyIndex)
        : m_context(context)
        , m_queue(VK_NULL_HANDLE)
        , m_familyIndex(familyIndex)
    {
        vkGetDeviceQueue(m_context->m_device, familyIndex, 0, &m_queue);
    }

    void IQueue::BeginLabel(const char* name, ColorValue<float> color)
    {
        if (auto fn = m_context->m_pfn.m_vkQueueBeginDebugUtilsLabelEXT)
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

    void IQueue::EndLabel()
    {
        if (auto fn = m_context->m_pfn.m_vkQueueEndDebugUtilsLabelEXT)
        {
            fn(m_queue);
        }
    }

    void IQueue::Submit(TL::Span<ICommandList* const> commandLists, IFence* fence)
    {
        TL::Vector<VkSemaphoreSubmitInfo> waitSemaphoreSIList;
        TL::Vector<VkSemaphoreSubmitInfo> signalSemaphoresSIList;
        TL::Vector<VkCommandBufferSubmitInfo> commandBuffersSIList;

        for (const auto& commandList : commandLists)
        {
            for (auto waitSemaphore : commandList->m_waitSemaphores)
            {
                waitSemaphoreSIList.push_back(waitSemaphore);
            }
            for (auto signalSemaphore : commandList->m_signalSemaphores)
            {
                signalSemaphoresSIList.push_back(signalSemaphore);
            }
            VkCommandBufferSubmitInfo commandBufferSubmitInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .pNext = nullptr,
                .commandBuffer = commandList->m_commandBuffer,
                .deviceMask = 0,
            };
            commandBuffersSIList.push_back(commandBufferSubmitInfo);
        }

        VkSubmitInfo2 info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext = nullptr,
            .flags = 0,
            .waitSemaphoreInfoCount = (uint32_t)waitSemaphoreSIList.size(),
            .pWaitSemaphoreInfos = waitSemaphoreSIList.data(),
            .commandBufferInfoCount = (uint32_t)commandBuffersSIList.size(),
            .pCommandBufferInfos = commandBuffersSIList.data(),
            .signalSemaphoreInfoCount = (uint32_t)signalSemaphoresSIList.size(),
            .pSignalSemaphoreInfos = signalSemaphoresSIList.data(),
        };
        vkQueueSubmit2(m_queue, 1, &info, fence ? fence->UseFence() : VK_NULL_HANDLE);
    }
} // namespace RHI::Vulkan