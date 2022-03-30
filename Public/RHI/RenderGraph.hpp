#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/RenderGraphBuilder.hpp"

namespace RHI
{

class ICommandContext;
class IRenderGraph
{
public:
    virtual ~IRenderGraph() = default;
    
    virtual uint32_t GetFrameIndex() const = 0;
    virtual uint32_t GetFrameCount() const = 0;
    
	virtual ICommandContext& GetPassCommandContext(const PassId id) = 0;
    
    virtual void BeginFrame() = 0;
    virtual void EndFrame()   = 0;
};
using RenderGraphPtr = Unique<IRenderGraph>;

} // namespace RHI
