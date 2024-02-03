#include "FrameScheduler.hpp"
#include "Context.hpp"
#include "Common.hpp"
#include "Resources.hpp"
#include "CommandList.hpp"
#include "TransientResourceAllocator.inl"

namespace Vulkan
{

    ///////////////////////////////////////////////////////////////////////////
    /// Pass
    ///////////////////////////////////////////////////////////////////////////

    Pass::Pass(Context* context, const char* name, RHI::QueueType queueType)
        : RHI::Pass(name, queueType)
        , m_context(context)
        , m_signalSemaphore(VK_NULL_HANDLE)
    {
    }

    Pass::~Pass()
    {
    }

    VkResult Pass::Init()
    {
        m_signalSemaphore = m_context->CreateSemaphore();
        return VK_SUCCESS;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// FrameScheduler
    ///////////////////////////////////////////////////////////////////////////

    FrameScheduler::FrameScheduler(Context* context)
        : RHI::FrameScheduler(context)
    {
    }

    FrameScheduler::~FrameScheduler()
    {
    }

    VkResult FrameScheduler::Init()
    {
        m_transientResourceAllocator = TransientResourceAllocator::Create();
        return VK_SUCCESS;
    }

    void FrameScheduler::DeviceWaitIdle()
    {
        auto context = (Context*)m_context;
        auto result = vkDeviceWaitIdle(context->m_device);
        VULKAN_ASSERT_SUCCESS(result);
    }

    void FrameScheduler::QueuePassSubmit(RHI::Pass* _pass, RHI::Fence* _fence)
    {
        auto context = (Context*)m_context;
        auto pass = (Pass*)_pass;
        auto queue = context->GetQueue(pass->m_queueType);

        VkSemaphoreSubmitInfo presentReadySemaphore = {};
        presentReadySemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        presentReadySemaphore.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        if (auto passAttachment = pass->m_swapchainImageAttachment; passAttachment && passAttachment->next == nullptr)
        {
            auto swapchain = (Swapchain*)passAttachment->attachment->swapchain;
            presentReadySemaphore.semaphore = swapchain->GetCurrentImageSemaphore();
        }

        auto waitSemaphores = GetPassWaitSemaphoresInfos({ (Pass**)pass->m_producers.data(), pass->m_producers.size() });
        auto commandBuffers = GetPassCommandBuffersSubmitInfos({ (CommandList**)pass->m_commandLists.data(), pass->m_commandLists.size() });

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.pNext = nullptr;
        submitInfo.flags = 0;
        submitInfo.waitSemaphoreInfoCount = uint32_t(waitSemaphores.size());
        submitInfo.pWaitSemaphoreInfos = waitSemaphores.data();
        submitInfo.commandBufferInfoCount = uint32_t(commandBuffers.size());
        submitInfo.pCommandBufferInfos = commandBuffers.data();

        if (presentReadySemaphore.semaphore != VK_NULL_HANDLE)
        {
            submitInfo.signalSemaphoreInfoCount = 1;
            submitInfo.pSignalSemaphoreInfos = &presentReadySemaphore;
        }

        auto fence = (Fence*)_fence;
        auto result = vkQueueSubmit2(queue, 1, &submitInfo, fence ? fence->UseFence() : VK_NULL_HANDLE);
        VULKAN_ASSERT_SUCCESS(result);
    }

    void FrameScheduler::QueueCommandsSubmit(RHI::QueueType queueType, RHI::TL::Span<RHI::CommandList*> _commandLists, RHI::Fence& _fence)
    {
        auto context = (Context*)m_context;
        auto& fence = (Fence&)_fence;
        auto queue = context->GetQueue(queueType);

        auto commandLists = RHI::TL::Span<CommandList*>((CommandList**)_commandLists.data(), _commandLists.size());
        auto commandBuffersSubmitInfos = GetPassCommandBuffersSubmitInfos(commandLists);

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.pNext = nullptr;
        submitInfo.flags = 0;
        submitInfo.waitSemaphoreInfoCount = 0;
        submitInfo.pWaitSemaphoreInfos = nullptr;
        submitInfo.commandBufferInfoCount = uint32_t(commandLists.size());
        submitInfo.pCommandBufferInfos = commandBuffersSubmitInfos.data();
        submitInfo.signalSemaphoreInfoCount = 0;
        submitInfo.pSignalSemaphoreInfos = nullptr;
        auto result = vkQueueSubmit2(queue, 1, &submitInfo, fence.UseFence());
        VULKAN_ASSERT_SUCCESS(result);
    }

    std::vector<VkCommandBufferSubmitInfo> FrameScheduler::GetPassCommandBuffersSubmitInfos(RHI::TL::Span<CommandList*> commandLists)
    {
        std::vector<VkCommandBufferSubmitInfo> submitInfos{};
        for (auto commandList : commandLists)
        {
            VkCommandBufferSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            submitInfo.commandBuffer = commandList->m_commandBuffer;
            submitInfos.push_back(submitInfo);
        }
        return submitInfos;
    }

    std::vector<VkSemaphoreSubmitInfo> FrameScheduler::GetPassWaitSemaphoresInfos(RHI::TL::Span<Pass*> passList)
    {
        std::vector<VkSemaphoreSubmitInfo> semaphores;
        for (auto pass : passList)
        {
            if (auto swapchain = (Swapchain*)pass->m_swapchain)
            {
                VkSemaphoreSubmitInfo submitInfo {};
                submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;
                submitInfo.semaphore = swapchain->GetCurrentImageSemaphore();
                submitInfo.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
                semaphores.push_back(submitInfo);
            }
        }
        return semaphores;
    }


} // namespace Vulkan