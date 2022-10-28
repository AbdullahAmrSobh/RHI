#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraphAttachment.hpp"
#include "RHI/FrameGraphPass.hpp"

namespace RHI
{
class ISwapchain;
class IFrameGraph;

class IAttachmentsRegistry
{
public:
    virtual ~IAttachmentsRegistry() = default;

    virtual ImageAttachmentReference  ImportSwapchain(std::string name, ISwapchain& swapchain)       = 0;
    virtual ImageAttachmentReference  ImportImageResource(std::string name, Unique<IImage>& image)   = 0;
    virtual BufferAttachmentReference ImportBufferResource(std::string name, Unique<IImage>& buffer) = 0;

    virtual ImageAttachmentReference  CreateTransientImageAttachment(const ImageFrameAttachmentDesc& attachmentDesc)   = 0;
    virtual BufferAttachmentReference CreateTransientBufferAttachment(const BufferFrameAttachmentDesc& attachmentDesc) = 0;

    ImageFrameAttachment*               GetIamgeFrameAttachment(ImageAttachmentReference reference) const;
    Expected<ImageAttachmentReference>  GetImageAttachmentReference(std::string_view name) const;
    BufferFrameAttachment*              GetBufferFrameAttachment(BufferAttachmentReference reference) const;
    Expected<BufferAttachmentReference> GetBufferAttachmentReference(std::string_view name) const;

    std::vector<ImageFrameAttachment*>  GetTransientIamgeFrameAttachment();
    std::vector<BufferFrameAttachment*> GetTransientBufferFrameAttachment();

    virtual void Compile(IFrameGraph& frameGraph) = 0;

protected:
    std::vector<Unique<ImageFrameAttachment>>  m_imageFrameAttachments;
    std::vector<Unique<BufferFrameAttachment>> m_bufferFrameAttachments;
};

class IFrameGraph
{
public:
    virtual ~IFrameGraph() = default;

    virtual Expected<Unique<IPass>> CreatePass(std::string name, EHardwareQueueType queueType) = 0;

    virtual void Submit(const IPass& pass) = 0;

    IAttachmentsRegistry&       GetAttachmentsRegistry();
    const IAttachmentsRegistry& GetAttachmentsRegistry() const;

    EResultCode Compile(); 
    EResultCode Execute();

    EResultCode BeginFrame();
    EResultCode EndFrame();

protected:
    std::vector<IPass*>          m_pPasses;
    Unique<IAttachmentsRegistry> m_attachmentsRegistry;
    uint32_t                     m_currentFrameNumber;
};

class FrameGraphBuilder
{
public:
    FrameGraphBuilder(IFrameGraph* pFrameGraph);
    
    EResultCode UseImageAttachment(ImagePassAttachmentDesc attachmentDesc, EAttachmentUsage usage, EAttachmentAccess access);
    EResultCode UseBufferAttachment(BufferPassAttachmentDesc attachmentDesc, EAttachmentUsage usage, EAttachmentAccess access);
    EResultCode SignalFence(Unique<IFence> SignalFence);
    EResultCode ExecuteAfter(const IPass& pass);
    EResultCode ExecuteBefore(const IPass& pass);

private:
    IFrameGraph* m_pFrameGraph;
};

} // namespace RHI