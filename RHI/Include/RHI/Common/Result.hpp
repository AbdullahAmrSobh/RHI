#pragma once

#include "RHI/Common/Assert.hpp"

#include <utility>

namespace RHI
{
    enum class RHI_NODISCARD ResultCode
    {
        Success,
        SwapchainOutOfDate,
        SwapchainSurfaceLost,
        ErrorUnknown,
        ErrorOutOfMemory,
        ErrorDeviceOutOfMemory,
        ErrorAllocationFailed,
    };

    RHI_NODISCARD inline static bool IsSucess(ResultCode result)
    {
        return result == ResultCode::Success;
    }

    RHI_NODISCARD inline static bool IsError(ResultCode result)
    {
        return result != ResultCode::Success;
    }

    template<typename T>
    struct RHI_NODISCARD Result
    {
        Result(ResultCode code)
            : value()
            , result(code)
        {
        }

        Result(T t)
            : value(t)
            , result(RHI::ResultCode::Success)
        {
        }

        Result(T t, ResultCode code)
            : value(t)
            , result(code)
        {
        }

        T value;

        ResultCode result;

        inline bool IsSucess() const { return ::RHI::IsSucess(result); }

        inline bool IsError() const { return ::RHI::IsError(result); }

        inline T GetValue()
        {
            RHI_ASSERT(result == ResultCode::Success);
            return std::move(value);
        }
    };
} // namespace RHI