#pragma once

#include "RHI/Resources.hpp"

namespace RHI
{
    class Context;
    class Swapchain;
    class RenderGraph;
    class ImageAttachment;
    class BufferAttachment;

    class TransientAliasingAllocator
    {
    public:
        TransientAliasingAllocator(RenderGraph* renderGraph);
        virtual ~TransientAliasingAllocator();

        void Begin();
        void End();

        void BeginLifetime(Handle<ImageAttachment> attachment);
        void BeginLifetime(Handle<BufferAttachment> attachment);

        void EndLifetime(Handle<ImageAttachment> attachment);
        void EndLifetime(Handle<BufferAttachment> attachment);

        void Allocate(Handle<ImageAttachment> attachment);
        void Allocate(Handle<BufferAttachment> attachment);

        void Release(Handle<ImageAttachment> attachment);
        void Release(Handle<BufferAttachment> attachment);

    private:
        RenderGraph* m_renderGraph;
    };

    using RGImageID      = Handle<Handle<Image>>;
    using RGBufferID     = Handle<Handle<Buffer>>;
    using RGImageViewID  = Handle<Handle<ImageView>>;
    using RGBufferViewID = Handle<Handle<BufferView>>;

    class FrameContext
    {
    public:
        Handle<Image> GetImage(RGImageID id) const;

        Handle<Buffer> GetBuffer(RGBufferID id) const;

        Handle<ImageView> GetImageView(RGImageViewID id) const;

        Handle<BufferView> GetBufferView(RGBufferViewID id) const;

        RGImageID AddSwapchain(Swapchain& swapchain);

        RGImageID AddImage(Handle<Image> image);

        RGBufferID AddBuffer(Handle<Buffer> buffer);

        RGImageID CreateImage(Context& context, const ImageCreateInfo& createInfo);

        void DestroyImage(Context& context, RGImageID id);

        RGBufferID CreateBuffer(Context& context, const BufferCreateInfo& createInfo);

        void DestroyBuffer(Context& context, RGBufferID id);

        RGImageViewID CreateImageView(Context& context, const ImageViewCreateInfo& createInfo, Swapchain* swapchain = nullptr);

        void DestroyImageView(Context& context, RGImageViewID id);

        RGBufferViewID CreateBufferView(Context& context, const BufferViewCreateInfo& createInfo);

        void DestroyBufferView(Context& context, RGBufferViewID id);

        void AdvanceFrame(uint64_t frameIndex);

    private:
        template<typename Type>
        using RingBuffer = TL::UnorderedMap<Handle<Handle<Type>>, TL::Vector<Handle<Type>>>;

        // list of images which will be rotated at the end of the frame
        RingBuffer<Image> m_imageRingBuffer;

        // list of buffers which will be rotated at the end of the frame
        RingBuffer<Buffer> m_bufferRingBuffer;

        // list of image views which will be rotated at the end of the frame
        RingBuffer<ImageView> m_imageViewRingBuffer;

        // list of buffer views which will be rotated at the end of the frame
        RingBuffer<BufferView> m_bufferViewRingBuffer;

        // list of active handles to active images in the current frame
        HandlePool<Handle<Image>> m_images;

        // list of active handles to active buffers in the current frame#pragma once
        HandlePool<Handle<Buffer>> m_buffers;

        // list of active handles to active image views in the current frame
        HandlePool<Handle<ImageView>> m_imageViews;

        // list of active handles to active buffer views in the current frame
        HandlePool<Handle<BufferView>> m_bufferViews;
    };
}; // namespace RHI