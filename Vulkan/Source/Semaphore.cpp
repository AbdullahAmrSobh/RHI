#include "Common.hpp"
#include "Device.hpp"
#include "Semaphore.hpp"

namespace RHI::Vulkan
{
    ResultCode ISemaphore::Init(IDevice* device, const SemaphoreCreateInfo& _createInfo)
    {
        VkSemaphoreTypeCreateInfo semaphoreType{
            .sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext         = nullptr,
            .semaphoreType = _createInfo.timeline ? VK_SEMAPHORE_TYPE_TIMELINE : VK_SEMAPHORE_TYPE_BINARY,
            .initialValue  = 0,
        };
        VkSemaphoreCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &semaphoreType,
            .flags = 0,
        };

        this->info = _createInfo;

        auto result = vkCreateSemaphore(device->m_device, &createInfo, nullptr, &this->handle);
        if (_createInfo.name != nullptr)
        {
            device->SetDebugName(this->handle, _createInfo.name);
        }
        return ConvertResult(result);
    }

    void ISemaphore::Shutdown(IDevice* device)
    {
        // vkDestroySemaphore(device->m_device, this->handle, nullptr);
        device->m_deleteQueue.DestroyObject(handle);
    }
} // namespace RHI::Vulkan