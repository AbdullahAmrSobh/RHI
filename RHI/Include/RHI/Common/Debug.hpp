#pragma once

#include <string_view>

namespace RHI
{
    class DebugCallbacks
    {
    public:
        virtual ~DebugCallbacks() = default;

        /// @brief Log an information.
        virtual void LogInfo(std::string_view message) = 0;

        /// @brief Log an warnning.
        virtual void LogWarnning(std::string_view message) = 0;

        /// @brief Log an error.
        virtual void LogError(std::string_view message) = 0;
    };
} // namespace RHI