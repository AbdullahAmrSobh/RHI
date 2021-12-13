#include "RHI/Backend/Vulkan/Context.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"
#include "RHI/Backend/Vulkan/Resources.hpp"

namespace RHI
{
namespace Vulkan
{
    Context::Context(Device& device)
        : m_pDevice(&device)
    {
        uint32_t familyIndex = m_pDevice->m_queueSettings.presentQueueIndex;
        uint32_t index       = 0;
        vkGetDeviceQueue(m_pDevice->GetHandle(), familyIndex, index, &m_PresentQueue);
    }

    EResultCode Context::Present(uint32_t _swapchainCount, ISwapChain** _ppSwapchains)
    {
        std::vector<VkSemaphore> waitSemaphores;
        
        std::vector<VkSwapchainKHR> swapchains(_swapchainCount);
        std::vector<uint32_t>       imageIndices(_swapchainCount);
        std::vector<VkResult>       results(_swapchainCount);
        
        for (uint32_t i = 0; i < _swapchainCount; i++)
        {
            auto swapchain  = static_cast<SwapChain*>(_ppSwapchains[i]);
            swapchains[i]   = swapchain->GetHandle();
            imageIndices[i] = swapchain->GetCurrentBackBufferIndex();
            
            waitSemaphores.push_back(swapchain->m_ImageAvailableSemaphores[swapchain->GetCurrentBackBufferIndex()]);
        }
        
        VkPresentInfoKHR presentInfo   = {};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext              = nullptr;
        presentInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
        presentInfo.pWaitSemaphores    = waitSemaphores.data();
        presentInfo.swapchainCount     = static_cast<uint32_t>(swapchains.size());
        presentInfo.pSwapchains        = swapchains.data();
        presentInfo.pImageIndices      = imageIndices.data();
        presentInfo.pResults           = results.data();
        
        VkResult result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
        
        for (auto& res : results)
            if (result && res != VK_SUCCESS)
                return ToResultCode(res);

        return ToResultCode(result);
    }

} // namespace Vulkan
} // namespace RHI
