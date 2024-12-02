#pragma once

#include <RHI/Image.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

#include "MemoryAllocator.hpp"

namespace RHI::Vulkan
{
    class IDevice;

    VkImageSubresource ConvertSubresource(const ImageSubresource& subresource);

    VkImageUsageFlags ConvertImageUsageFlags(TL::Flags<ImageUsage> imageUsageFlags);

    VkImageType ConvertImageType(ImageType imageType);

    VkImageAspectFlags ConvertImageAspect(TL::Flags<ImageAspect> imageAspect);

    VkComponentSwizzle ConvertComponentSwizzle(ComponentSwizzle componentSwizzle);

    VkImageSubresourceRange ConvertSubresourceRange(const ImageSubresourceRange& subresource);

    VkComponentMapping ConvertComponentMapping(ComponentMapping componentMapping);

    VkExtent2D ConvertExtent2D(ImageSize2D size);

    VkExtent3D ConvertExtent3D(ImageSize3D size);

    VkExtent2D ConvertExtent2D(ImageSize3D size);

    VkOffset2D ConvertOffset2D(ImageOffset2D offset);

    VkOffset3D ConvertOffset3D(ImageOffset3D offset);

    struct IImage : Image
    {
        DeviceAllocation      allocation;
        VkImage               handle;
        VkImageView           viewHandle;
        ImageSize3D           size;
        ImageSubresourceRange subresources;

        ResultCode Init(IDevice* device, const ImageCreateInfo& createInfo);
        ResultCode Init(IDevice* device, VkImage image, const VkSwapchainCreateInfoKHR& swapchainCreateInfo);
        void       Shutdown(IDevice* device);

        VkMemoryRequirements GetMemoryRequirements(IDevice* device) const;
    };
} // namespace RHI::Vulkan