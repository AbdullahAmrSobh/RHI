#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraphAttachment.hpp"

namespace RHI
{

class IFence;
class IFrameGraph;
class ICommandBuffer;
class ISwapchain;
class FrameGraphBuilder;

using PassId = uint32_t;

enum class EPassType
{
    Graphics,
    Compute
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

} // namespace RHI