#pragma once

#include <memory>
#include <string_view>

#include <RHI/Export.hpp>

#if defined(_MSC_VER)
#    define RHI_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) | defined(__clang__)
#    define RHI_DEBUG_BREAK() __builtin_trap()
#else
#    error "Unsupported compiler"
#endif

#define RHI_LOG(...)   ::RHI::Debug::Get().Log(__VA_ARGS__)
#define RHI_WARN(...)  ::RHI::Debug::Get().Warn(__VA_ARGS__)
#define RHI_ERROR(...) ::RHI::Debug::Get().Error(__VA_ARGS__)

#define RHI_ASSERT(expression)          \
    if (!(expression))                  \
    {                                   \
        RHI_DEBUG_BREAK();              \
    }

#define RHI_ASSERT_MSG(expression, ...) \
    if (!(expression))                  \
    {                                   \
        RHI_ERROR(__VA_ARGS__);         \
        RHI_DEBUG_BREAK();              \
    }

#define RHI_UNREACHABLE()        RHI_ASSERT(false)
#define RHI_UNREACHABLE_MSG(...) RHI_ASSERT_MSG(false, __VA_ARGS__)

namespace RHI
{

class DebugCallbacks
{
public:
    virtual ~DebugCallbacks() = default;

    virtual void Log(std::string_view message)   = 0;
    virtual void Warn(std::string_view message)  = 0;
    virtual void Error(std::string_view message) = 0;
};

class Debug
{
public:
    static void Init(std::unique_ptr<DebugCallbacks> callbacks);

    static DebugCallbacks& Get();

private:
    inline static std::shared_ptr<DebugCallbacks> s_callbacks;
};

}  // namespace RHI