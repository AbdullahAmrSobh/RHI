#pragma once

#include "RHI/Export.hpp"
#include <cstdint>

namespace RHI
{
    /// @brief Represents the state of a fence.
    enum class FenceState
    {
        NotSubmitted, ///< The fence has not been submitted for processing.
        Pending,      ///< The fence is pending and has not been signaled yet.
        Signaled,     ///< The fence has been signaled, indicating completion.
    };

    /// @brief Represents a synchronization primitive used to track GPU execution.
    class RHI_EXPORT Fence
    {
    public:
        /// @brief Default constructor.
        Fence();

        /// @brief Virtual destructor.
        virtual ~Fence();

        /// @brief Waits for the fence to be signaled.
        /// @param timeout The timeout duration in nanoseconds (default: UINT64_MAX, meaning wait indefinitely).
        /// @return true if the fence was signaled, false if the wait timed out.
        bool               Wait(uint64_t timeout = UINT64_MAX);

        /// @brief Resets the fence to its initial state.
        virtual void       Reset() = 0;

        /// @brief Gets the current state of the fence.
        /// @return The current state of the fence as a `FenceState`.
        virtual FenceState GetState() = 0;

    protected:
        /// @brief Waits for the fence using a platform-specific implementation.
        /// @param timeout The timeout duration in nanoseconds.
        /// @return true if the fence was signaled, false if the wait timed out.
        virtual bool WaitInternal(uint64_t timeout) = 0;
    };

    inline Fence::Fence() = default;

    inline Fence::~Fence() = default;

    inline bool Fence::Wait(uint64_t timeout)
    {
        return WaitInternal(timeout);
    }
} // namespace RHI
