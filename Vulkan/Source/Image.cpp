#include "Image.hpp"
#include "Device.hpp"
#include "Common.hpp"
#include "CommandList.hpp"

namespace RHI::Vulkan
{
    VkImageSubresource ConvertSubresource(const ImageSubresource& subresource)
    {
        auto vkSubresource       = VkImageSubresource{};
        vkSubresource.aspectMask = ConvertImageAspect(subresource.imageAspects);
        vkSubresource.mipLevel   = subresource.mipLevel;
        vkSubresource.arrayLayer = subresource.arrayLayer;
        return vkSubresource;
    }

    VkImageUsageFlagBits ConvertImageUsage(ImageUsage imageUsage)
    {
        switch (imageUsage)
        {
        case ImageUsage::None:            return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        case ImageUsage::ShaderResource:  return VK_IMAGE_USAGE_SAMPLED_BIT;
        case ImageUsage::StorageResource: return VK_IMAGE_USAGE_STORAGE_BIT;
        case ImageUsage::Color:           return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        case ImageUsage::Depth:           return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        case ImageUsage::Stencil:         return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        case ImageUsage::CopySrc:         return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        case ImageUsage::CopyDst:         return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        default:                          TL_UNREACHABLE(); return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    VkImageUsageFlags ConvertImageUsageFlags(TL::Flags<ImageUsage> imageUsageFlags)
    {
        VkImageUsageFlags result = 0;
        if (imageUsageFlags & ImageUsage::ShaderResource) result |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (imageUsageFlags & ImageUsage::StorageResource) result |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (imageUsageFlags & ImageUsage::Color) result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (imageUsageFlags & ImageUsage::Depth) result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (imageUsageFlags & ImageUsage::Stencil) result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (imageUsageFlags & ImageUsage::CopySrc) result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (imageUsageFlags & ImageUsage::CopyDst) result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        return result;
    }

    VkImageType ConvertImageType(ImageType imageType)
    {
        switch (imageType)
        {
        case ImageType::None:    return VK_IMAGE_TYPE_MAX_ENUM;
        case ImageType::Image1D: return VK_IMAGE_TYPE_1D;
        case ImageType::Image2D: return VK_IMAGE_TYPE_2D;
        case ImageType::Image3D: return VK_IMAGE_TYPE_3D;
        default:                 TL_UNREACHABLE(); return VK_IMAGE_TYPE_MAX_ENUM;
        }
    }

    VkImageAspectFlagBits ConvertImageAspect(TL::Flags<ImageAspect> imageAspect)
    {
        if (imageAspect & ImageAspect::Color) return VK_IMAGE_ASPECT_COLOR_BIT;
        if (imageAspect & ImageAspect::Depth) return VK_IMAGE_ASPECT_DEPTH_BIT;
        if (imageAspect & ImageAspect::Stencil) return VK_IMAGE_ASPECT_STENCIL_BIT;
        TL_UNREACHABLE();
        return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    }

    VkImageAspectFlags ConvertImageAspect(ImageAspect imageAspect)
    {
        switch (imageAspect)
        {
        case ImageAspect::None:         return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
        case ImageAspect::Color:        return VK_IMAGE_ASPECT_COLOR_BIT;
        case ImageAspect::Depth:        return VK_IMAGE_ASPECT_DEPTH_BIT;
        case ImageAspect::Stencil:      return VK_IMAGE_ASPECT_STENCIL_BIT;
        case ImageAspect::DepthStencil: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        case ImageAspect::All:          return VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:                        TL_UNREACHABLE(); return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
        }
    }

    VkImageAspectFlags FormatToAspect(VkFormat format)
    {
        if (format == VK_FORMAT_D32_SFLOAT) return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        else return VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkComponentSwizzle ConvertComponentSwizzle(ComponentSwizzle componentSwizzle)
    {
        switch (componentSwizzle)
        {
        case ComponentSwizzle::Identity: return VK_COMPONENT_SWIZZLE_IDENTITY;
        case ComponentSwizzle::Zero:     return VK_COMPONENT_SWIZZLE_ZERO;
        case ComponentSwizzle::One:      return VK_COMPONENT_SWIZZLE_ONE;
        case ComponentSwizzle::R:        return VK_COMPONENT_SWIZZLE_R;
        case ComponentSwizzle::G:        return VK_COMPONENT_SWIZZLE_G;
        case ComponentSwizzle::B:        return VK_COMPONENT_SWIZZLE_B;
        case ComponentSwizzle::A:        return VK_COMPONENT_SWIZZLE_A;
        default:                         TL_UNREACHABLE(); return VK_COMPONENT_SWIZZLE_IDENTITY;
        }
    }

    VkImageSubresourceRange ConvertSubresourceRange(const ImageSubresourceRange& subresource)
    {
        auto vkSubresource           = VkImageSubresourceRange{};
        vkSubresource.aspectMask     = ConvertImageAspect(subresource.imageAspects);
        vkSubresource.baseMipLevel   = subresource.mipBase;
        vkSubresource.levelCount     = subresource.mipLevelCount;
        vkSubresource.baseArrayLayer = subresource.arrayBase;
        vkSubresource.layerCount     = subresource.arrayCount;
        return vkSubresource;
    }

    VkExtent2D ConvertExtent2D(ImageSize2D size)
    {
        return {size.width, size.height};
    }

    VkExtent3D ConvertExtent3D(ImageSize3D size)
    {
        return {size.width, size.height, size.depth};
    }

    VkExtent2D ConvertExtent2D(ImageSize3D size)
    {
        TL_ASSERT(size.depth == 0);
        return {size.width, size.height};
    }

    VkOffset2D ConvertOffset2D(ImageOffset2D offset)
    {
        return {offset.x, offset.y};
    }

    VkOffset3D ConvertOffset3D(ImageOffset3D offset)
    {
        return {offset.x, offset.y, offset.z};
    }

    VkComponentMapping ConvertComponentMapping(ComponentMapping componentMapping)
    {
        VkComponentMapping mapping{};
        mapping.r = ConvertComponentSwizzle(componentMapping.r);
        mapping.g = ConvertComponentSwizzle(componentMapping.g);
        mapping.b = ConvertComponentSwizzle(componentMapping.b);
        mapping.a = ConvertComponentSwizzle(componentMapping.a);
        return mapping;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Image
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IImage::Init(IDevice* device, const ImageCreateInfo& createInfo)
    {
        VkResult result;

        VmaAllocationCreateInfo allocationInfo{
            .flags          = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
            .usage          = VMA_MEMORY_USAGE_GPU_ONLY,
            .requiredFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool           = VK_NULL_HANDLE,
            .pUserData      = nullptr,
            .priority       = 0.0f,
        };
        VkImageCreateInfo imageCI{
            .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = 0,
            .imageType             = ConvertImageType(createInfo.type),
            .format                = ConvertFormat(createInfo.format),
            .extent                = ConvertExtent3D(createInfo.size),
            .mipLevels             = createInfo.mipLevels,
            .arrayLayers           = createInfo.arrayCount,
            .samples               = ConvertSampleCount(createInfo.sampleCount),
            .tiling                = VK_IMAGE_TILING_OPTIMAL,
            .usage                 = ConvertImageUsageFlags(createInfo.usageFlags),
            .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = nullptr,
            .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        result = vmaCreateImage(device->m_allocator, &imageCI, &allocationInfo, &handle, &allocation.handle, &allocation.info);

        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }

        VkImageViewCreateInfo imageViewCI{
            .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext    = nullptr,
            .flags    = 0,
            .image    = handle,
            .viewType = VK_IMAGE_VIEW_TYPE_1D,
            .format   = imageCI.format,
            .components{
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange =
                {
                    .aspectMask     = 0,
                    .baseMipLevel   = 0,
                    .levelCount     = VK_REMAINING_MIP_LEVELS,
                    .baseArrayLayer = 0,
                    .layerCount     = VK_REMAINING_ARRAY_LAYERS,
                },
        };

        auto formatInfo = GetFormatInfo(createInfo.format);
        if (formatInfo.hasDepth) imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        if (formatInfo.hasStencil) imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        if (formatInfo.hasRed) imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;

        switch (imageCI.imageType)
        {
        case VK_IMAGE_TYPE_1D: imageViewCI.viewType = imageCI.arrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_1D : VK_IMAGE_VIEW_TYPE_1D_ARRAY; break;
        case VK_IMAGE_TYPE_2D: imageViewCI.viewType = imageCI.arrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY; break;
        case VK_IMAGE_TYPE_3D: imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
        default:               TL_UNREACHABLE(); break;
        }

        result = vkCreateImageView(device->m_device, &imageViewCI, nullptr, &viewHandle);

        return ConvertResult(result);
    }

    ResultCode IImage::Init(IDevice* device, VkImage image, const VkSwapchainCreateInfoKHR& swapchainCI)
    {
        this->handle = image;

        VkImageViewCreateInfo imageViewCI{
            .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext    = nullptr,
            .flags    = 0,
            .image    = handle,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format   = swapchainCI.imageFormat,
            .components{
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange =
                {
                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel   = 0,
                    .levelCount     = VK_REMAINING_MIP_LEVELS,
                    .baseArrayLayer = 0,
                    .layerCount     = VK_REMAINING_ARRAY_LAYERS,
                },
        };

        VkResult result = vkCreateImageView(device->m_device, &imageViewCI, nullptr, &viewHandle);

        return ConvertResult(result);
    }

    void IImage::Shutdown(IDevice* device)
    {
        // vmaDestroyImage(device->m_allocator, handle, allocation.handle);
        device->m_deleteQueue.DestroyObject(handle);
        device->m_deleteQueue.DestroyObject(viewHandle);
    }

    VkMemoryRequirements IImage::GetMemoryRequirements(IDevice* device) const
    {
        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(device->m_device, handle, &requirements);
        return requirements;
    }
} // namespace RHI::Vulkan