#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/FrameGraphAttachment.hpp"

namespace RHI
{

class IFence;
class IFrameGraph;
class RenderPassExecuter;
struct RenderTargetLayout;

struct RenderPassId : public Id
{
    using Id::Id;
};

class IRenderPass
{
public:
    IRenderPass(IFrameGraph& frameGraph, RenderPassExecuter& executer, RenderPassId id, std::string name, ERenderPassQueueType queueType)
        : m_pFrameGraph(&frameGraph)
        , m_pExecuter(&executer)
        , m_name(name)
        , m_renderPassId(id)
        , m_queueType(queueType)
    {
    }

    virtual ~IRenderPass() = default;

    inline RenderPassId GetRenderPassId() const
    {
        return m_renderPassId;
    }

    inline std::string GetName() const
    {
        return m_name;
    }

    inline ERenderPassQueueType GetQueueType() const
    {
        return m_queueType;
    }

    inline RenderPassExecuter& GetRenderPassExecuter() const
    {
        return *m_pExecuter;
    }

    inline void UseImageAttachment(ImageAttachmentId id, EAttachmentUsage usage, EAttachmentAccess access)
    {
        UseImageAttachments({&id, 1}, usage, access);
    }

    inline void UseBufferAttachment(BufferAttachmentId id, EAttachmentUsage usage, EAttachmentAccess access)
    {
        UseBufferAttachments({&id, 1}, usage, access);
    }

    inline void UseDepthStencilAttachment(ImageAttachmentId id, EAttachmentAccess access)
    {
        UseImageAttachments({&id, 1}, EAttachmentUsage::DepthStencil, access);
    }

    inline void UseInputAttachment(ImageAttachmentId id, EAttachmentAccess access)
    {
        UseImageAttachments({&id, 1}, EAttachmentUsage::Input, access);
    }

    inline void UseResolveAttachment(ImageAttachmentId id, EAttachmentAccess access)
    {
        UseImageAttachments({&id, 1}, EAttachmentUsage::Resolve, access);
    }

    inline void UseCopyImageAttachment(ImageAttachmentId id, EAttachmentAccess access)
    {
        UseImageAttachments({&id, 1}, EAttachmentUsage::Copy, access);
    }

    inline void UseCopyBufferAttachments(BufferAttachmentId id, EAttachmentAccess access)
    {
        UseBufferAttachments({&id, 1}, EAttachmentUsage::Copy, access);
    }

    inline IFence& GetSignalFence()
    {
        return *m_pSignalFence;
    }

    inline IFence& GetWaitFence()
    {
        return *m_pWaitFence;
    }

    inline void SignalFecne(IFence& fence)
    {
        m_pSignalFence = &fence;
    }

    inline void WaitFence(IFence& fence)
    {
        m_pWaitFence = &fence;
    }
    
    virtual RenderTargetLayout GetRenderTargetLayout() const = 0;
    
    virtual void Invalidate() = 0;
    
    virtual void UseSwapChainAttachment(SwapChainAttachmentId id, EAttachmentUsage usage, EAttachmentAccess access = EAttachmentAccess::Read) = 0;
    
    virtual void UseImageAttachments(Span<ImageAttachmentId> ids, EAttachmentUsage usage, EAttachmentAccess access) = 0;
    
    virtual void UseBufferAttachments(Span<BufferAttachmentId> ids, EAttachmentUsage usage, EAttachmentAccess access) = 0;

protected:
    IFrameGraph*         m_pFrameGraph;
    RenderPassId         m_renderPassId;
    std::string          m_name;
    ERenderPassQueueType m_queueType;
    RenderPassExecuter*  m_pExecuter;
    IFence*              m_pSignalFence = nullptr;
    IFence*              m_pWaitFence   = nullptr;
};
using RenderPassPtr = Unique<IRenderPass>;

} // namespace RHI
