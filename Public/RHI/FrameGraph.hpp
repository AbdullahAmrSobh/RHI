#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/FrameGraphAttachment.hpp"
#include "RHI/RenderPass.hpp"
#include "RHI/RenderPassExecuter.hpp"

namespace RHI
{

class IAttachmentResourceAllocator
{
};

class IAttachmentResource
{
public:
};

class IBufferAttachmentResource
{
};

class IImageAttachmentResource
{
public:
};

class ISwapChainAttachmentResource
{
};

class IFrameGraph
{
public:
    virtual ~IFrameGraph() = default;

    virtual void CreateRenderPass(std::string name, ERenderPassQueueType queueType, RenderPassExecuter& executer) = 0;

    virtual BufferAttachmentId CreateBufferAttachment(const BufferAttachmentDesc& desc) = 0;

    virtual ImageAttachmentId CreateImageAttachment(const ImageAttachmentDesc& desc) = 0;

    virtual BufferAttachmentId ImportBufferAsAttachment(const BufferAttachmentDesc& desc, IBuffer& buffer) = 0;
    
    virtual ImageAttachmentId ImportImageAsAttachment(const ImageAttachmentDesc& desc, IImage& image) = 0;

    virtual ImageAttachmentId ImportSwapchainAsAttachment(const ImageAttachmentDesc& desc, const ISwapChain& swapchain) = 0;
};
using FrameGraphPtr = Unique<IFrameGraph>;

} // namespace RHI
