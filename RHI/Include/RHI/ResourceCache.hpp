#pragma once
#include "RHI/Resources.hpp"

namespace RHI
{
    class Context;

    class ResourceCache
    {
    public:
        ResourceCache(Context* context);
        ~ResourceCache();

        Handle<ImageView>  GetImageView(const ImageViewCreateInfo& createInfo);
        Handle<BufferView> GetBufferView(const BufferViewCreateInfo& createInfo);

    private:
        Context*                                                     m_context;
        TL::UnorderedMap<ImageViewCreateInfo, Handle<ImageView>>   m_imageViewCache;
        TL::UnorderedMap<BufferViewCreateInfo, Handle<BufferView>> m_bufferViewCache;
    };
} // namespace RHI