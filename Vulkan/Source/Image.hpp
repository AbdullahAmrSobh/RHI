#pragma once

#include <RHI/Image.hpp>
#include <RHI/Result.hpp>

#include "MemoryAllocator.hpp"

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;

    VkImageSubresource ConvertSubresource(const ImageSubresource& subresource);

    VkImageUsageFlagBits ConvertImageUsage(ImageUsage imageUsage);

    VkImageUsageFlags ConvertImageUsageFlags(TL::Flags<ImageUsage> imageUsageFlags);

    VkImageType ConvertImageType(ImageType imageType);

    VkImageAspectFlagBits ConvertImageAspect(TL::Flags<ImageAspect> imageAspect);

    VkImageAspectFlags ConvertImageAspect(ImageAspect imageAspect);

    VkImageAspectFlags FormatToAspect(VkFormat format);

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
        Allocation            allocation;
        VkImage               handle;
        VkImageView           viewHandle;
        ImageSubresourceRange subresources;

        ResultCode Init(IDevice* device, const ImageCreateInfo& createInfo);
        ResultCode Init(IDevice* device, VkImage image, const VkSwapchainCreateInfoKHR& swapchainCreateInfo);
        void       Shutdown(IDevice* device);

        VkMemoryRequirements GetMemoryRequirements(IDevice* device) const;
    };
} // namespace RHI::Vulkan