#pragma once
#include "RHI/Buffer.hpp"
#include "RHI/Image.hpp"
#include "RHI/ObjectCache.hpp"

namespace RHI
{

class IDevice;

class ResourceCachedAllocator
{
public:
    ResourceCachedAllocator(IDevice& device)
        : m_device(&device)
    {
    }

    Shared<IImage> GetImage(const ImageDesc& desc);

    Shared<IBuffer> GetBuffer(const BufferDesc& desc);

    Shared<IImageView> GetImageView(const IImage& image, const ImageViewDesc& desc);

    Shared<IBufferView> GetBufferView(const IBuffer& buffer, const BufferViewDesc& desc);

private:
    IDevice* m_device;

    AllocationDesc m_allocationDesc;

    ObjectCache<IImage> m_imagesCache;

    ObjectCache<IBuffer> m_buffersCache;

    ObjectCache<IImageView> m_imageViewsCache;

    ObjectCache<IBufferView> m_bufferViewsCache;
};

}  // namespace RHI