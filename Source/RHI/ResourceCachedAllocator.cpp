#include "RHI/Pch.hpp"

#include "RHI/Common.hpp"

#include "RHI/ResourceCachedAllocator.hpp"

#include "RHI/Device.hpp"

namespace RHI
{

std::shared_ptr<IImage> ResourceCachedAllocator::GetImage(const ImageDesc& desc)
{
    size_t         key   = HashDescriptor(desc);
    std::shared_ptr<IImage> image = m_imagesCache.Find(key);
    if (image)
    {
        return image;
    }

    image = m_device->CreateImage(m_allocationDesc, desc).value();
    m_imagesCache.Insert(key, image);
    return image;
}

std::shared_ptr<IBuffer> ResourceCachedAllocator::GetBuffer(const BufferDesc& desc)
{
    size_t          key    = HashDescriptor(desc);
    std::shared_ptr<IBuffer> buffer = m_buffersCache.Find(key);
    if (buffer)
    {
        return buffer;
    }

    buffer = m_device->CreateBuffer(m_allocationDesc, desc).value();
    m_buffersCache.Insert(key, buffer);
    return buffer;
}

std::shared_ptr<IImageView> ResourceCachedAllocator::GetImageView(const IImage& image, const ImageViewDesc& desc)
{
    size_t             key       = HashDescriptor(desc, std::bit_cast<size_t>(&image));
    std::shared_ptr<IImageView> imageView = m_imageViewsCache.Find(key);
    if (imageView)
    {
        return imageView;
    }

    imageView = m_device->CreateImageView(image, desc).value();
    m_imageViewsCache.Insert(key, imageView);
    return imageView;
}

std::shared_ptr<IBufferView> ResourceCachedAllocator::GetBufferView(const IBuffer& buffer, const BufferViewDesc& desc)
{
    size_t              key        = HashDescriptor(desc, std::bit_cast<size_t>(&buffer));
    std::shared_ptr<IBufferView> bufferView = m_bufferViewsCache.Find(key);
    if (bufferView)
    {
        return bufferView;
    }

    bufferView = m_device->CreateBufferView(buffer, desc).value();
    m_bufferViewsCache.Insert(key, bufferView);
    return bufferView;
}

}  // namespace RHI