#include <RHI/RHI.hpp>

#include <TL/Assert.hpp>
#include <TL/Log.hpp>

#include <tracy/Tracy.hpp>

namespace RHI
{
    Result<Handle<Image>> CreateImageWithContent(Device& device, const ImageCreateInfo& createInfo, TL::Block content)
    {
        auto imageSizeBytes = CalcaulteImageSize(createInfo.format, createInfo.size, createInfo.mipLevels, createInfo.arrayCount);

        TL_ASSERT(imageSizeBytes != 0);
        TL_ASSERT(imageSizeBytes == content.size);
        TL_ASSERT(createInfo.usageFlags & ImageUsage::CopyDst);

        if (auto [image, result] = device.CreateImage(createInfo); IsSuccess(result))
        {
            auto stagingBuffer = device.StagingAllocate(imageSizeBytes);
            memcpy(stagingBuffer.ptr, content.ptr, content.size);
            device.UploadImage({
                .image           = image,
                .srcBuffer       = stagingBuffer.buffer,
                .srcBufferOffset = stagingBuffer.offset,
                .sizeBytes       = content.size,
            });
            return image;
        }
        else
        {
            return result;
        }
    }

    DeviceLimits Device::GetLimits() const
    {
        return *m_limits;
    }

} // namespace RHI