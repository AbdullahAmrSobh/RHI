#pragma once

namespace RHI
{

enum class ECommandQueueType
{
    Graphics,
    Compute,
    Transfer,
};

enum class EAttachmentType
{
    Buffer,
    Image,
    Resolve, // TODO
};

enum class EAttachmentAccess
{
    Read,
    Write,
    ReadWrite,
};

enum class EAttachmentUsage
{
    Uninitialized = 0,
    RenderTarget,
    DepthStencil,
    Shader,
    Copy,
};

struct ImageAttachment
{
    std::string   name;
    ImageViewDesc desc;
};

struct BufferAttachment
{
    std::string    name;
    BufferViewDesc desc;
};

class Attachment
{
public:
    using Id = std::string;
    struct Iterator;

private:
    Id m_id;
};

class ImageAttachment final : public Attachment
{
public:
    IImage&       GetImage();
    const IImage& GetImage() const;

private:
    Unique<IImage> m_resource;
};

class BufferAttachment final : public Attachment
{
public:
    IBuffer&       GetBuffer();
    const IBuffer& GetBuffer() const;

private:
    Unique<IBuffer> m_resource;
};

class SwapchainAttachment final : public Attachment
{
public:
private:
};

class AttachmentView
{
};

class ImageAttachmentView final : public AttachmentView
{
public:
    Unique<IImageView> m_view;
};

class BufferAttachmentView final : public AttachmentView
{
public:
    Unique<IBufferView> m_view;
};

class SwapchainAttachmentView final : public AttachmentView
{
};

class RenderPass
{
public:
    using Id = std::string;

    struct AttachmentDesc
    {
        std::string name;
    };

    struct ImageAttachmentDesc : AttachmentDesc
    {
        ImageViewRange viewRange;
    };

    struct BufferAttachmentDesc : AttachmentDesc
    {
        BufferViewRange viewRange;
    };

public:
    class Interface;
    class Builder;

    RenderTargetLayout GetRenderTargetLayout();
};

class FrameGraph
{
public:
    struct Compiler;
    struct Builder;

    // Attachments declaration.

    // Creates a new transient attachment resource, which exsist inside the framegraph.
    EResultCode CreateTransientImageAttachment(const ImageAttachmentDesc& desc);

    EResultCode CreateTransientBufferAttachment(const ImageAttachmentDesc& desc);

    // Imports external resources as an attachments, and takes their ownership.
    EResultCode ImportImage(const Attachment::Id& id, Unique<IImage>& image);

    EResultCode ImportBuffer(const Attachment::Id& id, Unique<IBuffer>& buffer);

    // Assigns ID to list of swapchain images
    EResultCode UseSwapchainImage(const Attachment::Id& id, ISwapChain& buffer);

    void BeginRenderPass();
    void EndRenderPass();

    // Building the graph.
    EResultCode UseImageAttachment(const RenderPass::ImageAttachmentDesc& desc, EAttachmentUsage usage, EAttachmentAccess access);
    
    EResultCode UseBufferAttachment(const RenderPass::BufferAttachmentDesc& desc, EAttachmentUsage usage, EAttachmentAccess access);
    
    void SignalFence(Unique<IFence>& fence);
    
    // Querying the graph.
    const std::vector<RenderPass*>& GetRenderPasses();

    RenderPass* GetRenderPass(const RenderPass::Id& id);

    const std::vector<RenderPass*>& GetConsumers(const RenderPass& producer) const;
    
    const std::vector<RenderPass*>& GetProducers(const RenderPass& consumer) const;

    RenderPass* GetAttachmentProducer(const Attachment::Id& id);
    
    std::vector<RenderPass*> GetAttachmentConsumers(const Attachment::Id& id);

    // Referenceing resources.
    IBufferView* GetBufferView(const RenderPass::Id& renderPassId, const Attachment::Id& attachmentId);

    IImageView* GetImageView(const RenderPass::Id& renderPassId, const Attachment::Id& id);

private:
    void        InsertEdge();
    EResultCode TopologicalSort();
    
private:
    // Attachments
    std::vector<FrameAttachment*>                      m_attachments;
    std::unordered_map<AttachmentId, FrameAttachment*> m_attachmentLookup;
    std::vector<SwapChainFrameAttachment*>             m_swapChainAttachments;
    std::vector<ImageFrameAttachment*>                 m_imageAttachments;
    std::vector<ImageFrameAttachment*>                 m_importedImageAttachments;
    std::vector<ImageFrameAttachment*>                 m_transientImageAttachments;
    std::vector<BufferFrameAttachment*>                m_bufferAttachments;
    std::vector<BufferFrameAttachment*>                m_importedBufferAttachments;
    std::vector<BufferFrameAttachment*>                m_transientBufferAttachments;

    // Graph
    std::unordered_map<RenderPass::Id, Unique<RenderPass>> m_renderPasses;
    std::vector<RenderPass*>                               m_pRenderPasses;
    
    struct GraphEdge
    {
        uint32_t src;
        uint32_t dst;
    };

    struct GraphNode
    {
        RenderPass*           pRenderPass;
        std::vector<uint32_t> producers;
        std::vector<uint32_t> consumers;
    };

    RenderPass* m_pCurrentRenderPass;

    std::vector<GraphEdge> m_graphEdges;
    std::vector<GraphNode> m_graphNode;
};

class RenderPass::Interface
{
public:
    virtual void Setup(FrameGraph& frameGraph)
    {
        RenderPass::ImageAttachmentDesc colorAttachment;
        colorAttachment.id = "ColorAttachment";
         
        frameGraph.UseImageAttachment();
    }
    
    virtual void BuildCommandBuffer() {}
};

} // namespace RHI