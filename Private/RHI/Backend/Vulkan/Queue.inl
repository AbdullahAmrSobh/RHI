#pragma once
#include "RHI/Backend/Vulkan/CommandList.hpp"
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/SwapChain.hpp"
#include "RHI/Backend/Vulkan/Fence.hpp"

namespace RHI
{
namespace Vulkan
{
    class Queue : public DeviceObject<VkQueue>
    {
    public:
        Queue(Device& _device, uint32_t queueFamilyIndex, uint32_t queueIndex = 0)
            : DeviceObject(_device)
            , m_queueFamilyIndex(queueFamilyIndex)
            , m_queueIndex(queueIndex)
        {
            vkGetDeviceQueue(m_pDevice->GetHandle(), m_queueFamilyIndex, m_queueIndex, &m_handle);
        }

        VkResult WaitIdle() { return vkQueueWaitIdle(m_handle); }

        VkResult Submit(uint32_t count, CommandList* pCmdLists, Fence& fence)
        {
            VkSubmitInfo* submitInfo = new VkSubmitInfo[count];
            return vkQueueSubmit(m_handle, count, submitInfo, fence.GetHandle());
        }
    
    public:
        const uint32_t m_queueFamilyIndex;
        const uint32_t m_queueIndex;
    };

    class PresentQueue final : public Queue
    {
    public:
        PresentQueue(Device& _device, uint32_t queueFamilyIndex, uint32_t queueIndex = 0)
            : Queue(_device, queueFamilyIndex, queueIndex)
        {
        }
        ~PresentQueue();

        VkResult CheckSurfaceSupport(Surface& surface)
        {
            VkBool32 surfaceSupport = VK_FALSE;
            VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_queueFamilyIndex, surface.GetHandle(), &surfaceSupport);
        
			if (result == VK_SUCCESS && surfaceSupport == VK_TRUE)
				return result;
			else 
				return VK_ERROR_SURFACE_LOST_KHR;
		}
        
        VkResult Present(uint32_t swapChainsCount, SwapChain* swapcainCount)
        {
            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.pNext            = nullptr;
            presentInfo.waitSemaphoreCount;
            presentInfo.pWaitSemaphores;
            presentInfo.swapchainCount;
            presentInfo.pSwapchains;
            presentInfo.pImageIndices;
            presentInfo.pResults;

            return vkQueuePresentKHR(m_handle, &presentInfo);
        }
    };

} // namespace Vulkan
} // namespace RHI
