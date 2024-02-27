#include "FrameScheduler.hpp"
#include "Context.hpp"
#include "Common.hpp"
#include "Resources.hpp"
#include "CommandList.hpp"
#include "TransientResourceAllocator.inl"
#include "Swapchain.hpp"

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    IFrameScheduler::IFrameScheduler(IContext* context)
        : FrameScheduler(context)
    {
    }

    IFrameScheduler::~IFrameScheduler()
    {
        ZoneScoped;

    }

    VkResult IFrameScheduler::Init()
    {
        m_transientResourceAllocator = ITransientResourceAllocator::Create();

        for (uint32_t i = 0; i < 2; i++)
        {
            m_frameReadyFence.emplace_back(m_context->CreateFence());
        }

        return VK_SUCCESS;
    }

    void IFrameScheduler::DeviceWaitIdle()
    {
        ZoneScoped;

        auto context = (IContext*)m_context;
        auto result = vkDeviceWaitIdle(context->m_device);
        VULKAN_ASSERT_SUCCESS(result);
    }

    void IFrameScheduler::QueuePassSubmit(Pass* pass, Fence* _fence)
    {
        ZoneScoped;

        auto context = (IContext*)m_context;
        auto queue = context->GetQueue(pass->m_queueType);

        std::vector<VkCommandBufferSubmitInfo> commandBuffers{};
        for (auto _commandList : pass->m_commandLists)
        {
            auto commandList = (ICommandList*)_commandList;
            VkCommandBufferSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            submitInfo.commandBuffer = commandList->m_commandBuffer;
            commandBuffers.push_back(submitInfo);
        }

        std::vector<VkSemaphoreSubmitInfo> waitSemaphores{};
        std::vector<VkSemaphoreSubmitInfo> signalSemaphores{};

        if (auto passAttachment = pass->m_swapchainImageAttachment)
        {
            auto swapchain = (ISwapchain*)passAttachment->attachment->swapchain;
            VkSemaphoreSubmitInfo semaphoreSubmitInfo{};
            semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            if (passAttachment->prev == nullptr)
            {
                semaphoreSubmitInfo.semaphore = swapchain->m_semaphores.imageAcquired;
                semaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                waitSemaphores.push_back(semaphoreSubmitInfo);
            }

            if (passAttachment->next == nullptr)
            {
                semaphoreSubmitInfo.semaphore = swapchain->m_semaphores.imageRenderComplete;
                semaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
                signalSemaphores.push_back(semaphoreSubmitInfo);
            }
        }

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.pNext = nullptr;
        submitInfo.flags = 0;
        submitInfo.waitSemaphoreInfoCount = uint32_t(waitSemaphores.size());
        submitInfo.pWaitSemaphoreInfos = waitSemaphores.data();
        submitInfo.signalSemaphoreInfoCount = uint32_t(signalSemaphores.size());
        submitInfo.pSignalSemaphoreInfos = signalSemaphores.data();
        submitInfo.commandBufferInfoCount = uint32_t(commandBuffers.size());
        submitInfo.pCommandBufferInfos = commandBuffers.data();

        auto fence = (IFence*)_fence;
        auto result = vkQueueSubmit2(queue, 1, &submitInfo, fence ? fence->UseFence() : VK_NULL_HANDLE);
        VULKAN_ASSERT_SUCCESS(result);
    }

    void IFrameScheduler::QueueCommandsSubmit(QueueType queueType, TL::Span<CommandList*> commandLists, Fence& _fence)
    {
        ZoneScoped;

        auto context = (IContext*)m_context;
        auto& fence = (IFence&)_fence;
        auto queue = context->GetQueue(queueType);

        std::vector<VkCommandBufferSubmitInfo> commandBuffers{};
        for (auto _commandList : commandLists)
        {
            auto commandList = (ICommandList*)_commandList;
            VkCommandBufferSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            submitInfo.commandBuffer = commandList->m_commandBuffer;
            commandBuffers.push_back(submitInfo);
        }

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.pNext = nullptr;
        submitInfo.flags = 0;
        submitInfo.waitSemaphoreInfoCount = 0;
        submitInfo.pWaitSemaphoreInfos = nullptr;
        submitInfo.commandBufferInfoCount = uint32_t(commandBuffers.size());
        submitInfo.pCommandBufferInfos = commandBuffers.data();
        submitInfo.signalSemaphoreInfoCount = 0;
        submitInfo.pSignalSemaphoreInfos = nullptr;
        auto result = vkQueueSubmit2(queue, 1, &submitInfo, fence.UseFence());
        VULKAN_ASSERT_SUCCESS(result);
    }

} // namespace RHI::Vulkan