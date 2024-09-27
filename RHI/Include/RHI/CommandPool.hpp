#pragma once

#include "RHI/Export.hpp"
#include "RHI/Queue.hpp"

#include <TL/Assert.hpp>
#include <TL/Containers.hpp>
#include <TL/UniquePtr.hpp>

namespace RHI
{
    class CommandList;

    /// @brief Flags for command pool configuration.
    enum class CommandPoolFlags
    {
        None      = 0,    ///< No flags set.
        Transient = 0x01, ///< The command pool is transient.
        Reset     = 0x02, ///< The command pool supports resetting.
    };

    /// @brief Specifies the level of a command list.
    enum class CommandListLevel
    {
        Primary,   ///< Primary command list, which can be executed directly.
        Secondary, ///< Secondary command list, which can be executed within a primary command list.
    };

    /// @brief Represents a pool of command lists.
    /// @note command lists allocated from a pool are only freed on calling Reset
    class RHI_EXPORT CommandPool
    {
    public:
        CommandPool() = default;

        CommandPool(const CommandPool&) = delete;

        CommandPool(CommandPool&&) = delete;

        virtual ~CommandPool();

        /// @brief Resets the command pool, clearing any allocated command lists.
        virtual void                                          Reset() = 0;

        /// @brief Allocates a command list from the pool.
        /// @param queueType Type of queue to allocate for.
        /// @param level Level of the command list to allocate.
        /// @return A pointer to the allocated command list.
        TL_NODISCARD TL::Ptr<CommandList>                     Allocate(QueueType queueType, CommandListLevel level);

        /// @brief Allocates multiple command lists from the pool.
        /// @param queueType Type of queue to allocate for.
        /// @param level Level of the command lists to allocate.
        /// @param count Number of command lists to allocate.
        /// @return A vector of pointers to the allocated command lists.
        TL_NODISCARD virtual TL::Vector<TL::Ptr<CommandList>> Allocate(QueueType queueType, CommandListLevel level, uint32_t count) = 0;
    };

}; // namespace RHI