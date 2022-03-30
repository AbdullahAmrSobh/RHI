#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/DrawCommand.hpp"
#include "RHI/CopyCommand.hpp"
#include "RHI/DispatchCommand.hpp"

namespace RHI
{

// Command Lists are executed inside a render graph.
class ICommandContext
{
public:
    inline void SetViewport(const Viewport& viewport) { SetViewports(ArrayView<const Viewport>({viewport})); }
    inline void SetScissor(const Rect& scissor) { SetScissors(ArrayView<const Rect>({scissor})); }
    
    virtual void Begin() = 0;
    virtual void End()   = 0;
    
    virtual void SetViewports(ArrayView<const Viewport>) = 0;
    virtual void SetScissors(ArrayView<const Rect>)      = 0;
    
    virtual void Submit(const DrawCommand&) = 0;
    virtual void Submit(const CopyCommand&)    = 0;
    virtual void Submit(const DispatchCommand&) = 0;
};
using CommandContextPtr = Unique<ICommandContext>;

} // namespace RHI
