#include "RHI/Backend/Vulkan/Queue.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

#include "RHI/Backend/Vulkan/Fence.hpp"

namespace RHI
{
namespace Vulkan
{
    Queue::Queue(Device& device, const QueueDesc& queueDesc)
        : DeviceObject(device)
        , m_queueFamilyIndex(queueDesc.queueFamilyIndex)
        , m_queueIndex(queueDesc.queueIndex)
    {
        vkGetDeviceQueue(m_pDevice->GetHandle(), m_queueFamilyIndex, m_queueIndex, &m_handle);
    }

    VkResult Queue::Submit(const std::vector<SubmitInfo>& submitInfos, IFence& signalFence)
    {
        Fence&                    vkSignalFence = static_cast<Fence&>(signalFence);
        std::vector<VkSubmitInfo> vkSubmitInfos(CountElements(submitInfos));
        for (auto& submitInfo : submitInfos)
        {
            VkSubmitInfo vkSubmitInfo         = {};
            vkSubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            vkSubmitInfo.pNext                = nullptr;
            vkSubmitInfo.commandBufferCount   = CountElements(submitInfo.commandBuffers);
            vkSubmitInfo.pCommandBuffers      = submitInfo.commandBuffers.data();
            vkSubmitInfo.signalSemaphoreCount = CountElements(submitInfo.signalSemaphores);
            vkSubmitInfo.pSignalSemaphores    = submitInfo.signalSemaphores.data();
            vkSubmitInfo.waitSemaphoreCount   = CountElements(submitInfo.waitSemaphores);
            vkSubmitInfo.pWaitSemaphores      = submitInfo.waitSemaphores.data();
            vkSubmitInfo.pWaitDstStageMask    = submitInfo.waitStages.data();
            vkSubmitInfos.push_back(vkSubmitInfo);
        }
        return vkQueueSubmit(m_handle, CountElements(vkSubmitInfos), vkSubmitInfos.data(), vkSignalFence.GetHandle());
    }
    
    VkResult Queue::Present(const PresentInfo& desc, std::vector<VkResult>& outPresentResults)
    {
        VkPresentInfoKHR presentInfo   = {};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext              = nullptr;
        presentInfo.waitSemaphoreCount = CountElements(desc.waitSemaphores);
        presentInfo.pWaitSemaphores    = desc.waitSemaphores.data();
        presentInfo.swapchainCount     = CountElements(desc.swapchainHandles);
        presentInfo.pSwapchains        = desc.swapchainHandles.data();
        presentInfo.pImageIndices      = desc.imageIndices.data();
        presentInfo.pResults           = outPresentResults.data();
        return vkQueuePresentKHR(m_handle, &presentInfo);
    }

} // namespace Vulkan
} // namespace RHI
