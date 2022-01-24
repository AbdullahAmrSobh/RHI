#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

class IDebugMessenger
{
public:
    virtual ~IDebugMessenger() = default;
    
    virtual void Info(std::string_view message)  = 0;
    virtual void Warn(std::string_view message)  = 0;
    virtual void Error(std::string_view message) = 0;
    virtual void Fatel(std::string_view message) = 0;
};

} // namespace RHI
