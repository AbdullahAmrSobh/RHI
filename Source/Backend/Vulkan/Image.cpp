#include "RHI/Pch.hpp"

#include "Backend/Vulkan/Common.hpp"

#include "Backend/Vulkan/Image.hpp"

#include "Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{

VkSampleCountFlagBits ConvertSampleCount(SampleCount sampleCount)
{
    return std::bit_cast<VkSampleCountFlagBits>(sampleCount);
}

VkImageType ConvertImageType(ImageType imageType)
{
    switch (imageType)
    {
        case ImageType::Image1D: return VK_IMAGE_TYPE_1D;
        case ImageType::Image2D: return VK_IMAGE_TYPE_2D;
        case ImageType::Image3D: return VK_IMAGE_TYPE_3D;
        default: assert(false); return VK_IMAGE_TYPE_MAX_ENUM;
    }
}

VkImageUsageFlags ConvertImageUsage(ImageUsageFlags usageFlags)
{
    VkImageUsageFlags flags = 0;
    if (usageFlags & ImageUsageFlagBits::Color)
    {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (usageFlags & ImageUsageFlagBits::DepthStencil)
    {
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (usageFlags & ImageUsageFlagBits::ShaderInput)
    {
        flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    if (usageFlags & ImageUsageFlagBits::Transfer)
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    return flags;
}

VkImageViewType ConvertImageViewType(ImageViewType imageType)
{
    VkImageViewType views[] = {VK_IMAGE_VIEW_TYPE_1D,
                               VK_IMAGE_VIEW_TYPE_1D_ARRAY,
                               VK_IMAGE_VIEW_TYPE_2D,
                               VK_IMAGE_VIEW_TYPE_2D_ARRAY,
                               VK_IMAGE_VIEW_TYPE_3D,
                               VK_IMAGE_VIEW_TYPE_CUBE,
                               VK_IMAGE_VIEW_TYPE_CUBE_ARRAY};

    return views[static_cast<uint32_t>(imageType)];
}

VkImageAspectFlags ConvertViewAspect(ImageViewAspectFlags aspectFlags)
{
    VkImageAspectFlags flags {};
    if (aspectFlags & ImageViewAspectFlagBits::Color)
    {
        flags |= VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (aspectFlags & ImageViewAspectFlagBits::Depth)
    {
        flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    if (aspectFlags & ImageViewAspectFlagBits::Stencil)
    {
        flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    return flags;
}

Expected<std::unique_ptr<IImage>> Device::CreateImage(const AllocationDesc& allocationDesc, const ImageDesc& desc)
{
    std::unique_ptr<Image> image  = std::make_unique<Image>(*this);
    VkResult      result = image->Init(allocationDesc, desc);

    if (Utils::IsSuccess(result))
        return std::move(image);

    return Unexpected(ConvertResult(result));
}

Expected<std::unique_ptr<IImageView>> Device::CreateImageView(const IImage& image, const ImageViewDesc& desc)
{
    std::unique_ptr<ImageView> imageView = std::make_unique<ImageView>(*this);
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
    createInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.pNext                 = nullptr;
    createInfo.flags                 = 0;
    createInfo.imageType             = ConvertImageType(desc.imageType);
    createInfo.format                = ConvertFormat(desc.format);
    createInfo.extent                = ConvertExtent(desc.extent);
    createInfo.mipLevels             = desc.mipLevelsCount;
    createInfo.arrayLayers           = desc.arraySize;
    createInfo.samples               = ConvertSampleCount(desc.sampleCount);
    createInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage                 = ConvertImageUsage(desc.usage);
    createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices   = nullptr;
    createInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage                   = ConvertMemoryUsage(allocationDesc.usage);
    
    VkResult result =
        vmaCreateImage(m_device->GetAllocator(), &createInfo, &allocationCreateInfo, &m_handle, &m_allocation, &m_allocationInfo);

    m_memorySize = m_allocationInfo.size;

    return result;
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