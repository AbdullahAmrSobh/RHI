#pragma once
#include "RHI/Common.hpp"

#include "RHI/Debug.hpp"
#include "RHI/Format.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Resource.hpp"
#include "RHI/Image.hpp"
#include "RHI/Buffer.hpp"

/////////////////////////////////////////////////
#include "Backend/Vulkan/Vma/vk_mem_alloc.hpp"
/////////////////////////////////////////////////
#include "Backend/Vulkan/DeviceObject.hpp"

namespace RHI
{
namespace Vulkan
{

namespace Utils
{

inline void AssertSuccess(VkResult result)
{
    assert(result == VK_SUCCESS);
}

inline bool IsSuccess(VkResult result)
{
    return result >= 0;
}

inline bool IsError(VkResult result)
{
    return !(IsSuccess(result));
}

}  // namespace Utils
}  // namespace Vulkan
}  // namespace RHI

#define VK_RETURN_ON_ERROR(result)                                                                                                         \
    if (::RHI::Vulkan::Utils::IsError(result))                                                                                             \
    {                                                                                                                                      \
        return result;                                                                                                                     \
    }