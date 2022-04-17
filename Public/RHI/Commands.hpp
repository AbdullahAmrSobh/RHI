#pragma once
#include "RHI/CopyCommand.hpp"
#include "RHI/Definitions.hpp"
#include "RHI/DispatchCommand.hpp"
#include "RHI/DrawCommand.hpp"

namespace RHI
{

// Command Lists are executed inside a render graph.

struct ConditionBuffer
{
    ConditionBuffer() = default;
    
	ConditionBuffer(const IBuffer& buffer, size_t offset)
        : pBuffer(&buffer)
        , offset(offset)
    {
    }

    const IBuffer* pBuffer;
    size_t         offset;
};

class ICommandList
{
public:
    inline void SetViewport(const Viewport& viewport) { SetViewports(Span<const Viewport>({viewport})); }
    inline void SetScissor(const Rect& scissor) { SetScissors(Span<const Rect>({scissor})); }

    virtual void Reset() = 0;
    
    virtual void Begin() = 0;
    virtual void End()   = 0;
    
    virtual void SetViewports(Span<const Viewport> viewports) = 0;
    virtual void SetScissors(Span<const Rect> scissors)       = 0;

    virtual void Submit(const DrawCommand& drawCommand)         = 0;
    virtual void Submit(const CopyCommand& copyCommand)         = 0;
    virtual void Submit(const DispatchCommand& dispatchCommand) = 0;

    virtual void BeginConditionalRendering(ConditionBuffer condition) = 0;
    virtual void EndConditionalRendering()                            = 0;
};
using CommandListPtr = Unique<ICommandList>;

} // namespace RHI
