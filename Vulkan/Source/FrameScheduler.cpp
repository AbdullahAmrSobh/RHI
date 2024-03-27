#include "FrameScheduler.hpp"
#include "Context.hpp"
#include "Resources.hpp"
#include "CommandList.hpp"
#include "CommandListAllocator.hpp"
#include "Common.hpp"

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    IFrameScheduler::IFrameScheduler(IContext* context)
        : FrameScheduler(context)
    {
        m_commandListAllocator = CreatePtr<ICommandListAllocator>(context);
    }

    IFrameScheduler::~IFrameScheduler()
    {
    }

    VkResult IFrameScheduler::Init()
    {
        auto context = (IContext*)m_context;
        vkGetDeviceQueue(context->m_device, context->m_graphicsQueueFamilyIndex, 0, &m_graphicsQueue);
        vkGetDeviceQueue(context->m_device, context->m_computeQueueFamilyIndex, 0, &m_computeQueue);
        vkGetDeviceQueue(context->m_device, context->m_transferQueueFamilyIndex, 0, &m_transferQueue);

        ((ICommandListAllocator*)m_commandListAllocator.get())->Init();

        return VK_SUCCESS;
    }

    void IFrameScheduler::PassSubmit(Pass* pass, Fence* _fence)
    {
        auto fence = (IFence*)_fence;
        auto queue = pass->m_queueType == QueueType::Graphics ? m_graphicsQueue : m_computeQueue;
        auto& commandlists = pass->m_commandLists;

        TL::Vector<VkSemaphoreSubmitInfo> waitSemaphores;
        TL::Vector<VkSemaphoreSubmitInfo> signalSemaphores;
        TL::Vector<VkCommandBufferSubmitInfo> commandBuffers;

        for (auto _commandList : commandlists)
        {
            auto commandList = (ICommandList*)_commandList;
            waitSemaphores.insert(waitSemaphores.end(), commandList->m_waitSemaphores.begin(), commandList->m_waitSemaphores.end());
            signalSemaphores.insert(signalSemaphores.end(), commandList->m_signalSemaphores.begin(), commandList->m_signalSemaphores.end());

            commandList->m_signalSemaphores.clear();
            commandList->m_waitSemaphores.clear();

            auto& commandBuffer = commandBuffers.emplace_back();
            commandBuffer.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            commandBuffer.pNext = nullptr;
            commandBuffer.commandBuffer = commandList->m_commandBuffer;
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
        vkQueueSubmit2(queue, 1, &submitInfo, fence ? fence->UseFence() : VK_NULL_HANDLE);

        commandlists.clear();
    }

    void IFrameScheduler::StageImageWrite(const BufferToImageCopyInfo& copyInfo)
    {
        auto context = (IContext*)m_context;
        auto image = context->m_imageOwner.Get(copyInfo.dstImage);

        image->waitSemaphore = context->CreateSemaphore();

        auto commandList = (ICommandList*)m_commandListAllocator->Allocate(QueueType::Transfer);

        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.pNext = nullptr;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.srcAccessMask = 0;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image->handle;
        auto layer = ConvertSubresourceLayer(copyInfo.dstSubresource);
        barrier.subresourceRange.aspectMask = layer.aspectMask;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = 1;
        commandList->Begin();
        commandList->PipelineBarrier({}, {}, barrier);
        commandList->Copy(copyInfo);
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.dstAccessMask = 0;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        commandList->PipelineBarrier({}, {}, barrier);
        commandList->End();

        VkCommandBufferSubmitInfo cmdBufSubmitInfo{};
        cmdBufSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdBufSubmitInfo.pNext = nullptr;
        cmdBufSubmitInfo.commandBuffer = ((ICommandList*)commandList)->m_commandBuffer;

        VkSemaphoreSubmitInfo semaphoreSubmitInfo{};
        semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        semaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
        semaphoreSubmitInfo.semaphore = image->waitSemaphore;

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.pNext = nullptr;
        submitInfo.flags = 0;
        submitInfo.waitSemaphoreInfoCount = 0;
        submitInfo.pWaitSemaphoreInfos = nullptr;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cmdBufSubmitInfo;
        submitInfo.signalSemaphoreInfoCount = 0;
        // submitInfo.pSignalSemaphoreInfos = &semaphoreSubmitInfo;
        vkQueueSubmit2(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

        // TODO: defer cleanup
    }
} // namespace RHI::Vulkan