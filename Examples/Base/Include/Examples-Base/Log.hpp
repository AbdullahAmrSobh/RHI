#pragma once

#include <format>
#include <iostream>

namespace Core
{
    template<typename... Args>
    inline static void LogInfo(const char* fmt, Args... args)
    {
        std::cout << "[INFO] " << std::vformat(fmt, std::make_format_args(args)...) << "\n";
    }

    inline static void LogInfo(const char* message)
    {
        std::cout << "[INFO] " << message << "\n";
    }

    inline static void LogInfo(std::string_view message)
    {
        std::cout << "[INFO] " << message << "\n";
    }

    template<typename... Args>
    inline static void LogWarnning(const char* fmt, Args... args)
    {
        std::cout << "[WARNNING] " << std::vformat(fmt, std::make_format_args(args)...) << "\n";
    }

    inline static void LogWarnning(const char* message)
    {
        std::cout << "[WARNNING] " << message << "\n";
    }

    inline static void LogWarnning(std::string_view message)
    {
        std::cout << "[WARNNING] " << message << "\n";
    }

    template<typename... Args>
    inline static void LogError(const char* fmt, Args... args)
    {
        std::cout << "[ERROR] " << std::vformat(fmt, std::make_format_args(args)...) << "\n";
    }

    inline static void LogError(const char* message)
    {
        std::cout << "[ERROR] " << message << "\n";
    }

    inline static void LogError(std::string_view message)
    {
        std::cout << "[ERROR] " << message << "\n";
    }
}; // namespace Core