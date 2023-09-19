#pragma once

#include "RHI/Context.hpp"

namespace RHI
{

class DebugCallbacks;
class Context;

/// @brief A base class for all RHI objects.
class Object
{
public:
    Object(Context* context)
        : m_context(context)
    {
    }

protected:
    RHI_FORCE_INLINE DebugCallbacks& Logger() const
    {
        return m_context->GetDebugCallbacks();
    }

protected:
    Context* m_context;
};

}  // namespace RHI