#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

class ICommandContext;
class IFence;
class ISwapChain;

struct SwapchainPresentDesc
{
    ISwapChain* m_pSwapchain;
    
};

class IQueue
{
public:
    virtual ~IQueue() = default;
    
    virtual void Submit(ICommandContext& cmdCtx, IFence& signalFence) = 0;
    
    virtual void Present(const SwapchainPresentDesc& desc) = 0;
};

} // namespace RHI
