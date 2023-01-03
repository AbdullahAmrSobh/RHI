#pragma once

namespace RHI
{

enum class DebugMessageSeverity
{
    Log,
    Warn,
    Error,
    Fatel,
};

class IDebugCallbacks
{
public:
    virtual ~IDebugCallbacks() = default;

    virtual void Log(std::string_view message)   = 0;
    virtual void Warn(std::string_view message)  = 0;
    virtual void Error(std::string_view message) = 0;
};

class Debug
{
public:
    static void Init(Unique<IDebugCallbacks> callbacks);

    static Shared<IDebugCallbacks>& Get();
};

}  // namespace RHI

#define RHI_LOG(...)   ::RHI::Debug::Get()->Log(__VA_ARGS__)
#define RHI_WARN(...)  ::RHI::Debug::Get()->Warn(__VA_ARGS__)
#define RHI_ERROR(...) ::RHI::Debug::Get()->Error(__VA_ARGS__)

#define RHI_DEBUG_BREAK() assert(false)
#define RHI_ASSERT(expression, ...)                                                                                                        \
    if (!expression)                                                                                                                       \
    {                                                                                                                                      \
        RHI_ERROR(__VA_ARGS__);                                                                                                            \
        RHI_DEBUG_BREAK();                                                                                                                 \
    }