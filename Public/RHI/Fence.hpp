#pragma once
#include "RHI/Common.hpp"
#include "RHI/Definitions.hpp"

namespace RHI
{

class IFence
{
public:
    virtual ~IFence() = default;

    virtual EResultCode Wait() const      = 0;
    virtual EResultCode Reset() const     = 0;
    virtual EResultCode GetStatus() const = 0;
};

using FencePtr = Unique<IFence>;

} // namespace RHI
