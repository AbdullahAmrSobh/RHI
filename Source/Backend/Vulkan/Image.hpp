#pragma once
#include "RHI/Image.hpp"

#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

VkSampleCountFlagBits ConvertSampleCount(SampleCount sampleCount);

VkImageType ConvertImageType(ImageType imageType);

VkImageUsageFlags ConvertImageUsage(ImageUsageFlags usageFlags);

VkImageViewType ConvertImageViewType(ImageViewType imageType);

VkImageAspectFlags ConvertViewAspect(ImageViewAspectFlags aspectFlags);

class Image final
    : public IImage
    , public Resource<VkImage>
{
public:
    Image(Device& device, VkImage imageHandle = VK_NULL_HANDLE)
        : Resource(device, imageHandle)
    {
    }
    ~Image();

    VkResult Init(const AllocationDesc& allocationDesc, const ImageDesc& desc);

    ResultCode SetDataInternal(size_t byteOffset, const uint8_t* bufferData, size_t bufferDataByteSize) override
    {
        return ConvertResult(UploadResourceData(*m_device, m_allocation, byteOffset, bufferData, bufferDataByteSize));
    }
};

class ImageView final
    : public IImageView
    , public DeviceObject<VkImageView>
{
public:
    ImageView(Device& device)
        : DeviceObject(device)
    {
    }
    ~ImageView();

    VkResult Init(const Image& image, const ImageViewDesc& desc);
};

}  // namespace Vulkan
}  // namespace RHI