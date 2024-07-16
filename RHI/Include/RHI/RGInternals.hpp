#pragma once

#include "RHI/Resources.hpp"

namespace RHI
{
    class Context;
    class Swapchain;
    class RenderGraph;

    class RGResourcePool
    {
    public:
        template<typename T>
        struct Resource
        {
            uint32_t  count;
            Handle<T> resource[4]; // max count is 4 TODO: define as constexpr
        };

        using RGImageID      = Handle<Resource<Image>>;
        using RGBufferID     = Handle<Resource<Buffer>>;
        using RGImageViewID  = Handle<Resource<ImageView>>;
        using RGBufferViewID = Handle<Resource<BufferView>>;

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

        RGImageViewID CreateImageView(Context& context, RGImageID id, const ImageViewCreateInfo& createInfo);

        void DestroyImageView(Context& context, RGImageViewID id);

        RGBufferViewID CreateBufferView(Context& context, RGBufferID id, const BufferViewCreateInfo& createInfo);

        void DestroyBufferView(Context& context, RGBufferViewID id);

        void AdvanceFrame();

    private:

        uint64_t m_head;

        template<typename ResourceType>
        Handle<ResourceType> SelectCurrentResource(const HandlePool<Resource<ResourceType>>& pool, Handle<Resource<ResourceType>> resourceHandle) const
        {
            auto resource = pool.Get(resourceHandle);
            auto index = resource->count % m_head;
            return resource->resource[index];
        }

        // list of active handles to active images in the current frame
        HandlePool<Resource<Image>> m_images;

        // list of active handles to active buffers in the current frame#pragma once
        HandlePool<Resource<Buffer>> m_buffers;

        // list of active handles to active image views in the current frame
        HandlePool<Resource<ImageView>> m_imageViews;

        // list of active handles to active buffer views in the current frame
        HandlePool<Resource<BufferView>> m_bufferViews;
    };

    using RGImageID      = RGResourcePool::RGImageID;
    using RGBufferID     = RGResourcePool::RGBufferID;
    using RGImageViewID  = RGResourcePool::RGImageViewID;
    using RGBufferViewID = RGResourcePool::RGBufferViewID;

}; // namespace RHI