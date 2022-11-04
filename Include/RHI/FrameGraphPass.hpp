#pragma once
#include <functional>
#include <vector>
#include "RHI/Common.hpp"
#include "RHI/FrameGraph.hpp"
#include "RHI/FrameGraphAttachment.hpp"
#include "RHI/Resource.hpp"

namespace RHI
{

<<<<<<< HEAD
class IFrameGraph;
=======
struct RenderTargetLayout;

class FrameGraphResourceAccessContext;
class FrameGraphBuilder;
class ISwapchain;
>>>>>>> 49ff0baea8856acd38e8e358c4e24685c7cec3bb
class ICommandBuffer;
class ISwapchain;
class FrameGraphBuilder;

using PassId = uint32_t;

enum class EPassType
{
    Graphics,
    Compute
};

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
    
    inline const std::string& GetName() const
    {
        return m_name;
    }

    inline PassId GetId() const
    {
        return m_passId;
    }

    inline EPassType GetType() const
    {
        return m_passType;
    }

    bool HasSignalFence() const;

    void SetSignalFence(Unique<IFence> fence);
    
    const IFence& GetSignalFence() const;

    const std::vector<const ImagePassAttachment*> GetImageAttachments() const;

    const std::vector<const BufferPassAttachment*> GetBufferAttachments() const;

    bool HasDepthStencil() const;

    const ImagePassAttachment* GetDepthStencilAttachment() const;

    bool HasSwapchainAttachment() const;

    const std::vector<const ImagePassAttachment*>& GetCurrentSwapchainImageAttachment() const;
    

    void UseImageAttachment(Unique<ImagePassAttachment> attachment);
    void UseDepthStencilAttachment(Unique<ImagePassAttachment> attachment);
    void UseBufferAttachemnt(Unique<BufferPassAttachment> attachment);
    void UseSwapchainImageAttachemnt();

private:
    PassId      m_passId;
    std::string m_name;
    EPassType   m_passType;
    
    std::vector<Unique<ImagePassAttachment>> m_usedImages;
    
    std::vector<Unique<BufferPassAttachment>> m_usedBuffers;

    Unique<ImagePassAttachment> m_depthStencilAttachment;

    ISwapchain* m_pSwapchain;
    
    std::vector<Unique<ImagePassAttachment>> m_backbufferAttachment;

    Unique<IFence> m_signalFence;
};

class IPassCallbacks
{
public:
    IPassCallbacks(IFrameGraph& frameGraph, std::string name, EPassType passType);
    virtual ~IPassCallbacks() = default;

    inline const IPass& GetPass() const
    {
        return *m_pass;
    }
    
    virtual void Setup(FrameGraphBuilder& builder) = 0;

    virtual void UseAttachments(IPass& pass) = 0;

    virtual void BuildCommandBuffer(ICommandBuffer& commandBuffer) = 0;

private:
    Unique<IPass> m_pass;
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