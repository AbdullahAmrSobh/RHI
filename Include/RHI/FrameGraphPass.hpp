#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraphAttachment.hpp"
#include "RHI/FrameGraphInterface.hpp"

namespace RHI
{
class IFence;

enum class EPassType
{
    Graphics,
    Compute
};

class IPass
{
public:
    IPass(std::string name, EPassType type)
        : m_name(name)
        , m_type(type)
    {
    }

    virtual ~IPass() = default;

    std::string_view GetName() const;

    EPassType GetType() const;

    const IFence& GetFence() const;

    bool HasSwapchainTarget() const;

    bool HasDepthStencil() const;

    const ImagePassAttachment* GetSwapchainAttachemnt() const;

    const ImagePassAttachment* GetDepthStencilAttachment() const;

    const std::vector<const ImagePassAttachment*> GetImageAttachments() const;

    const std::vector<const BufferPassAttachment*> GetBufferAttachments() const;

private:
    std::string m_name;

    EPassType m_type;

    Unique<IFence> m_signalFence;

    ISwapchain* m_pSwapchain;

    std::vector<Unique<ImagePassAttachment>> m_swapchainImagePassAttachments;

    Unique<ImagePassAttachment> m_depthStencilAttachment = nullptr;

    std::vector<Unique<ImagePassAttachment>> m_imageAttachments;

    std::vector<Unique<BufferPassAttachment>> m_bufferAttachments;

};

class IPassProducer
{
public:
    virtual ~IPassProducer() = default;

    inline const IPass& GetPass() const
    {
        return *m_pass;
    }

    inline IPass& GetPass()
    {
        return *m_pass;
    }

    virtual void Setup(FrameGraphBuilder& builder) = 0;

    virtual void UseResources(/*AttachmentAccessContext& context*/){};

    virtual void Execute(ExecuteContext& context) = 0;

private:
    friend class IFrameGraph;

    Unique<IPass> m_pass = nullptr;
};

} // namespace RHI