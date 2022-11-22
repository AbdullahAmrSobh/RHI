#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraphAttachment.hpp"

namespace RHI
{

class IPass;
class IFence;
class FrameGraphBuilder;
class FrameGraphContext;
class ICommandBuffer;

enum class EPassType
{
    Graphics,
    Compute
};

class IPassProducer
{
    friend class IFrameGraph;

public:
    virtual ~IPassProducer() = default;

    inline const IPass& GetPass() const
    {
        return *m_pass;
    }

    virtual void Setup(FrameGraphBuilder& builder)                 = 0;
    virtual void Compile(FrameGraphContext& context)               = 0;
    virtual void BuildCommandBuffer(ICommandBuffer& commandBuffer) = 0;

private:
    Unique<IPass> m_pass = nullptr;
    std::string m_name;
    EPassType m_passType;
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

    inline bool HasSwapchainTarget() const
    {
        return m_pSwapchain;
    }

    const std::vector<const ImagePassAttachment*> GetImageAttachments() const;

    const std::vector<const BufferPassAttachment*> GetBufferAttachments() const;

    const std::vector<const ImagePassAttachment*> GetSwapchainAttachemnts() const;

protected:
    std::string m_name;

    Unique<IFence> m_signalFence;

    Unique<ImagePassAttachment> m_depthStencilAttachment;

    std::vector<Unique<ImagePassAttachment>> m_imageAttachments;

    std::vector<Unique<BufferPassAttachment>> m_bufferAttachments;

    ISwapchain* m_pSwapchain;

    std::vector<Unique<ImagePassAttachment>> m_swapchainImages;

    std::vector<Unique<ICommandBuffer>> m_commandBuffers;

    EPassType m_type;

    bool m_valid;

    bool m_compiled;
};

} // namespace RHI