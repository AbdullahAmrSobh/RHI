#pragma once
#include "RHI/Definitions.hpp"
#include <cassert>
#include <vulkan/vulkan.h>

#include "RHI/Core/Expected.hpp"

namespace RHI
{

namespace Vulkan
{

    template <class T>
    using VkExpected = tl::expected<T, VkResult>;
    
    inline EResultCode ToResultCode(VkResult result)
    {
        switch (result)
        {
        case VK_SUCCESS: return EResultCode::Success;
        case VK_NOT_READY: return EResultCode::NotReady;
        case VK_TIMEOUT: return EResultCode::Timeout;
        case VK_EVENT_SET:
        case VK_EVENT_RESET:
        case VK_INCOMPLETE:
        case VK_ERROR_OUT_OF_HOST_MEMORY: return EResultCode::OutOfMemory;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return EResultCode::DeviceOutOfMemory;
        case VK_ERROR_INITIALIZATION_FAILED:
        case VK_ERROR_DEVICE_LOST:
        case VK_ERROR_MEMORY_MAP_FAILED:
        case VK_ERROR_LAYER_NOT_PRESENT: return EResultCode::LayerNotPresent;
        case VK_ERROR_EXTENSION_NOT_PRESENT: return EResultCode::ExtensionNotPresent;
        case VK_ERROR_FEATURE_NOT_PRESENT:
        case VK_ERROR_INCOMPATIBLE_DRIVER:
        case VK_ERROR_TOO_MANY_OBJECTS:
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
        case VK_ERROR_FRAGMENTED_POOL:
        case VK_ERROR_UNKNOWN:
        case VK_ERROR_OUT_OF_POOL_MEMORY:
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        case VK_ERROR_FRAGMENTATION:
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
        case VK_ERROR_SURFACE_LOST_KHR:
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        case VK_SUBOPTIMAL_KHR:
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        case VK_ERROR_VALIDATION_FAILED_EXT:
        case VK_ERROR_INVALID_SHADER_NV:
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
        case VK_ERROR_NOT_PERMITTED_EXT:
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        case VK_THREAD_IDLE_KHR:
        case VK_THREAD_DONE_KHR:
        case VK_OPERATION_DEFERRED_KHR:
        case VK_OPERATION_NOT_DEFERRED_KHR:
        case VK_PIPELINE_COMPILE_REQUIRED_EXT:
        // case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
        // case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR:
        // case VK_ERROR_FRAGMENTATION_EXT:
        // case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
        // case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR:
        // case VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT:
        case VK_RESULT_MAX_ENUM: return EResultCode::Fail;
        }
        return EResultCode::Fail;
    }
    
    inline void Assert(VkResult result) { assert(result == VK_SUCCESS); }

#define RHI_VK_RETURN_ON_FAIL(result) \
if (result != VK_SUCCESS)             \
{                                     \
	return result;                    \
}

} // namespace Vulkan

} // namespace RHI
