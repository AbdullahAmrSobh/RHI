#pragma once

namespace RHI
{

/// @brief Indicate the result of the operation.
enum class ResultCode
{
    /// @brief The operation succeeded without any issues.
    Success,

    /// @brief The operation failed for an unkowen reason.
    ErrorUnkowen,

    /// @brief The resource creation/allocation failed due to lack of memory in allocator.
    ErrorOutOfMemory,

    /// @brief The resource creation/allocation failed due to lack of device (GPU) memory.
    ErrorDeviceOutOfMemory,

    /// @brief The resource allocation failed.
    ErrorAllocationFailed,

    /// @brief The frame graph cycle dependency.
    ErrorCyclicFrameGraph,

    /// @brief Attempted to access a resource after it was deleted.
    ErrorResourceUsedAfterFree,
};

/// @brief Encapsulate an value and a result code for error handling.
/// @tparam T The type of the value created if the result is a success.
template<typename T>
struct Result
{
    /// @brief The actual value (only valid if result is a success value).
    T value;

    /// @brief The result of the operation.
    ResultCode result;
};

/// @brief Return true if the value is a success value.
inline static bool IsSucess(ResultCode result)
{
    return result == ResultCode::Success;
}

/// @brief Return true if the value is a failure value.
inline static bool IsError(ResultCode result)
{
    return result != ResultCode::Success;
}

}  // namespace RHI