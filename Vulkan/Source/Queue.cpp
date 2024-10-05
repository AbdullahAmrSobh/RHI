#include "Queue.hpp"

#include "Common.hpp"
#include "Context.hpp"
#include "CommandList.hpp"

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    IQueue::IQueue(IContext* context, uint32_t familyIndex)
        : m_context(context)
        , m_queue(VK_NULL_HANDLE)
        , m_familyIndex(familyIndex)
    {
        vkGetDeviceQueue(m_context->m_device, familyIndex, 0, &m_queue);
    }

    void IQueue::BeginLabel(const char* name, float color[4])
    {
        if (auto fn = m_context->m_pfn.m_vkQueueBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT labelInfo{};
            labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            labelInfo.pNext = nullptr;
            labelInfo.pLabelName = name;
            labelInfo.color[0] = color[0];
            labelInfo.color[1] = color[1];
            labelInfo.color[2] = color[2];
            labelInfo.color[3] = color[3];
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

    void IQueue::Submit(TL::Span<const SubmitInfo> submitInfos, Fence* _fence)
    {
        auto fence = (IFence*)_fence;

        for (const auto& submitInfo : submitInfos)
        {
            // Vectors to hold Vulkan semaphore and command buffer submit info structures
            TL::Vector<VkSemaphoreSubmitInfo> waitSemaphoreSIList;
            TL::Vector<VkSemaphoreSubmitInfo> signalSemaphoreSIList;
            TL::Vector<VkCommandBufferSubmitInfo> commandBufferSIList;

            // Process wait semaphores
            for (const auto& waitSemaphore : submitInfo.waitSemaphores)
            {
                VkSemaphoreSubmitInfo waitSemaphoreInfo{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                    .pNext = nullptr,
                    .semaphore = m_context->m_semaphoreOwner.Get(waitSemaphore.semaphore)->handle, // Get the VkSemaphore handle
                    .value = waitSemaphore.value,
                    .stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, // TODO:
                    .deviceIndex = 0,
                };
                waitSemaphoreSIList.push_back(waitSemaphoreInfo);
            }

            // Process signal semaphores
            for (const auto& signalSemaphore : submitInfo.signalSemaphores)
            {
                VkSemaphoreSubmitInfo signalSemaphoreInfo{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                    .pNext = nullptr,
                    .semaphore = m_context->m_semaphoreOwner.Get(signalSemaphore.semaphore)->handle, // Get the VkSemaphore handle
                    .value = signalSemaphore.value,
                    .stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, // TODO:
                    .deviceIndex = 0,
                };
                signalSemaphoreSIList.push_back(signalSemaphoreInfo);
            }

            // Process command lists (command buffers)
            for (CommandList* const commandList : submitInfo.commandLists)
            {
                VkCommandBufferSubmitInfo commandBufferSubmitInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                    .pNext = nullptr,
                    .commandBuffer = ((ICommandList*)commandList)->m_commandBuffer, // Get the VkCommandBuffer handle
                    .deviceMask = 0,
                };
                commandBufferSIList.push_back(commandBufferSubmitInfo);
            }

            // Create the VkSubmitInfo2 structure for queue submission
            VkSubmitInfo2 submitInfo2{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .pNext = nullptr,
                .flags = 0,
                .waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphoreSIList.size()),
                .pWaitSemaphoreInfos = waitSemaphoreSIList.data(),
                .commandBufferInfoCount = static_cast<uint32_t>(commandBufferSIList.size()),
                .pCommandBufferInfos = commandBufferSIList.data(),
                .signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphoreSIList.size()),
                .pSignalSemaphoreInfos = signalSemaphoreSIList.data(),
            };

            // Submit the batch to the Vulkan queue
            vkQueueSubmit2(m_queue, 1, &submitInfo2, fence ? fence->UseFence() : VK_NULL_HANDLE);
        }
    }

} // namespace RHI::Vulkan