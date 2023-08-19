#pragma once
#include "RHI/Flags.hpp"
#include "RHI/Object.hpp"

namespace RHI
{

class Context;

/// @brief Synchronization object, used by to wait on GPU operation.
class Fence : public Object
{
public:
    using Object::Object;
    virtual ~Fence() = default;

    /// @brief Blocks until timeout is reached.
    // virtual ResultCode Wait(uint64_t timeouts) const = 0;

    /// @brief Returns the current state of the fence.
    // virtual FenceState GetState() const = 0;

protected:
    /// @brief Resets the fence to its unsignaled state
    // virtual ResultCode Reset() = 0;
};

}  // namespace RHI
