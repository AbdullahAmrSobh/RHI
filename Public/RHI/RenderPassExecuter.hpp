#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/ExecuteContext.hpp"
#include "RHI/RenderPass.hpp"

namespace RHI
{

class RenderPassExecuter
{
public:
    RenderPassExecuter(RenderPassId id)
        : m_renderPassId(id)
    {
    }
    virtual ~RenderPassExecuter() = default;

    inline RenderPassId GetRenderPassId() const
    {
        return m_renderPassId;
    }

    virtual void Setup(IRenderPass& frameGraph)        = 0;
    virtual void Execute(ExecuteContext& commandList) = 0;
    // virtual void OnSwapChainResize(Extent2D newExtent) = 0;

protected:
    friend class FrameGraph;

    RenderPassId m_renderPassId;

    void SetRenderPassId(RenderPassId renderPassId)
    {
        m_renderPassId = renderPassId;
    }
};

template <typename UserData>
class RenderPassExecuterCallbacks final : public RenderPassExecuter
{
public:
    using SetupCallback   = std::function<void(IRenderPass&, UserData&)>;
    using ExecuteCallback = std::function<void(ExecuteContext&, UserData&)>;

    RenderPassExecuterCallbacks(UserData userData, SetupCallback setupCallback, ExecuteCallback executeCallback)
        : m_userData(userData)
        , m_setup(setupCallback)
        , m_execute(executeCallback)
    {
    }

    inline virtual void Setup(IRenderPass& frameGraph) override
    {
        m_setup(frameGraph, m_userData);
    }

    inline virtual void Execute(ExecuteContext& commandList) override
    {
        m_execute(commandList, m_userData);
    }

private:
    UserData        m_userData;
    SetupCallback   m_setup;
    ExecuteCallback m_execute;
};

} // namespace RHI
