#pragma once
#include "RHI/ResultCode.hpp"

namespace RHI
{
template<typename _Ty>
struct Result 
{
    _Ty value; 
    ResultCode result; 

    Result(_Ty val)
        : value(val)
        , result(ResultCode::Success)
    {
    }

    Result(ResultCode resultCode)
        : result(resultCode)
    {
    }

    bool IsSuccess() const 
    {
        return result == ResultCode::Success;
    }
    
};

} // namespace RHI
