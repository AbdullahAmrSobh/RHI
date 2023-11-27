#pragma once
#include <RHI/Common/Result.hpp>
#include <string>
#include <vk_mem_alloc.h>

#define VULKAN_LOAD_PROC(device, proc) reinterpret_cast<PFN_##proc>(vkGetDeviceProcAddr(device, #proc));

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

    template<typename T>
    inline static bool IsPow2(T x)
    {
        return (x & (x - 1)) == 0;
    }

    template<typename T>
    inline static T AlignUp(T val, T alignment)
    {
        RHI_ASSERT(IsPow2(alignment));
        return (val + alignment - 1) & ~(alignment - 1);
    }

    template<typename T>
    inline static uint64_t HashAny(const T& data)
    {
        auto                   stream = std::string(reinterpret_cast<const char*>(&data), sizeof(data));
        std::hash<std::string> hasher;
        return hasher(stream);
    }

    inline static uint64_t HashCombine(uint64_t seed, uint64_t value)
    {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }

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