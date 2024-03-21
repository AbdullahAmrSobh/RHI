#include "FrameScheduler.hpp"
#include "Context.hpp"
#include "Resources.hpp"
#include "StagingBuffer.hpp"
#include "CommandList.hpp"
#include "TransientAllocator.hpp"

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    IFrameScheduler::IFrameScheduler(IContext* context)
        : FrameScheduler(context)
    {
        m_stagingBuffer = CreatePtr<IStagingBuffer>(context);
        m_transientAllocator = CreatePtr<ITransientAllocator>(context);
        m_commandListAllocator = CreatePtr<ICommandListAllocator>(context);
        m_currentFrameIndex = 0;
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

        ((IStagingBuffer*)m_stagingBuffer.get())->Init();

        ((ICommandListAllocator*)m_commandListAllocator.get())->Init();

        return VK_SUCCESS;
    }

    void IFrameScheduler::Flush()
    {
        auto context = (IContext*)m_context;

        for (auto imageHandle : m_stagedWriteImages)
        {
            auto image = context->m_imageOwner.Get(imageHandle);
            context->FreeSemaphore(image->signalSemaphore);
            image->signalSemaphore = VK_NULL_HANDLE;
        }

        for (auto bufferHandle : m_stagedWriteBuffers)
        {
            auto buffer = context->m_bufferOwner.Get(bufferHandle);
            context->FreeSemaphore(buffer->waitSemaphore);
            buffer->waitSemaphore = VK_NULL_HANDLE;
        }

        for (auto imageHandle : m_stagedReadImages)
        {
            auto image = context->m_imageOwner.Get(imageHandle);
            context->FreeSemaphore(image->signalSemaphore);
            image->signalSemaphore = VK_NULL_HANDLE;
        }

        for (auto bufferHandle : m_stagedReadBuffers)
        {
            auto buffer = context->m_bufferOwner.Get(bufferHandle);
            context->FreeSemaphore(buffer->waitSemaphore);
            buffer->waitSemaphore = VK_NULL_HANDLE;
        }

        // context->DestroyResources();
    }

    void IFrameScheduler::WaitIdle()
    {
        auto context = (IContext*)m_context;
        vkDeviceWaitIdle(context->m_device);
    }

    void IFrameScheduler::PassSubmit(Pass* pass, Fence* _fence)
    {
        auto fence = (IFence*)_fence;
        auto queue = pass->m_queueType == QueueType::Graphics ? m_graphicsQueue : m_computeQueue;
        auto& commandlists = pass->m_commandLists;

        std::vector<VkSemaphoreSubmitInfo> waitSemaphores;
        std::vector<VkSemaphoreSubmitInfo> signalSemaphores;
        std::vector<VkCommandBufferSubmitInfo> commandBuffers;

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

    void IFrameScheduler::StageImageWrite(Handle<Image> handle)
    {
        auto context = (IContext*)m_context;
        auto image = context->m_imageOwner.Get(handle);
        image->waitSemaphore = context->CreateSemaphore();
    }

    void IFrameScheduler::StageBufferWrite(Handle<Buffer> handle)
    {
        auto context = (IContext*)m_context;
        auto buffer = context->m_bufferOwner.Get(handle);
        buffer->waitSemaphore = context->CreateSemaphore();
    }

    VkSemaphore IFrameScheduler::CreateTempSemaphore()
    {
        if (m_tmpSemaphoreHead < m_tmpSemaphores.size())
        {
            return m_tmpSemaphores[m_tmpSemaphoreHead++];
        }

        m_tmpSemaphoreHead++;
        return m_tmpSemaphores.emplace_back(((IContext*)m_context)->CreateSemaphore());
    }

    void IFrameScheduler::ExecuteCommandLists(CommandList& commandList, Fence* _fence)
    {
        auto fence = (IFence*)_fence;
        // VkSemaphoreSubmitInfo semaphoreSubmitInfo{};
        // semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        // semaphoreSubmitInfo.pNext = nullptr;
        // semaphoreSubmitInfo.semaphore = ;
        // semaphoreSubmitInfo.value;
        // semaphoreSubmitInfo.stageMask;
        // semaphoreSubmitInfo.deviceIndex;

        VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
        commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandBufferSubmitInfo.pNext = nullptr;
        commandBufferSubmitInfo.commandBuffer = ((ICommandList&)commandList).m_commandBuffer;

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.pNext = nullptr;
        submitInfo.flags = 0;
        submitInfo.waitSemaphoreInfoCount = 0;
        submitInfo.pWaitSemaphoreInfos = nullptr;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;
        submitInfo.signalSemaphoreInfoCount = 0;
        // submitInfo.pSignalSemaphoreInfos = &semaphoreSubmitInfo;
        vkQueueSubmit2(m_graphicsQueue, 1, &submitInfo, fence->UseFence());
    }

} // namespace RHI::Vulkan