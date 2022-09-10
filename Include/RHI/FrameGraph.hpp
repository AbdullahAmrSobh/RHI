#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "RHI/Commands.hpp"
#include "RHI/Common.hpp"
#include "RHI/Format.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Resource.hpp"

namespace RHI
{
class ISwapchain;
struct RenderTargetLayout;
class Pass;
class PassAttachment;
class Attachment;

enum class EHardwareQueueType
{
    Graphics,
    Compute,
    Transfer,
};

struct Lifetime
{
    bool     imported = false;
    uint32_t begin;
    uint32_t end;
};

enum class EAttachmentUsage
{
    DepthStencil,
    RenderTarget,
    ShaderInput,
    Input,
    Transfer,
};

class Attachment
{
public:
    using Id = std::string;

    enum class EType
    {
        Image,
        Buffer,
        Resolve,
        Input,
    };

    struct Lifetime
    {
        uint16_t begin;
        uint16_t end;
    };

public:
    const Id& GetId() const;
    EType     GetType() const;
    EFormat   GetFormat() const;
    Lifetime  GetLifetime() const;

    std::vector<Pass*> GetProducerPass() const;
    std::vector<Pass*> GetConsumersPass() const;

    std::vector<PassAttachment*> GetFirstUse() const;
    std::vector<PassAttachment*> GetLastUse() const;

private:
    Id                 m_id;
    EType              m_type;
    EFormat            m_format;
    Lifetime           m_lfetime;
    Pass*              m_pProducer;
    std::vector<Pass*> m_consumersPtr;
};

struct ImageAttachmentDesc
{
    Attachment::Id id;
    ImageDesc      imageDesc;
};

struct BufferAttachmentDesc
{
    Attachment::Id id;
    BufferDesc     bufferDesc;
};

class ImageAttachment final : public Attachment
{
public:
    IImage& GetImage();

private:
    Unique<IImage> m_imageResource;
};

class BufferAttachment final : public Attachment
{
public:
    IBuffer& GetBuffer();

private:
    Unique<IBuffer> m_bufferResource;
};

class SwapchainAttachment final : public Attachment
{
public:
    ISwapchain& GetSwapchain();

private:
    ISwapchain* m_pSwapchain;
};

struct ImagePassAttachmentDesc
{
    Attachment::Id id;
    ImageViewDesc  view;
};

struct BufferPassAttachmentDesc
{
    Attachment::Id id;
    BufferViewDesc view;
};

class PassAttachment
{
};

class ImagePassAttachment final : public PassAttachment
{
};

class BufferPassAttachment final : public PassAttachment
{
};

class AttachmentsDatabase
{
public:
    EResultCode CreateTransientImageAttachment(const ImageAttachmentDesc& desc);

    EResultCode CreateTransientBufferAttachment(const BufferAttachmentDesc& desc);

    EResultCode ImportImage(const Attachment::Id& id, Unique<IImage>& image);

    EResultCode ImportBuffer(const Attachment::Id& id, Unique<IBuffer>& buffer);

    EResultCode UseSwapchainImage(const Attachment::Id& id, ISwapchain& buffer);

private:
    // Union of all Attachments.
    std::vector<Attachment*> m_attachments;

    // Attachments lookup.
    std::unordered_map<Attachment::Id, Attachment*> m_attachmentLookup;

    // All Swapchain attachments.
    std::vector<SwapchainAttachment*> m_swapChainAttachments;

    // Union of all image attachments.
    std::vector<ImageAttachment*> m_imageAttachments;

    // Imported Image Attachments.
    std::vector<ImageAttachment*> m_importedImageAttachments;

    // Transient Image Attachments.
    std::vector<ImageAttachment*> m_transientImageAttachments;

    // Union of all buffer Attachments.
    std::vector<BufferAttachment*> m_bufferAttachments;

    // Imported Buffer Attachments.
    std::vector<BufferAttachment*> m_importedBufferAttachments;

    // Transient Buffer Attachments.
    std::vector<BufferAttachment*> m_transientBufferAttachments;
};

class Pass
{
public:
    using Id = std::string;

    class Callbacks;

    template <typename UserData>
    class CallbacksFn;

public:
    Id                 GetId() const;
    EHardwareQueueType GetQueueType();

    void SetCallbacks(Callbacks& callbacks);

    RenderTargetLayout GetRenderTargetLayout() const;
};

class FrameGraph
{
public:
    class ExecuteContext;
    class BindingContext;

    class Compiler;
    class Builder;
    friend class FrameGraph::Builder;

    Pass& AddRenderPass(std::string name, EHardwareQueueType queueType);

    AttachmentsDatabase& GetDatabase();

    void BeginFrame();
    void EndFrame();

private:
    void BeginRenderPass(Pass& renderPass);
    void EndRenderPass();

