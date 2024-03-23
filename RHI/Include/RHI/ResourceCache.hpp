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
        std::unordered_map<ImageViewCreateInfo, Handle<ImageView>>   m_imageViewCache;
        std::unordered_map<BufferViewCreateInfo, Handle<BufferView>> m_bufferViewCache;
    };
} // namespace RHI