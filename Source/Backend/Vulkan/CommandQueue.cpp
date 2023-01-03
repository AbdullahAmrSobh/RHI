#include "RHI/Pch.hpp"

#include "Backend/Vulkan/Common.hpp"

#include "Backend/Vulkan/CommandQueue.hpp"

#include "Backend/Vulkan/Commands.hpp"
#include "Backend/Vulkan/Resource.hpp"
#include "Backend/Vulkan/Swapchain.hpp"

namespace RHI
{
namespace Vulkan
{

void CommandQueue::WaitIdle()
{
    vkQueueWaitIdle(m_handle);
}

void CommandQueue::Submit(const SubmitInfo& submitInfo, IFence* signalFence)
{
    std::vector<VkSemaphore>          waitSemaphores;
    std::vector<VkPipelineStageFlags> waitPoints;
    std::vector<VkCommandBuffer>      commandBuffers;
    std::vector<VkSemaphore>          signalSemaphores;

    for (auto waitPoint : submitInfo.waitPoints)
    {
        waitSemaphores.push_back(waitPoint.semaphore->GetHandle());
        waitPoints.push_back(waitPoint.stage);
    }

    for (auto signalSemaphore : submitInfo.signalSemaphores)
    {
        signalSemaphores.push_back(signalSemaphore->GetHandle());
    }

    for (auto commandBuffer : submitInfo.commandBuffers)
    {
        commandBuffers.push_back(commandBuffer->GetHandle());
    }

    VkSubmitInfo vkSubmitInfo {};
    vkSubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.pNext                = nullptr;
    vkSubmitInfo.waitSemaphoreCount   = CountElements(waitSemaphores);
    vkSubmitInfo.pWaitSemaphores      = waitSemaphores.data();
    vkSubmitInfo.pWaitDstStageMask    = waitPoints.data();
    vkSubmitInfo.commandBufferCount   = CountElements(commandBuffers);
    vkSubmitInfo.pCommandBuffers      = commandBuffers.data();
    vkSubmitInfo.signalSemaphoreCount = CountElements(signalSemaphores);
    vkSubmitInfo.pSignalSemaphores    = signalSemaphores.data();

    Fence* fence = static_cast<Fence*>(signalFence);

    VkResult result = vkQueueSubmit(m_handle, 1, &vkSubmitInfo, fence ? fence->GetHandle() : nullptr);
    Utils::AssertSuccess(result);
}

void CommandQueue::Present(std::span<const Semaphore* const> waitSemaphores, Swapchain& swapchain)
{
    std::vector<VkSemaphore> semaphores;
    for (auto waitSemaphore : waitSemaphores)
    {
        semaphores.push_back(waitSemaphore->GetHandle());
    }

    uint32_t currentImageIndex = swapchain.GetCurrentImageIndex();

    VkSwapchainKHR swapchainHandle = swapchain.GetHandle();

    VkPresentInfoKHR presentInfo {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = nullptr;
    presentInfo.waitSemaphoreCount = CountElements(waitSemaphores);
    presentInfo.pWaitSemaphores    = semaphores.data();
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &swapchainHandle;
    presentInfo.pImageIndices      = &currentImageIndex;
    presentInfo.pResults           = nullptr;

    VkResult result = vkQueuePresentKHR(m_handle, &presentInfo);

    swapchain.SwapImages();
    Utils::AssertSuccess(result);
}

}  // namespace Vulkan
}  // namespace RHI