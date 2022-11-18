#pragma once
#include <vector>
#include "RHI/Common.hpp"
#include "RHI/FrameGraphAttachment.hpp"
#include "RHI/FrameGraphPass.hpp"
#include "RHI/Resource.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{

class FrameGraphBuilder;
class FrameGraphContext;
class IPass;
class ICommandBuffer;
class IFrameGraph;
class IPassProducer;

class ITransientAttachmentsAllocator
{
public:
    virtual ~ITransientAttachmentsAllocator() = default;

    virtual Expected<Unique<ImageFrameAttachment>> AllocateImageFrameAttachment(const ImageFrameAttachmentDesc& attachmentDesc) = 0;

    virtual Expected<Unique<BufferFrameAttachment>> AllocatBufferFrameAttachment(const BufferFrameAttachmentDesc& attachmentDesc) = 0;
};

class IAttachmentStorage
{
public:
    virtual ~IAttachmentStorage() = default;

    virtual Expected<Unique<ImagePassAttachment>> CreateImagePassAttachment() = 0;

    virtual Expected<Unique<BufferPassAttachment>> CreateBufferPassAttachment() = 0;
};

class IFrameGraph
{
public:
    virtual ~IFrameGraph() = default;

    virtual EResultCode BeginFrame();
    virtual EResultCode EndFrame();
    virtual EResultCode SubmitPass(IPassProducer& producer);

    virtual Expected<ImageAttachmentReference> ImportSwapchain(Unique<ISwapchain> swapchain) = 0;

    EResultCode RegisterPass(IPassProducer& producer);

protected:
    virtual Expected<Unique<IPass>> CreatePass(std::string name, EPassType type) = 0;

protected:
    Unique<ITransientAttachmentsAllocator> m_allocator;
    std::vector<IPassProducer*>            m_producers;
    std::vector<Unique<ISwapchain>>        m_swapchains;
};

} // namespace RHI