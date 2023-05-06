#pragma once

namespace RHI
{

enum class ResultCode
{
    Success,
    ErrorOutOfMemory, 
    ErrorOutOfDeviceMemory, 
    ErrorInvalidOperation,
    ErrorInvalidArgument, 
};

template<typename T>
struct Result
{
    ResultCode result;
    T          object;
};

}  // namespace RHI