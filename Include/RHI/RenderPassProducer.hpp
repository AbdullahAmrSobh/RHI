#pragma once

namespace RHI
{

class FrameGraphBuilder;
class ICommandBuffer;

class RenderPassProducer
{
    friend class IFrameScheduler;

public:
    RenderPassProducer()          = default;
    virtual ~RenderPassProducer() = default;

    IRenderPass* GetRenderPass()
    {
        return m_renderpass.get();
    }

protected:
    virtual void SetupAttachments(FrameGraphBuilder& builder) = 0;

    virtual void BuildCommandBuffer(ICommandBuffer& commandBuffer) = 0;

private:
    std::string         m_name;
    std::unique_ptr<IRenderPass> m_renderpass;
};

class RenderPassProducerCallbacks final : public RenderPassProducer
{
public:
    using SetupAttachmentsCallback = std::function<void(FrameGraphBuilder& builder)>;

    using BuildCommandBufferCallback = std::function<void(ICommandBuffer& commandBuffer)>;

    RenderPassProducerCallbacks(std::string                name,
                                SetupAttachmentsCallback   setupAttachmentsCallback,
                                BuildCommandBufferCallback buildCommandBufferCallback)
        : m_setupAttachmentsCallback(setupAttachmentsCallback)
        , m_buildCommandBufferCallback(buildCommandBufferCallback)
    {
    }

    ~RenderPassProducerCallbacks() = default;

private:
    void SetupAttachments(FrameGraphBuilder& builder) override
    {
        m_setupAttachmentsCallback(builder);
    }

    void BuildCommandBuffer(ICommandBuffer& commandBuffer) override
    {
        m_buildCommandBufferCallback(commandBuffer);
    }

private:
    SetupAttachmentsCallback m_setupAttachmentsCallback;

    BuildCommandBufferCallback m_buildCommandBufferCallback;
};

}  // namespace RHI
