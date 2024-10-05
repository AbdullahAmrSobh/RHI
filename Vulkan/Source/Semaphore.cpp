#include "Common.hpp"
#include "Context.hpp"
#include "Semaphore.hpp"

namespace RHI::Vulkan
{
    ResultCode ISemaphore::Init(IContext* context, const SemaphoreCreateInfo& _createInfo)
    {
        VkSemaphoreTypeCreateInfo semaphoreType{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = nullptr,
            .semaphoreType = _createInfo.timeline ? VK_SEMAPHORE_TYPE_TIMELINE : VK_SEMAPHORE_TYPE_BINARY,
            .initialValue = 0,
        };
        VkSemaphoreCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &semaphoreType,
            .flags = 0,
        };

        this->info = _createInfo;

        auto result = vkCreateSemaphore(context->m_device, &createInfo, nullptr, &this->handle);
        return ConvertResult(result);
    }

    void ISemaphore::Shutdown(IContext* context)
    {
        vkDestroySemaphore(context->m_device, this->handle, nullptr);
    }
} // namespace RHI::Vulkan