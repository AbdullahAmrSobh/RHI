#include "Resources.hpp"

#include <vk_mem_alloc.h>

#include "Common.hpp"
#include "Device.hpp"

namespace RHI::Vulkan
{
    // TODO: Sort functions in this file

    VkBufferUsageFlags ConvertBufferUsageFlags(TL::Flags<BufferUsage> bufferUsageFlags)
    {
        VkBufferUsageFlags result = 0;
        if (bufferUsageFlags & BufferUsage::Storage) result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Uniform) result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Vertex) result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Index) result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::CopySrc) result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if (bufferUsageFlags & BufferUsage::CopyDst) result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (bufferUsageFlags & BufferUsage::Indirect) result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        return result;
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

    VkImageViewType ConvertImageViewType(ImageViewType imageType)
    {
        switch (imageType)
        {
        case ImageViewType::View1D:      return VK_IMAGE_VIEW_TYPE_1D;
        case ImageViewType::View1DArray: return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        case ImageViewType::View2D:      return VK_IMAGE_VIEW_TYPE_2D;
        case ImageViewType::View2DArray: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case ImageViewType::View3D:      return VK_IMAGE_VIEW_TYPE_3D;
        case ImageViewType::CubeMap:     return VK_IMAGE_VIEW_TYPE_CUBE;
        case ImageViewType::None:
        default:
            return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
        }
    }

    VkImageAspectFlags ConvertImageAspect(TL::Flags<ImageAspect> imageAspect)
    {
        VkImageAspectFlags vkAspectFlags = 0;

        if (imageAspect & ImageAspect::Color) vkAspectFlags |= VK_IMAGE_ASPECT_COLOR_BIT;
        if (imageAspect & ImageAspect::Depth) vkAspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
        if (imageAspect & ImageAspect::Stencil) vkAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;

        // Validate for combined flags or specific cases
        if (vkAspectFlags == 0)
        {
            if (imageAspect == ImageAspect::All)
                return VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            if (imageAspect == ImageAspect::DepthStencil)
                return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

            TL_UNREACHABLE(); // This handles invalid input
        }

        return vkAspectFlags;
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

    VkFilter ConvertFilter(SamplerFilter samplerFilter)
    {
        switch (samplerFilter)
        {
        case SamplerFilter::Point:  return VK_FILTER_NEAREST;
        case SamplerFilter::Linear: return VK_FILTER_LINEAR;
        default:                    TL_UNREACHABLE(); return VK_FILTER_MAX_ENUM;
        }
    }

    VkSamplerAddressMode ConvertSamplerAddressMode(SamplerAddressMode addressMode)
    {
        switch (addressMode)
        {
        case SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddressMode::Clamp:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        default:                         TL_UNREACHABLE(); return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
        }
    }

    VkCompareOp ConvertCompareOp(CompareOperator compareOperator)
    {
        switch (compareOperator)
        {
        case CompareOperator::Never:          return VK_COMPARE_OP_NEVER;
        case CompareOperator::Equal:          return VK_COMPARE_OP_EQUAL;
        case CompareOperator::NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOperator::Greater:        return VK_COMPARE_OP_GREATER;
        case CompareOperator::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOperator::Less:           return VK_COMPARE_OP_LESS;
        case CompareOperator::LessOrEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOperator::Always:         return VK_COMPARE_OP_ALWAYS;
        default:                              TL_UNREACHABLE(); return VK_COMPARE_OP_MAX_ENUM;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Sampler
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IBuffer::Init(IDevice* device, const BufferCreateInfo& createInfo)
    {
        VmaAllocationCreateInfo allocationCI =
            {
                .flags          = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
                .usage          = createInfo.hostMapped ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                .requiredFlags  = 0,
                .preferredFlags = 0,
                .memoryTypeBits = 0,
                .pool           = VK_NULL_HANDLE,
                .pUserData      = nullptr,
                .priority       = 0.0f,
            };
        VkBufferCreateInfo bufferCI =
            {
                .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .pNext                 = nullptr,
                .flags                 = 0,
                .size                  = createInfo.byteSize,
                .usage                 = ConvertBufferUsageFlags(createInfo.usageFlags),
                .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 0,
                .pQueueFamilyIndices   = nullptr,
            };
        auto result = vmaCreateBuffer(device->m_deviceAllocator, &bufferCI, &allocationCI, &handle, &allocation, nullptr);
        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }
        return ConvertResult(result);
    }

    void IBuffer::Shutdown(IDevice* device)
    {
        vmaDestroyBuffer(device->m_deviceAllocator, handle, allocation);
    }

    VkMemoryRequirements IBuffer::GetMemoryRequirements(IDevice* device) const
    {
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(device->m_device, handle, &requirements);
        return requirements;
    }

    DeviceMemoryPtr IBuffer::Map(IDevice* device)
    {
        DeviceMemoryPtr ptr = nullptr;
        auto            res = vmaMapMemory(device->m_deviceAllocator, allocation, &ptr);
        Validate(res);
        return ptr;
    }

    void IBuffer::Unmap(IDevice* device)
    {
        vmaUnmapMemory(device->m_deviceAllocator, allocation);
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

        result = vmaCreateImage(device->m_deviceAllocator, &imageCI, &allocationInfo, &handle, &allocation, nullptr);

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
                        .aspectMask     = ConvertImageAspect(GetFormatAspects(createInfo.format)),
                        .baseMipLevel   = 0,
                        .levelCount     = VK_REMAINING_MIP_LEVELS,
                        .baseArrayLayer = 0,
                        .layerCount     = VK_REMAINING_ARRAY_LAYERS,
                        },
        };

        switch (imageCI.imageType)
        {
        case VK_IMAGE_TYPE_1D: imageViewCI.viewType = imageCI.arrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_1D : VK_IMAGE_VIEW_TYPE_1D_ARRAY; break;
        case VK_IMAGE_TYPE_2D: imageViewCI.viewType = imageCI.arrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY; break;
        case VK_IMAGE_TYPE_3D: imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
        default:               TL_UNREACHABLE(); break;
        }

        this->subresources = {
            .imageAspects = ImageAspect::Color,
        };

        result = vkCreateImageView(device->m_device, &imageViewCI, nullptr, &viewHandle);

        size = createInfo.size;

        return ConvertResult(result);
    }

    ResultCode IImage::Init(IDevice* device, VkImage image, const VkSwapchainCreateInfoKHR& swapchainCI)
    {
        this->subresources = {
            .imageAspects = ImageAspect::Color,
        };
        this->handle = image;

        VkImageViewCreateInfo imageViewCI{
            .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext      = nullptr,
            .flags      = 0,
            .image      = handle,
            .viewType   = VK_IMAGE_VIEW_TYPE_2D,
            .format     = swapchainCI.imageFormat,
            .components = {
                           VK_COMPONENT_SWIZZLE_IDENTITY,
                           VK_COMPONENT_SWIZZLE_IDENTITY,
                           VK_COMPONENT_SWIZZLE_IDENTITY,
                           VK_COMPONENT_SWIZZLE_IDENTITY,
                           },
            .subresourceRange = {
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
        vkDestroyImageView(device->m_device, viewHandle, nullptr);
        vmaDestroyImage(device->m_deviceAllocator, handle, allocation);
    }

    VkMemoryRequirements IImage::GetMemoryRequirements(IDevice* device) const
    {
        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(device->m_device, handle, &requirements);
        return requirements;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Sampler
    ///////////////////////////////////////////////////////////////////////////

    ResultCode ISampler::Init(IDevice* device, const SamplerCreateInfo& createInfo)
    {
        VkSamplerCreateInfo samplerCI{
            .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext                   = nullptr,
            .flags                   = 0,
            .magFilter               = ConvertFilter(createInfo.filterMag),
            .minFilter               = ConvertFilter(createInfo.filterMin),
            .mipmapMode              = createInfo.filterMip == SamplerFilter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU            = ConvertSamplerAddressMode(createInfo.addressU),
            .addressModeV            = ConvertSamplerAddressMode(createInfo.addressV),
            .addressModeW            = ConvertSamplerAddressMode(createInfo.addressW),
            .mipLodBias              = createInfo.mipLodBias,
            .anisotropyEnable        = VK_TRUE,
            .maxAnisotropy           = 1.0f,
            .compareEnable           = VK_TRUE,
            .compareOp               = ConvertCompareOp(createInfo.compare),
            .minLod                  = createInfo.minLod,
            .maxLod                  = createInfo.maxLod,
            .borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
            .unnormalizedCoordinates = VK_FALSE,
        };
        auto result = vkCreateSampler(device->m_device, &samplerCI, nullptr, &handle);
        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }
        return ConvertResult(result);
    }

    void ISampler::Shutdown(IDevice* device)
    {
        vkDestroySampler(device->m_device, handle, nullptr);
    }

} // namespace RHI::Vulkan