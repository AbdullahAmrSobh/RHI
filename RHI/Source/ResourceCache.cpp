#include "RHI/ResourceCache.hpp"
#include "RHI/Context.hpp"

namespace RHI
{
    ResourceCache::ResourceCache(Context* context)
        : m_context(context)
    {
    }

    ResourceCache::~ResourceCache()
    {
        for (auto [_, handle] : m_imageViewCache)
        {
            m_context->DestroyImageView(handle);
        }

        for (auto [_, handle] : m_bufferViewCache)
        {
            m_context->DestroyBufferView(handle);
        }
    }

    Handle<ImageView> ResourceCache::GetImageView(const ImageViewCreateInfo& createInfo)
    {
        if (auto it = m_imageViewCache.find(createInfo); it != m_imageViewCache.end())
        {
            return it->second;
        }

        return m_imageViewCache[createInfo] = m_context->CreateImageView(createInfo);
    }

    Handle<BufferView> ResourceCache::GetBufferView(const BufferViewCreateInfo& createInfo)
    {
        if (auto it = m_bufferViewCache.find(createInfo); it != m_bufferViewCache.end())
        {
            return it->second;
        }

        return m_bufferViewCache[createInfo] = m_context->CreateBufferView(createInfo);
    }

} // namespace RHI