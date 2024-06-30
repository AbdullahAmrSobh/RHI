
#include "RHI/RGInternals.hpp"
#include "RHI/RenderGraph.hpp"
#include "RHI/Context.hpp"

namespace RHI
{
    TransientAliasingAllocator::TransientAliasingAllocator(RenderGraph* renderGraph)
        : m_renderGraph(renderGraph)
    {
    }

    TransientAliasingAllocator::~TransientAliasingAllocator() = default;

    void TransientAliasingAllocator::Begin()
    {
    }

    void TransientAliasingAllocator::End()
    {
    }

    void TransientAliasingAllocator::BeginLifetime([[maybe_unused]] Handle<ImageAttachment> attachmentHandle)
    {
    }

    void TransientAliasingAllocator::BeginLifetime([[maybe_unused]] Handle<BufferAttachment> attachmentHandle)
    {
    }

    void TransientAliasingAllocator::EndLifetime([[maybe_unused]] Handle<ImageAttachment> attachmentHandle)
    {
    }

    void TransientAliasingAllocator::EndLifetime([[maybe_unused]] Handle<BufferAttachment> attachmentHandle)
    {
    }

    void TransientAliasingAllocator::Allocate(Handle<ImageAttachment> attachmentHandle)
    {
        auto context = m_renderGraph->m_context;
        auto attachment = m_renderGraph->m_imageAttachmentPool[attachmentHandle];
        auto frameContext = m_renderGraph->m_frameContext.get();
        attachment->m_resource = frameContext->CreateImage(*context, attachment->m_description);
    }

    void TransientAliasingAllocator::Allocate(Handle<BufferAttachment> attachmentHandle)
    {
        auto context = m_renderGraph->m_context;
        auto attachment = m_renderGraph->m_bufferAttachmentPool[attachmentHandle];
        auto frameContext = m_renderGraph->m_frameContext.get();
        attachment->m_resource = frameContext->CreateBuffer(*context, attachment->m_description);
    }

    void TransientAliasingAllocator::Release(Handle<ImageAttachment> attachmentHandle)
    {
        auto context = m_renderGraph->m_context;
        auto attachment = m_renderGraph->m_imageAttachmentPool[attachmentHandle];
        auto frameContext = m_renderGraph->m_frameContext.get();
        frameContext->DestroyImage(*context, attachment->m_resource);
    }

    void TransientAliasingAllocator::Release(Handle<BufferAttachment> attachmentHandle)
    {
        auto context = m_renderGraph->m_context;
        auto attachment = m_renderGraph->m_bufferAttachmentPool[attachmentHandle];
        auto frameContext = m_renderGraph->m_frameContext.get();
        frameContext->DestroyBuffer(*context, attachment->m_resource);
    }

    // Frame Context

    template<typename Type>
    inline static Handle<Type> AdvanceQueue(TL::Vector<Handle<Type>>& ring, uint64_t index)
    {
        RHI_ASSERT(ring.empty() == false);
        return ring[index = index % ring.size()];
    }

    Handle<Image> FrameContext::GetImage(RGImageID id) const
    {
        return *m_images[id];
    }

    Handle<Buffer> FrameContext::GetBuffer(RGBufferID id) const
    {
        return *m_buffers[id];
    }

    Handle<ImageView> FrameContext::GetImageView(RGImageViewID id) const
    {
        return *m_imageViews[id];
    }

    Handle<BufferView> FrameContext::GetBufferView(RGBufferViewID id) const
    {
        return *m_bufferViews[id];
    }

    RGImageID FrameContext::AddSwapchain([[maybe_unused]] Swapchain& swapchain)
    {
        return {};
    }

    RGImageID FrameContext::AddImage([[maybe_unused]] Handle<Image> image)
    {
        return {};
    }

    RGBufferID FrameContext::AddBuffer([[maybe_unused]] Handle<Buffer> buffer)
    {
        return {};
    }

    RGImageID FrameContext::CreateImage([[maybe_unused]] Context& context, [[maybe_unused]] const ImageCreateInfo& createInfo)
    {
        return {};
    }

    void FrameContext::DestroyImage([[maybe_unused]] Context& context, [[maybe_unused]] RGImageID id)
    {
    }

    RGBufferID FrameContext::CreateBuffer([[maybe_unused]] Context& context, [[maybe_unused]] const BufferCreateInfo& createInfo)
    {
        return {};
    }

    void FrameContext::DestroyBuffer([[maybe_unused]] Context& context, [[maybe_unused]] RGBufferID id)
    {
    }

    RGImageViewID FrameContext::CreateImageView([[maybe_unused]] Context& context, [[maybe_unused]] const ImageViewCreateInfo& createInfo, [[maybe_unused]] Swapchain* swapchain)
    {
        return {};
    }

    void FrameContext::DestroyImageView([[maybe_unused]] Context& context, [[maybe_unused]] RGImageViewID id)
    {
    }

    RGBufferViewID FrameContext::CreateBufferView([[maybe_unused]] Context& context, [[maybe_unused]] const BufferViewCreateInfo& createInfo)
    {
        return {};
    }

    void FrameContext::DestroyBufferView([[maybe_unused]] Context& context, [[maybe_unused]] RGBufferViewID id)
    {
    }

    void FrameContext::AdvanceFrame(uint64_t frameIndex)
    {
        for (auto [id, ring] : m_imageRingBuffer)
        {
            *m_images[id] = AdvanceQueue(ring, frameIndex);
        }

        for (auto [id, ring] : m_bufferRingBuffer)
        {
            *m_buffers[id] = AdvanceQueue(ring, frameIndex);
        }

        for (auto [id, ring] : m_imageViewRingBuffer)
        {
            *m_imageViews[id] = AdvanceQueue(ring, frameIndex);
        }

        for (auto [id, ring] : m_bufferViewRingBuffer)
        {
            *m_bufferViews[id] = AdvanceQueue(ring, frameIndex);
        }
    }
} // namespace RHI::RG