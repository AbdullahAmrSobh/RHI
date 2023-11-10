#pragma once
#include <RHI/Result.hpp>
#include <vk_mem_alloc.h>

#define VULKAN_ASSERT_SUCCESS(result) RHI_ASSERT(result == VK_SUCCESS)

#define VULKAN_RETURN_VKERR_CODE(result) \
    if (result != VK_SUCCESS)            \
    {                                    \
        return result;                   \
    }

#define VULKAN_RETURN_ERR_CODE(result) \
    if (result != VK_SUCCESS)          \
    {                                  \
        return ConvertResult(result);  \
    }

namespace Vulkan
{
    inline static RHI::ResultCode ConvertResult(VkResult result)
    {
        switch (result)
        {
        case VK_SUCCESS:
            return RHI::ResultCode::Success;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return RHI::ResultCode::ErrorOutOfMemory;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return RHI::ResultCode::ErrorDeviceOutOfMemory;
        default:
            return RHI::ResultCode::ErrorUnkown;
        }
    }
} // namespace Vulkan