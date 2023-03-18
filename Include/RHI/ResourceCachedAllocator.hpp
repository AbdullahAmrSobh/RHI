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

    std::shared_ptr<IImage> GetImage(const ImageDesc& desc);

    std::shared_ptr<IBuffer> GetBuffer(const BufferDesc& desc);

    std::shared_ptr<IImageView> GetImageView(const IImage& image, const ImageViewDesc& desc);

    std::shared_ptr<IBufferView> GetBufferView(const IBuffer& buffer, const BufferViewDesc& desc);

private:
    IDevice* m_device;

    AllocationDesc m_allocationDesc;

    ObjectCache<IImage> m_imagesCache;

    ObjectCache<IBuffer> m_buffersCache;

    ObjectCache<IImageView> m_imageViewsCache;

    ObjectCache<IBufferView> m_bufferViewsCache;
};

}  // namespace RHI