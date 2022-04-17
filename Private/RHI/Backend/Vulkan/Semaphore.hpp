#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Definitions.hpp"

namespace RHI
{
namespace Vulkan
{
    class Semaphore : public DeviceObject<VkSemaphore>
    {
    public:
        inline Semaphore(Device& device, VkSemaphore semaphore = VK_NULL_HANDLE)
            : DeviceObject(device, semaphore)
        {
        }
        
        inline ~Semaphore()
        {
            vkDestroySemaphore(m_pDevice->GetHandle(), m_handle, nullptr);
        }

        inline VkResult Init()
        {
            VkSemaphoreCreateInfo info = {};
            info.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            info.pNext                 = nullptr;
            info.flags                 = 0;
            return vkCreateSemaphore(m_pDevice->GetHandle(), &info, nullptr, &m_handle);
        }
		
    };

} // namespace Vulkan
} // namespace RHI
