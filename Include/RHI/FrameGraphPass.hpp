#pragma once
#include "RHI/FrameGraphAttachment.hpp"

namespace RHI
{

class IFrameGraph;
class ICommandBuffer; 
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
    
    EPassType GetPassType() const;
    
    PassId GetPassId() const;
    
    const std::vector<const ImagePassAttachment*> GetImageAttachments() const;

    const std::vector<const BufferPassAttachment*> GetBufferAttachments() const;
    
    bool HasDepthStencil() const;
    
    const ImagePassAttachment* GetDepthStencilAttachment() const;

private:
    PassId                                       m_passId; 
    std::string                                  m_name;
    EPassType                                    m_passType;
    std::vector<Unique<ImagePassAttachment>>     m_usedImages;
    std::vector<Unique<BufferPassAttachment>>    m_usedBuffers;
};

class IPassCallbacks
{
public:
    IPassCallbacks(IFrameGraph& frameGraph, std::string name, EPassType passType);

    inline const IPass& GetPass() const; 
    
    virtual ~IPassCallbacks() = default;

    virtual void Setup(FrameGraphBuilder& builder) = 0;

    virtual void UseAttachments(IPass& pass) = 0;
    
    virtual void BuildCommandBuffer(ICommandBuffer& commandBuffer) = 0;

private:
    Unique<IPass> m_pass;
};

} // namespace RHI