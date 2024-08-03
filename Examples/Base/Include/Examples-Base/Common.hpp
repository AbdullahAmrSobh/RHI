#pragma once

#include "RHI/RHI.hpp"

namespace TL2 = RHI::TL;

template<typename T> using Handle = RHI::Handle<T>;
template<typename T> using Ptr    = RHI::Ptr<T>;
template<typename T> using Flags  = RHI::Flags<T>;

enum class ResultCode
{
    Success,
    Error,
};

inline static bool IsSucess(ResultCode result)
{
    return result == ResultCode::Success;
}

inline static bool IsError(ResultCode result)
{
    return result != ResultCode::Success;
}

template<typename T>
struct [[nodiscard]] Result
{
    Result(ResultCode code)
        : value()
        , result(code)
    {
    }

    Result(T t)
        : value(t)
        , result(ResultCode::Success)
    {
    }

    Result(T t, ResultCode code)
        : value(t)
        , result(code)
    {
    }

    T value;

    ResultCode result;

    inline bool IsSucess() const { return ::IsSucess(result); }

    inline bool IsError() const { return ::IsError(result); }

    inline T GetValue()
    {
        RHI_ASSERT(result == ResultCode::Success);
        return std::move(value);
    }
};
