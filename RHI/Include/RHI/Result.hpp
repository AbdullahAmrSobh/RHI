#pragma once

namespace RHI
{

enum class ResultCode
{
    Success,
    Error,
    ErrorOutOfMemory,
    ErrorOutOfDeviceMemory,
    ErrorInvalidOperation,
    ErrorInvalidArgument,
};

template<typename T>
struct Result
{
    T value;

    ResultCode result;
};

}  // namespace RHI