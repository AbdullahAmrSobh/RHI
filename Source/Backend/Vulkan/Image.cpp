#include "RHI/Pch.hpp"
#include "Backend/Vulkan/Common.hpp"

#include "Backend/Vulkan/Image.hpp"

#include "Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{

Expected<Unique<IImage>> Device::CreateImage(const AllocationDesc& allocationDesc, const ImageDesc& desc)
{
    Unique<Image> image  = CreateUnique<Image>(*this);
    VkResult      result = image->Init(allocationDesc, desc);

    if (Utils::IsSuccess(result))
        return std::move(image);

    return Unexpected(ConvertResult(result));
}

Expected<Unique<IImageView>> Device::CreateImageView(const IImage& image, const ImageViewDesc& desc)
{
    Unique<ImageView> imageView = CreateUnique<ImageView>(*this);
    VkResult          result    = imageView->Init(static_cast<const Image&>(image), desc);

    if (Utils::IsSuccess(result))
        return std::move(imageView);

    return Unexpected(ConvertResult(result));
}

Image::~Image()
{
    if (m_handle)
    {
        vmaDestroyImage(m_device->GetAllocator(), m_handle, m_allocation);
    }
}

VkResult Image::Init(const AllocationDesc& allocationDesc, const ImageDesc& desc)
{
    *m_desc = desc;

    VkImageCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    if (desc.extent.sizeY == 1 && desc.extent.sizeZ == 1)
    {
        createInfo.imageType = VK_IMAGE_TYPE_1D;
    }
    else if (desc.extent.sizeY == 1)
    {
        createInfo.imageType = VK_IMAGE_TYPE_2D;
    }
    else
    {
        createInfo.imageType = VK_IMAGE_TYPE_3D;
    }
    createInfo.format                = ConvertFormat(desc.format);
    createInfo.extent                = ConvertExtent(desc.extent);
    createInfo.mipLevels             = desc.mipLevelsCount;
    createInfo.arrayLayers           = desc.arraySize;
    createInfo.samples               = ConvertSampleCount(desc.sampleCount);
    createInfo.tiling                = VK_IMAGE_TILING_LINEAR;
    createInfo.usage                 = ConvertImageUsage(desc.usage);
    createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices   = nullptr;
    createInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage                   = ConvertMemoryUsage(allocationDesc.usage);

    VkResult result =
        vmaCreateImage(m_device->GetAllocator(), &createInfo, &allocationCreateInfo, &m_handle, &m_allocation, &m_allocationInfo);

    if (Utils::IsSuccess(result))
    {
        m_memorySize = m_allocationInfo.size;
    }

    return result;
}

ResultCode Image::SetDataInternal(size_t byteOffset, const uint8_t* bufferData, size_t bufferDataByteSize)
{
    uint8_t*    data   = nullptr;
    VkResult result = vmaMapMemory(m_device->GetAllocator(), m_allocation, reinterpret_cast<void**>(&data));
    if (Utils::IsError(result))
    {
        return ConvertResult(result);
    }

    std::memcpy(data + byteOffset, bufferData, bufferDataByteSize);

    vmaUnmapMemory(m_device->GetAllocator(), m_allocation);

    return ResultCode::Success;
}

ImageView::~ImageView()
{
    if (m_handle)
    {
        vkDestroyImageView(m_device->GetHandle(), m_handle, nullptr);
    }
}

VkResult ImageView::Init(const Image& image, const ImageViewDesc& desc)
{
    *m_desc = desc;

    VkImageViewCreateInfo createInfo {};
    createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.pNext                           = nullptr;
    createInfo.flags                           = 0;
    createInfo.image                           = image.GetHandle();
    createInfo.viewType                        = ConvertImageViewType(desc.type);
    createInfo.format                          = ConvertFormat(desc.format);
    createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask     = ConvertViewAspect(desc.viewAspect);
    createInfo.subresourceRange.baseMipLevel   = desc.range.baseMipLevel;
    createInfo.subresourceRange.levelCount     = desc.range.mipLevelsCount;
    createInfo.subresourceRange.baseArrayLayer = desc.range.baseArrayElement;
    createInfo.subresourceRange.layerCount     = desc.range.arraySize;

    return vkCreateImageView(m_device->GetHandle(), &createInfo, nullptr, &m_handle);
}

}  // namespace Vulkan
}  // namespace RHI