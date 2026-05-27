#pragma once

#include <TL/Compiler.hpp>

namespace RHI
{
    // TODO: Break this down to more granular enums
    // e.g. memory allocation results, sync result, swapchain results
    enum class TL_NODISCARD ResultCode
    {
        Success,                ///< Operation was successful.
        SuccessSuboptimal,      ///< Operation succeeded but not in optimal conditions.
        ErrorUnknown,           ///< An unknown error occurred.
        ErrorOutOfMemory,       ///< A general out-of-memory error.
        ErrorDeviceOutOfMemory, ///< The device ran out of memory.
        ErrorAllocationFailed,  ///< Memory allocation failed.
        ErrorPoolOutOfMemory,   ///< The pool ran out of memory.
        ErrorDeviceRemoved,     ///< The device was removed.
        ErrorInvalidArguments,  ///< Invalid arguments were provided.
        ErrorTimeout,           ///< Operation timed out.
        ErrorOutdated,          ///< Resource or data is outdated.
        ErrorSurfaceLost,       ///< Surface is no longer available.
        ErrorDeviceLost,        ///< Device is no longer available.
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
