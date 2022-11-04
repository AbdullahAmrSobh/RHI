#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraphAttachment.hpp"

namespace RHI
{

class FrameGraphBuilder;
class FrameGraphContext
{
public:
};

class IFrameGraph;

enum class EPassType
{
    Graphics,
    Compute
};

class IPassProducer
{
    friend class IFrameGraph;

public:
    IPassProducer(std::string name, EPassType passType);
    virtual ~IPassProducer() = default;

    virtual void Setup(FrameGraphBuilder& builder)                 = 0;
    virtual void Compile(FrameGraphContext& context)               = 0;
    virtual void BuildCommandBuffer(ICommandBuffer& commandBuffer) = 0;

private:
    Unique<IPass> m_pass = nullptr;
};

class IPass
{
    friend class IFrameGraph; 
public:
    IPass(std::string name, EPassType type)
        : m_name(name)
        , m_type(type)
        , m_valid(false)
    {
    }
    virtual ~IPass() = default;

    inline bool IsCompiled() const
    {
        return m_compiled;
    }

    inline bool IsValid() const
    {
        return m_valid;
    }

    inline EPassType GetType() const
    {
        return m_type;
    }

    inline const IFence& GetSignalFence() const
    {
        return *m_signalFence;
    }

    inline bool HasDepthStencil() const
    {
        return m_depthStencilAttachment != nullptr;
    }

    inline const ImagePassAttachment* GetDepthStencilAttachment() const
    {
        return m_depthStencilAttachment.get();
    }

    const std::vector<const ImagePassAttachment*> GetImageAttachments() const;

    const std::vector<const BufferPassAttachment*> GetBufferAttachments() const;

    inline bool HasSwapchainTarget() const
    {
        return m_pSwapchain;
    }

    const std::vector<const ImagePassAttachment*> GetSwapchainAttachemnts() const;

protected:
    std::string m_name;

    Unique<IFence>                            m_signalFence;
    Unique<ImagePassAttachment>               m_depthStencilAttachment;
    std::vector<Unique<ImagePassAttachment>>  m_imageAttachments;
    std::vector<Unique<BufferPassAttachment>> m_bufferAttachments;

    ISwapchain*                              m_pSwapchain;
    std::vector<Unique<ImagePassAttachment>> m_swapchainImages;

    std::vector<Unique<ICommandBuffer>> m_commandBuffers;
    
    EPassType m_type;
    bool      m_valid;
    bool      m_compiled;
};

class IFrameGraph
{
public:
    virtual ~IFrameGraph() = default;

    /// Graph Interface
    EResultCode BeginFrame();
    EResultCode EndFrame();

    EResultCode BeginPass(IPass& pass);
    EResultCode EndPass();

    EResultCode Execute(IPassProducer& producer);

    virtual EResultCode BeginFrameInternal() = 0;

    virtual EResultCode EndFrameInternal() = 0;

    virtual EResultCode CompilePass(IPass& pass) = 0;
    
    virtual EResultCode Submit(IPass& pass) = 0;

private:
    // FrameGraphBuilder interface.
    friend class FrameGraphBuilder;

private:
    std::vector<Unique<ISwapchain>> m_swapchains;
    
    uint32_t m_currentIndex; 
    uint32_t m_maxIndex; 
};

class FrameGraphBuilder
{
public:
    FrameGraphBuilder(IFrameGraph* frameGraph);

    EResultCode UseImageAttachment(const ImagePassAttachment& attachmentDesc, EAttachmentUsage usage, EAttachmentAccess access)
    {
        return EResultCode::Fail;
    }

    EResultCode UseImageAttachments(const std::vector<ImagePassAttachmentDesc>& attachmentDesc, EAttachmentUsage usage, EAttachmentAccess access)
    {
        return EResultCode::Fail;
    }

    EResultCode UseRenderTargetAttachments(const std::vector<ImagePassAttachmentDesc>& attachmentDesc, EAttachmentAccess access)
    {
        return EResultCode::Fail;
    }

    EResultCode UseRenderTargetAttachment(const std::vector<ImagePassAttachmentDesc>& attachmentDesc, EAttachmentAccess access)
    {
        return EResultCode::Fail;
    }

    EResultCode UseDepthStencilAttachment(const ImagePassAttachmentDesc& attachmentDesc, EAttachmentAccess access)
    {
        return EResultCode::Fail;
    }

    EResultCode UseBufferAttachemnt(const BufferPassAttachment& attachmentDesc, EAttachmentUsage usage, EAttachmentAccess access)
    {
        return EResultCode::Fail;
    }

    EResultCode WaitForPass(const IPass& pass)
    {
        return EResultCode::Fail;
    }

private:
    IFrameGraph* m_pFrameGraph;
    IPass*       m_pass;
};

} // namespace RHI