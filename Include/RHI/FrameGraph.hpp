#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraphAttachment.hpp"
#include "RHI/FrameGraphPass.hpp"

namespace RHI
{

class ISwapchain;

class IAttachmentsRegistry
{
public:
    virtual ImageAttachmentReference ImportSwapchain(std::string name, ISwapchain& swapchain) = 0;

    virtual ImageAttachmentReference ImportImageResource(std::string name, Unique<IImage>& image) = 0;

    virtual BufferAttachmentReference ImportBufferResource(std::string name, Unique<IBuffer>& buffer) = 0;

    virtual ImageAttachmentReference CreateTransientImageAttachment(const ImageFrameAttachmentDesc& attachmentDesc) = 0;

    virtual BufferAttachmentReference CreateTransientBufferAttachment(const BufferFrameAttachmentDesc& attachmentDesc) = 0;

    virtual void Compile(IFrameGraph& frameGraph) = 0;

    Expected<ImageAttachmentReference> GetImageAttachmentReference(std::string_view name) const;

    Expected<BufferAttachmentReference> GetBufferAttachmentReference(std::string_view name) const;

    ImageFrameAttachment* GetIamgeFrameAttachment(ImageAttachmentReference reference);

    BufferFrameAttachment* GetBufferFrameAttachment(BufferAttachmentReference reference);

    std::vector<ImageFrameAttachment*> GetTransientIamgeFrameAttachment();

    std::vector<BufferFrameAttachment*> GetTransientBufferFrameAttachment();

protected:
    std::vector<Unique<ImageFrameAttachment>>  m_imageFrameAttachments;
    std::vector<Unique<BufferFrameAttachment>> m_bufferFrameAttachments;
};

class IFrameGraph
{
public:
    virtual ~IFrameGraph() = default;

    inline IAttachmentsRegistry&       GetAttachmentsRegistry();
    inline const IAttachmentsRegistry& GetAttachmentsRegistry() const;
    
    virtual EResultCode BeginFrame();
    virtual EResultCode EndFrame();

    virtual EResultCode Execute(const IPassCallbacks& pass);
};

class FrameGraphBuilder
{
public:
    IAttachmentsRegistry&       GetAttachmentsRegistry();
    const IAttachmentsRegistry& GetAttachmentsRegistry() const;
    
    void UseBufferAttachment(const BufferPassAttachmentDesc& attachmentDesc, EAttachmentUsage usage, EAttachmentAccess access);

    void UseBufferAttachments(const std::vector<BufferPassAttachmentDesc>& attachmentsDesc, EAttachmentUsage usage, EAttachmentAccess access);

    void UseImageAttachment(const ImagePassAttachmentDesc& attachmentDesc, EAttachmentUsage usage, EAttachmentAccess access);

    void UseImageAttachments(const std::vector<ImagePassAttachmentDesc>& attachmentsDesc, EAttachmentUsage usage, EAttachmentAccess access);

    void ExecuteAfter(PassId passId);

};

} // namespace RHI