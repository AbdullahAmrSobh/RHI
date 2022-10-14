#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraphAttachment.hpp"
#include "RHI/FrameGraphPass.hpp"

namespace RHI
{

class ISwapchain;

enum class EHardwareQueueType
{
    Graphics,
    Compute,
    Transfer
};

class IAttachmentsRegistry
{
public:
    virtual ImageAttachmentReference  ImportSwapchain(std::string name, ISwapchain& swapchain)       = 0;
    virtual ImageAttachmentReference  ImportImageResource(std::string name, Unique<IImage>& image)   = 0;
    virtual BufferAttachmentReference ImportBufferResource(std::string name, Unique<IImage>& buffer) = 0;

    virtual ImageAttachmentReference  CreateTransientImageAttachment(const ImageFrameAttachmentDesc& attachmentDesc)   = 0;
    virtual BufferAttachmentReference CreateTransientBufferAttachment(const BufferFrameAttachmentDesc& attachmentDesc) = 0;

    Expected<ImageAttachmentReference>  GetImageAttachmentReference(std::string_view name) const;
    Expected<BufferAttachmentReference> GetBufferAttachmentReference(std::string_view name) const;

    ImageFrameAttachment*  GetIamgeFrameAttachment(ImageAttachmentReference reference);
    BufferFrameAttachment* GetBufferFrameAttachment(BufferAttachmentReference reference);

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
    virtual Expected<Unique<IPass>> CreatePass(std::string name, EHardwareQueueType queueType) = 0;

    virtual void Submit(const IPass& pass) = 0;

    IAttachmentsRegistry&       GetAttachmentsRegistry();
    const IAttachmentsRegistry& GetAttachmentsRegistry() const;
    
    EResultCode BeginFrame();
    EResultCode EndFrame();

protected:
    virtual EResultCode BeginFrameInternal() = 0;
    virtual EResultCode EndFrameInternal() = 0;


protected:
    std::vector<IPass*>          m_pPasses;
    Unique<IAttachmentsRegistry> m_attachmentsRegistry;
    uint32_t                     m_currentFrameNumber;
};

class FrameGraphBuilder
{
public:
    EResultCode UseImageAttachment(ImagePassAttachmentDesc attachmentDesc, EAttachmentUsage usage, EAttachmentAccess access);
    EResultCode UseBufferAttachment(BufferPassAttachmentDesc attachmentDesc, EAttachmentUsage usage, EAttachmentAccess access);
    EResultCode SignalFence(Unique<IFence> SignalFence);
    EResultCode ExecuteAfter(const IPass& pass);
    EResultCode ExecuteBefore(const IPass& pass);

private:
    IFrameGraph* m_pFrameGraph;
};

} // namespace RHI