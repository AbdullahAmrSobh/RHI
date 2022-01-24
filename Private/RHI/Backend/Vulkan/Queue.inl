#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/Fence.hpp"
#include "RHI/Backend/Vulkan/SwapChain.hpp"

namespace RHI
{
namespace Vulkan
{

    class Queue : public DeviceObject<VkQueue>
    {
    public:
        inline Queue(Device& device, uint32_t queueFamilyIndex, uint32_t queueIndex)
            : DeviceObject(device)
        {
            VkDeviceQueueInfo2 queueInfo;
            queueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
            queueInfo.pNext            = nullptr;
            queueInfo.flags            = 0;
            queueInfo.queueFamilyIndex = queueFamilyIndex;
            queueInfo.queueIndex       = queueIndex;

            vkGetDeviceQueue2(m_pDevice->GetHandle(), &queueInfo, &m_handle);
        }
        ~Queue() = default;

        inline bool QueuePresentSupport(const Surface& surface)
        {
            // check if the queue supports presentation to the swap chain's surface
            VkBool32 supported = false;
            VkResult result =
                vkGetPhysicalDeviceSurfaceSupportKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_queueFamilyIndex, surface.GetHandle(), &supported);
            Assert(result);
            return supported == VK_TRUE;
        }
    
    private:
        uint32_t m_queueFamilyIndex, m_queueIndex;
    };
} // namespace Vulkan
} // namespace RHI
