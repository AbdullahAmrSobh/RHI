#pragma once

#include <TL/Compiler.hpp>

namespace RHI
{
    // TODO: Break this down to more granular enums
    // e.g. memory allocation results, sync result, swapchain results
    enum class TL_NODISCARD ResultCode
    {
        Success,
        SuccessSuboptimal,
        ErrorUnknown,
        ErrorOutOfMemory,
        ErrorDeviceOutOfMemory,
        ErrorAllocationFailed,
        ErrorPoolOutOfMemory,
        ErrorDeviceRemoved,
        ErrorInvalidArguments,
        ErrorTimeout,
        ErrorOutdated,
        ErrorSurfaceLost,
        ErrorDeviceLost,
    };

    inline static bool IsSuccess(ResultCode result)
    {
        return result == ResultCode::Success || result == ResultCode::SuccessSuboptimal;
    }

    inline static bool IsError(ResultCode result)
    {
        return !IsSuccess(result);
    }
} // namespace RHI
