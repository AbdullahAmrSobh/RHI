#pragma once

#include "RHI/Export.hpp"

#include <stacktrace>
#include <memory>
#include <format>

#define RHI_LOG_INFO(msg, ...)    Log(MessageLevel::Log, msg, __VA_ARGS__)
#define RHI_LOG_WARNING(msg, ...) Log(MessageLevel::Warnning, msg, __VA_ARGS__)
#define RHI_LOG_ERROR(msg, ...)   Log(MessageLevel::Error, msg, __VA_ARGS__)

namespace RHI::Debug
{

    struct Stacktrace {};

    std::unique_ptr<Stacktrace> GetCurrentCallstack();

    enum class MessageLevel 
    {
        Log,
        Warnning,
        Error,
    };

    void log(MessageLevel level, std::string message);

    template<typename ...Args>
    inline static void Log(MessageLevel level, std::string_view message, Args ... args)
    {
        auto formattedMessage = std::format(message, std::forward(args) ...);
        log(level, formattedMessage);
    }

}