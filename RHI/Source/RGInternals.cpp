#include "RHI/RGInternals.hpp"
#include "RHI/Context.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{
    Handle<Image> RGResourcePool::GetImage(RGImageID id) const
    {
        return SelectCurrentResource(m_images, id);
    }

    Handle<Buffer> RGResourcePool::GetBuffer(RGBufferID id) const
    {
        return SelectCurrentResource(m_buffers, id);
    }

    Handle<ImageView> RGResourcePool::GetImageView(RGImageViewID id) const
    {
        return SelectCurrentResource<ImageView>(m_imageViews, id);
    }

    Handle<BufferView> RGResourcePool::GetBufferView(RGBufferViewID id) const
    {
        return SelectCurrentResource<BufferView>(m_bufferViews, id);
    }

    RGImageID RGResourcePool::AddSwapchain(Swapchain& swapchain)
    {
        Resource<Image> resource{};
        resource.count = swapchain.GetImagesCount();
        for (uint32_t i = 0; i < resource.count; ++i)
        {
            // TODO: ensure that image inserted to ring starts at 0
            resource.resource[i] = swapchain.GetImage(i);
        }
        return m_images.Emplace(std::move(resource));
    }

    RGImageID RGResourcePool::AddImage(Handle<Image> image)
    {
        return m_images.Emplace({ 1, { image } });
    }

    RGBufferID RGResourcePool::AddBuffer(Handle<Buffer> buffer)
    {
        return m_buffers.Emplace({ 1, { buffer } });
    }

    RGImageID RGResourcePool::CreateImage(Context& context, const ImageCreateInfo& createInfo)
    {
        auto image = context.CreateImage(createInfo).GetValue();
        return m_images.Emplace({ 1, { image } });
    }

    void RGResourcePool::DestroyImage(Context& context, RGImageID id)
    {
        auto resource = m_images.Get(id);
        for (auto image : resource->resource)
        {
            if (image)
            {
                context.DestroyImage(image);
            }
        }
    }

    RGBufferID RGResourcePool::CreateBuffer(Context& context, const BufferCreateInfo& createInfo)
    {
        auto buffer = context.CreateBuffer(createInfo).GetValue();
        return m_buffers.Emplace({ 1, { buffer } });
    }

    void RGResourcePool::DestroyBuffer(Context& context, RGBufferID id)
    {
        auto resource = m_buffers.Get(id);
        for (auto buffer : resource->resource)
        {
            if (buffer)
            {
                context.DestroyBuffer(buffer);
            }
        }
    }

    RGImageViewID RGResourcePool::CreateImageView(Context& context, RGImageID id, const ImageViewCreateInfo& createInfo)
    {
        Resource<ImageView> rgView{};

        auto image = m_images.Get(id);

        if (image->count != 1)
        {
            rgView.count = image->count;
            ImageViewCreateInfo viewCI = createInfo;
            for (uint32_t i = 0; i < image->count; ++i)
            {
                viewCI.image = image->resource[i];
                rgView.resource[i] = context.CreateImageView(viewCI);
            }
            return m_imageViews.Emplace(std::move(rgView));
        }
        else
        {
            return m_imageViews.Emplace({ 1, { context.CreateImageView(createInfo) } });
        }
    }

    void RGResourcePool::DestroyImageView(Context& context, RGImageViewID id)
    {
        auto resource = m_imageViews.Get(id);
        for (auto view : resource->resource)
        {
            if (view)
            {
                context.DestroyImageView(view);
            }
        }
    }

    RGBufferViewID RGResourcePool::CreateBufferView(Context& context, RGBufferID id, const BufferViewCreateInfo& createInfo)
    {
        (void)id;
        return m_bufferViews.Emplace({ 1, { context.CreateBufferView(createInfo) } });
    }

    void RGResourcePool::DestroyBufferView(Context& context, RGBufferViewID id)
    {
        auto resource = m_bufferViews.Get(id);
        for (auto view : resource->resource)
        {
            if (view)
            {
                context.DestroyBufferView(view);
            }
        }
    }

    void RGResourcePool::AdvanceFrame()
    {
        m_head++;
    }

}; // namespace RHI