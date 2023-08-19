#pragma once

#include <memory>
#include <string_view>

#include <RHI/Export.hpp>

/// This header should be included in source files, and not used in headers.
/// This be used by RHI objects types only, which has pointer to the context
/// as member variable named m_context.
#define RHI_LOG(...)   ::RHI::GetDebugCallbacks(m_context).Log(__VA_ARGS__)
#define RHI_WARN(...)  ::RHI::GetDebugCallbacks(m_context).Warn(__VA_ARGS__)
#define RHI_ERROR(...) ::RHI::GetDebugCallbacks(m_context).Error(__VA_ARGS__)

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

DebugCallbacks& GetDebugCallbacks(Context* context)
{
    return *context->m_debugCallbacks;
}

}  // namespace RHI