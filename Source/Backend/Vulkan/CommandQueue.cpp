#include "Backend/Vulkan/CommandQueue.hpp"

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

void CommandQueue::Submit(std::span<const SubmitRequest*> submitList,
                          IFence*                         signalFence)
{
    struct SubmitInfoData
    {
        std::vector<VkSemaphore>          waitSemaphores;
        std::vector<VkPipelineStageFlags> waitPoints;
        std::vector<VkCommandBuffer>      commandBuffers;
        std::vector<VkSemaphore>          signalSemaphores;
    };

    std::vector<SubmitInfoData> submitInfosData;
    submitInfosData.reserve(submitList.size());

    std::vector<VkSubmitInfo> submitInfos;
    submitInfos.reserve(submitList.size());

    for (auto submitRequest : submitList)
    {
        SubmitInfoData& data = submitInfosData.emplace_back(SubmitInfoData());

        for (auto waitPoint : submitRequest->waitPoints)
        {
            data.signalSemaphores.push_back(waitPoint.semaphore->GetHandle());
            data.waitPoints.push_back(waitPoint.stage);
        }

        VkSubmitInfo submitInfo {};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext                = nullptr;
        submitInfo.waitSemaphoreCount   = CountElements(data.waitSemaphores);
        submitInfo.pWaitSemaphores      = data.waitSemaphores.data();
        submitInfo.pWaitDstStageMask    = data.waitPoints.data();
        submitInfo.commandBufferCount   = CountElements(data.commandBuffers);
        submitInfo.pCommandBuffers      = data.commandBuffers.data();
        submitInfo.signalSemaphoreCount = CountElements(data.signalSemaphores);
        submitInfo.pSignalSemaphores    = data.signalSemaphores.data();
        submitInfos.push_back(submitInfo);
    }

    Fence* fence = static_cast<Fence*>(signalFence);
    VkResult result = vkQueueSubmit(m_handle, CountElements(submitInfos), submitInfos.data(),  fence? fence->GetHandle() : nullptr);
    RHI_VK_ASSERT_SUCCESS(result);
}

VkResult CommandQueue::Present(std::span<const Semaphore*> waitSemaphores,
                               const Swapchain&            swapchain)
{
    std::vector<VkSemaphore> semaphores;
    for(auto waitSemaphore : waitSemaphores)
    {
        semaphores.push_back(waitSemaphore->GetHandle());
    }

    uint32_t currentImageIndex = swapchain.GetCurrentBackBufferIndex();

    VkSwapchainKHR swapchainHandle = swapchain.GetHandle();

    VkResult presentationResult = VK_ERROR_UNKNOWN;

    VkPresentInfoKHR presentInfo {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = nullptr;
    presentInfo.waitSemaphoreCount = CountElements(waitSemaphores);
    presentInfo.pWaitSemaphores    = semaphores.data();
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &swapchainHandle; 
    presentInfo.pImageIndices      = &currentImageIndex;
    presentInfo.pResults           = &presentationResult;

    VkResult result = vkQueuePresentKHR(m_handle, &presentInfo);
    RHI_VK_ASSERT_SUCCESS(result);

    return presentationResult;
}

}  // namespace Vulkan
}  // namespace RHI