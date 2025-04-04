#pragma once

#include <RHI/Device.hpp>
#include <RHI/Resources.hpp>
#include <RHI/Result.hpp>

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    class IDevice;

    VkBufferUsageFlags      ConvertBufferUsageFlags(TL::Flags<BufferUsage> bufferUsageFlags);
    VkImageUsageFlags       ConvertImageUsageFlags(TL::Flags<ImageUsage> imageUsageFlags);
    VkImageType             ConvertImageType(ImageType imageType);
    VkImageViewType         ConvertImageViewType(ImageViewType imageType);
    VkImageAspectFlags      ConvertImageAspect(TL::Flags<ImageAspect> imageAspect, Format format);
    VkComponentSwizzle      ConvertComponentSwizzle(ComponentSwizzle componentSwizzle);
    VkImageSubresourceRange ConvertSubresourceRange(const ImageSubresourceRange& subresource);
    VkComponentMapping      ConvertComponentMapping(ComponentMapping componentMapping);
    VkExtent2D              ConvertExtent2D(ImageSize2D size);
    VkExtent3D              ConvertExtent3D(ImageSize3D size);
    VkExtent2D              ConvertExtent2D(ImageSize3D size);
    VkOffset2D              ConvertOffset2D(ImageOffset2D offset);
    VkOffset3D              ConvertOffset3D(ImageOffset3D offset);
    VkFilter                ConvertFilter(SamplerFilter samplerFilter);
    VkSamplerAddressMode    ConvertSamplerAddressMode(SamplerAddressMode addressMode);
    VkCompareOp             ConvertCompareOp(CompareOperator compareOperator);

    struct IBuffer : Buffer
    {
        VmaAllocation allocation;
        VkBuffer      handle;

        ResultCode Init(IDevice* device, const BufferCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        VkMemoryRequirements GetMemoryRequirements(IDevice* device) const;

        DeviceMemoryPtr Map(IDevice* device);
        void            Unmap(IDevice* device);
    };

    struct IImage : Image
    {
        VmaAllocation         allocation;
        VkImage               handle;
        VkImageView           viewHandle;
        ImageSize3D           size;
        Format                format;
        ImageSubresourceRange subresources;

        ResultCode Init(IDevice* device, const ImageCreateInfo& createInfo);
        ResultCode Init(IDevice* device, VkImage image, const VkSwapchainCreateInfoKHR& swapchainCreateInfo);
        void       Shutdown(IDevice* device);

        VkMemoryRequirements GetMemoryRequirements(IDevice* device) const;

        // Selects specifc aspect from the available image aspects
        VkImageAspectFlags SelectImageAspect(ImageAspect aspect);
    };

    struct ISampler : Sampler
    {
        VkSampler handle;

        ResultCode Init(IDevice* device, const SamplerCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };
} // namespace RHI::Vulkan