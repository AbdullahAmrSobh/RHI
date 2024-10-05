#include "RHI/Fence.hpp"

namespace RHI
{
    Fence::Fence() = default;

    Fence::~Fence() = default;

    bool Fence::Wait(uint64_t timeout)
    {
        return WaitInternal(timeout);
    }
} // namespace RHI