#pragma once

#include "RHI/RHI.hpp"

template<typename T> using Handle = RHI::Handle<T>;

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
        TL_ASSERT(result == ResultCode::Success);
        return std::move(value);
    }
};
