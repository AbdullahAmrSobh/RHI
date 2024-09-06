#pragma once

#include <TL/Assert.hpp>

namespace RHI
{
    /// @brief Represents the result codes for RHI operations.
    enum class TL_NODISCARD ResultCode
    {
        Success,                ///< Operation was successful.
        ErrorUnknown,           ///< An unknown error occurred.
        ErrorOutOfMemory,       ///< A general out-of-memory error.
        ErrorDeviceOutOfMemory, ///< The device ran out of memory.
        ErrorAllocationFailed,  ///< Memory allocation failed.
    };

    /// @brief Checks if the result code indicates success.
    /// @param result The result code to check.
    /// @return true if the result is `ResultCode::Success`, false otherwise.
    inline static bool IsSuccess(ResultCode result)
    {
        return result == ResultCode::Success;
    }

    /// @brief Checks if the result code indicates an error.
    /// @param result The result code to check.
    /// @return true if the result is not `ResultCode::Success`, false otherwise.
    inline static bool IsError(ResultCode result)
    {
        return result != ResultCode::Success;
    }

    /// @brief A templated structure that holds a result value and a corresponding result code.
    /// @tparam T The type of the value.
    template<typename T>
    struct TL_NODISCARD Result
    {
        /// @brief Constructs a result with a result code and a default value.
        /// @param code The result code.
        Result(ResultCode code)
            : value()
            , result(code)
        {
        }

        /// @brief Constructs a result with a value and assumes success.
        /// @param t The value to store.
        Result(T t)
            : value(t)
            , result(RHI::ResultCode::Success)
        {
        }

        /// @brief Constructs a result with both a value and a result code.
        /// @param t The value to store.
        /// @param code The result code.
        Result(T t, ResultCode code)
            : value(t)
            , result(code)
        {
        }

        T           value;  ///< The result value.
        ResultCode  result; ///< The result code.

        /// @brief Checks if the result indicates success.
        /// @return true if the result is `ResultCode::Success`, false otherwise.
        inline bool IsSuccess() const { return ::RHI::IsSuccess(result); }

        /// @brief Checks if the result indicates an error.
        /// @return true if the result is not `ResultCode::Success`, false otherwise.
        inline bool IsError() const { return ::RHI::IsError(result); }

        /// @brief Retrieves the result value, asserting that the result was successful.
        /// @return The result value.
        inline T    GetValue()
        {
            TL_ASSERT(result == ResultCode::Success);
            return std::move(value);
        }
    };
} // namespace RHI
