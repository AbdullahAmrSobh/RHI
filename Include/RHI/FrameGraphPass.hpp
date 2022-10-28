#pragma once
#include <functional>
#include <vector>
#include "RHI/Common.hpp"
#include "RHI/FrameGraph.hpp"
#include "RHI/FrameGraphAttachment.hpp"
#include "RHI/Resource.hpp"

namespace RHI
{

struct RenderTargetLayout;

class FrameGraphResourceAccessContext;
class FrameGraphBuilder;
class ISwapchain;
class ICommandBuffer;

enum class EHardwareQueueType
{
    Graphics,
    Compute,
    Transfer
};

class IPassExecuter
{
public:
    virtual void BuildGraph(FrameGraphBuilder& builder)                 = 0;
    virtual void UseResources(FrameGraphResourceAccessContext& context) = 0;
    virtual void BuildCommandBuffer(ICommandBuffer& commandBuffer)      = 0;
};

class PassExecuterCallbacks final : public IPassExecuter
{
public:
    using BuildFrameGraphCallback    = std::function<void(FrameGraphBuilder& builder)>;
    using ResourceAccessCallback     = std::function<void(FrameGraphResourceAccessContext& context)>;
    using BuildCommandBufferCallback = std::function<void(ICommandBuffer& commandBuffer)>;

    virtual void BuildGraph(FrameGraphBuilder& builder) override
    {
        m_buildFrameGraphCallback(builder);
    }

    virtual void UseResources(FrameGraphResourceAccessContext& context) override
    {
        m_resourceAccessCallback(context);
    }
    
    virtual void BuildCommandBuffer(ICommandBuffer& commandBuffer) override
    {
        m_buildCommandBufferCallback(commandBuffer);
    }

private:
    BuildFrameGraphCallback    m_buildFrameGraphCallback;
    ResourceAccessCallback     m_resourceAccessCallback;
    BuildCommandBufferCallback m_buildCommandBufferCallback;
};

class IPassBase
{
public:
    virtual ~IPassBase() = default;

    const std::string_view GetName() const;

    const std::vector<ImagePassAttachment*>&  GetImageAttachments();
    const std::vector<BufferPassAttachment*>& GetBufferAttachments();

    void SetSignalPass(Unique<IFence>& signalFence);

    virtual EResultCode Submit() = 0;

protected:
    uint32_t*                                 m_pIndexPtr;
    std::vector<Unique<ImagePassAttachment>>  m_imageAttachments;
    std::vector<Unique<BufferPassAttachment>> m_bufferAttachments;

    Unique<IFence> m_fence;
};

class IRenderPass : public IPassBase
{
public:
    bool HasDepthStencil() const;
};

class IComputePass : public IPassBase
{
public:
};

class IPass
{
public:
    virtual ~IPass() = default;

    virtual EResultCode Submit()  = 0;
    virtual EResultCode Compile() = 0;

    inline void SetCallbacks(class PassCallbacks& callbacks)
    {
        m_pCallback = &callbacks;
    }

    inline EHardwareQueueType GetQueueType() const
    {
        return m_queueType;
    }

    // Return a union list of all image attachments used by this pass
    inline const std::vector<ImagePassAttachment*>& GetImagePassAttachments() const
    {
        return m_imagePassAttachments;
    }

    inline const ImagePassAttachment* GetDepthStencilAttachment() const
    {
        return m_pDepthStencilAttachment;
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

    inline const IFence* GetSignalFence() const
    {
        return m_pSignalFences;
    }

    inline bool HasDepthStencil() const
    {
        return m_pDepthStencilAttachment == nullptr;
    }

protected:
    EHardwareQueueType m_queueType;

    std::vector<ImagePassAttachment*>  m_imagePassAttachments;
    std::vector<BufferPassAttachment*> m_bufferPassAttachments;
    ImagePassAttachment*               m_pDepthStencilAttachment;

    IFence* m_pSignalFences;

    std::vector<ISwapchain*> m_presentSwapchains;
    PassCallbacks*           m_pCallback;

    IFrameGraph* m_pFrameGraph;
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

class PassCallbacks
{
public:
    virtual ~PassCallbacks() = default;

    virtual void Setup(FrameGraphBuilder& builder)      = 0;
    virtual void Compile(PassCompileContext& context)   = 0;
    virtual void Execute(ICommandBuffer& commandBuffer) = 0;
};

class PassCallbacksFn final : public PassCallbacks
{
public:
    using SetupCallback   = std::function<void(FrameGraphBuilder& builder)>;
    using CompileCallback = std::function<void(PassCompileContext& context)>;
    using ExecuteCallback = std::function<void(ICommandBuffer& commandBuffer)>;

    PassCallbacksFn(SetupCallback setupCallback, CompileCallback compileCallback, ExecuteCallback executeCallback)
        : m_setupCallback(setupCallback)
        , m_compileCallback(compileCallback)
        , m_executeCallback(executeCallback)
    {
    }

    virtual ~PassCallbacksFn() = default;

    virtual void Setup(FrameGraphBuilder& builder) override
    {
        m_setupCallback(builder);
    }

    virtual void Compile(PassCompileContext& context) override
    {
        m_compileCallback(context);
    }

    virtual void Execute(ICommandBuffer& commandBuffer) override
    {
        m_executeCallback(commandBuffer);
    }

private:
    SetupCallback   m_setupCallback;
    CompileCallback m_compileCallback;
    ExecuteCallback m_executeCallback;
};

} // namespace RHI