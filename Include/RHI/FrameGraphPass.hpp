#pragma once
#include "RHI/FrameGraphAttachment.hpp"

namespace RHI
{

struct RenderTargetLayout;

class FrameGraphBuilder;
class ISwapchain;
class ICommandBuffer;

class IPass
{
public:
    virtual ~IPass() = default;

    virtual EResultCode Submit() = 0;

    const RenderTargetLayout& GetRenderTargetLayout() const;

    inline void SetCallbacks(class PassCallbacks& callbacks)
    {
        m_pCallback = &callbacks;
    }

    // Return a union list of all image attachments used by this pass
    inline const std::vector<ImagePassAttachment*>& GetImagePassAttachments() const
    {
        return m_imagePassAttachments;
    }

    // Return a union list of all buffer attachments used by this pass
    inline const std::vector<BufferPassAttachment*>& GetBufferPassAttachments() const
    {
        return m_bufferPassAttachments;
    }

    inline const std::vector<ISwapchain*>& GetSwapchainsToPresent() const
    {
        return m_presentSwapchains;
    }

    inline const std::vector<IFence*>& GetFencesToSignal() const
    {
        return m_signalFences;
    }

protected:
    std::vector<ImagePassAttachment*>  m_imagePassAttachments;
    std::vector<BufferPassAttachment*> m_bufferPassAttachments;
    ImagePassAttachment*               m_pDepthStencilAttachment;

    std::vector<IFence*>     m_signalFences;
    std::vector<ISwapchain*> m_presentSwapchains;
    PassCallbacks*           m_pCallback;

    mutable Unique<RenderTargetLayout> m_renderTargetLayout;
};

class PassCompileContext
{
public:
    IImageView*  GetImageView(ImageAttachmentReference reference);
    IBufferView* GetBufferView(BufferAttachmentReference reference);
    IImage*      GetImageResource(ImageAttachmentReference reference);
    IBuffer*     GetBufferResource(BufferAttachmentReference reference);

private:
    const IPass* m_pPass;
};

class PassExecuteContext
{
public:
    const std::vector<ICommandBuffer*>& GetCommandBuffers() const
    {
        return m_commandBuffers;
    }

private:
    std::vector<ICommandBuffer*> m_commandBuffers;
};

class PassCallbacks
{
public:
    virtual ~PassCallbacks() = default;

    virtual void Setup(FrameGraphBuilder& builder)    = 0;
    virtual void Compile(PassCompileContext& context) = 0;
    virtual void Execute(PassExecuteContext& context) = 0;
};

} // namespace RHI