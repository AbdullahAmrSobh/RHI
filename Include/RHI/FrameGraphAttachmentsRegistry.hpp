#pragma once
#include "RHI/FrameGraphAttachment.hpp"

namespace RHI
{

class IDevice;

class IAttachmentsRegistry
{
public:
    ~IAttachmentsRegistry() = default;

    Expected<ImageAttachmentReference> CreateTransientImageAttachment(const ImageFrameAttachmentDesc& description);

    Expected<BufferAttachmentReference> CreateTransientBufferAttachment(const BufferFrameAttachmentDesc& description);

    ImageAttachmentReference  FindImageReference(const std::string& name) const;
    
    BufferAttachmentReference FindBufferReference(const std::string& name) const;

    SwapchainAttachmentReference FindSwapchainReference(const std::string& name) const;

    SwapchainAttachmentReference ImportSwapchain(std::string name, Unique<ISwapchain>& swapchain);

    const std::vector<ImageFrameAttachment*> GetImageFrameAttachments() const;

    const std::vector<BufferFrameAttachment*> GetBufferFrameAttachments() const;

    ImageFrameAttachment* GetImageFrameAttachment(ImageAttachmentReference reference);

    BufferFrameAttachment* GetBufferFrameAttachment(BufferAttachmentReference reference);


private:
    friend class IFrameGraph;

    std::unordered_map<std::string, ImageAttachmentReference> m_imageAttachmentReferences;

    std::unordered_map<std::string, BufferAttachmentReference> m_bufferAttachmentReferences;

    std::vector<Unique<ImageFrameAttachment>> m_imageAttachments;

    std::vector<Unique<BufferFrameAttachment>> m_bufferAttachments;

    std::vector<ISwapchain*> m_swapchains; 

    std::vector<Unique<SwapchainFrameAttachment>> m_swapchainAttachments;
};
} // namespace RHI