    // Building the graph.
    EResultCode UseImageAttachment(const ImagePassAttachmentDesc& desc, EAttachmentUsage usage, EAccess access);
    EResultCode UseBufferAttachment(const BufferPassAttachmentDesc& desc, EAttachmentUsage usage, EAccess access);
    EResultCode UseDepthStencilAttachment(const ImagePassAttachmentDesc& desc);
    void        SignalFence(Unique<IFence>& fence);

    // return a list of topologically sorted render passes.
    const std::vector<Pass*>& GetRenderPasses() const;

    // return a list of all render passes which consumes the given render pass.
    const std::vector<Pass*>& GetConsumers(const Pass& producer) const;

    // return a list of all render passes which produces the given render pass.
    const std::vector<Pass*>& GetProducers(const Pass& consumer) const;

    // Return the RenderPass which produces the attachment assocaited with id.
    Pass* GetAttachmentProducer(const Attachment::Id& id);

private:
    Unique<AttachmentsDatabase> m_database;

    // Graph
    std::unordered_map<Pass::Id, Unique<Pass>> m_renderPasses;
    std::vector<Pass*>                         m_renderPassesPtr;

    struct GraphEdge
    {
        uint32_t src;
        uint32_t dst;
    };

    struct GraphNode
    {
        Pass*                 pRenderPass;
        std::vector<uint32_t> producers;
        std::vector<uint32_t> consumers;
    };

    Pass* m_pCurrentRenderPass;

    std::vector<GraphEdge> m_graphEdges;
    std::vector<GraphNode> m_graphNode;
};

class ExecuteContext
{
public:
    ICommandBuffer&                    GetCommandBuffer();
    const std::vector<ICommandBuffer*> GetCommandBuffers(uint32_t count);
};

class AttachmentsBindingContext
{
public:
    IImage* GetImage(Attachment::Id id);

    IImageView* GetImageView(Attachment::Id id);

    IBufferView* GetBufferView(Attachment::Id id);

    IBuffer* GetBuffer(Attachment::Id id);
};

class FrameGraph::Builder
{
public:
    void UseImageAttachment(const std::vector<ImagePassAttachmentDesc>& colorAttachments, EAttachmentUsage usage, EAccess access);

    void UseColorAttachment(const ImagePassAttachmentDesc& colorAttachment, EAccess access);

    void UseColorAttachments(const std::vector<ImagePassAttachmentDesc>& colorAttachments, EAccess access);

    void UseDepthStencilAttachment(const ImagePassAttachmentDesc& colorAttachment, EAccess access);

    void UseBufferAttachment(const BufferPassAttachmentDesc& viewDesc, EAttachmentUsage usage, EAccess access);

    void UseTransferAttachment(const ImagePassAttachmentDesc& colorAttachment, EAccess access);

    void UseTransferAttachmet(const BufferPassAttachmentDesc& colorAttachment, EAccess access);

    void SignalFence(Unique<IFence>& fence);

    void ExecuteAfter(const Pass::Id& pass);

    void ExecuteBefore(const Pass::Id& pass);

    void ExecuteAfter(const Pass& pass);

    void ExecuteBefore(const Pass& pass);

private:
    FrameGraph* m_pFrameGraph;
};

class Pass::Callbacks
{
public:
    virtual void Setup(FrameGraph::Builder& frameGraph)              = 0;
    virtual void BindAttachments(AttachmentsBindingContext& context) = 0;
    virtual void Execute(ExecuteContext& context)                    = 0;
};

template <typename UserData>
class CallbacksFn : public Pass::Callbacks
{
public:
    using SetupCallback             = std::function<void(FrameGraph::Builder&, UserData&)>;
    using AttachmentBindingCallback = std::function<void(AttachmentsBindingContext&, UserData&)>;
    using ExecuteCallback           = std::function<void(ExecuteContext&, UserData&)>;

    CallbacksFn(UserData data, SetupCallback setupFn, AttachmentBindingCallback bindingFn, ExecuteCallback executeFn)
        : m_data(data)
        , m_setupFn(setupFn)
        , m_bindingFn(bindingFn)
        , m_executeFn(executeFn)
    {
    }

    UserData&       GetData();
    const UserData& GetData() const;

    virtual void Setup(FrameGraph::Builder& frameGraph) override
    {
        m_SetupCallback(frameGraph, m_data);
    }

    virtual void BindAttachments(AttachmentsBindingContext& context) override
    {
        m_AttachmentBindingCallback(m_bindingFn, m_data);
    }

    virtual void Execute(ExecuteContext& context) override
    {
        m_ExecuteCallback(m_executeFn, m_data);
    }

private:
    UserData                  m_data;
    SetupCallback             m_setupFn;
    AttachmentBindingCallback m_bindingFn;
    ExecuteCallback           m_executeFn;
};

} // namespace RHI