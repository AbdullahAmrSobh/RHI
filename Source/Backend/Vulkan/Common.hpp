#pragma once
#include "RHI/Common.hpp"

#include "RHI/Debug.hpp"
#include "RHI/Format.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Resource.hpp"
#include "RHI/Image.hpp"
#include "RHI/Buffer.hpp"

/////////////////////////////////////////////////
#include "Backend/Vulkan/Vma/vk_mem_alloc.hpp"
/////////////////////////////////////////////////
#include "Backend/Vulkan/DeviceObject.hpp"

namespace RHI
{
namespace Vulkan
{

ResultCode ConvertResult(VkResult resultCode);

VkShaderStageFlags CovnertShaderStages(ShaderStageFlags stages);

VkShaderStageFlagBits CovnertShaderStages(ShaderStageFlagBits stages);

VkCullModeFlags ConvertRasterizationStateCullMode(RasterizationCullMode cullMode);

VkPolygonMode ConvertRasterizationStateFillMode(RasterizationFillMode fillMode);

VkFilter ConvertFilter(SamplerFilter filter);

VkSamplerMipmapMode ConvertSamplerMipMapMode(SamplerFilter filter);

VkSamplerAddressMode ConvertSamplerAddressMode(SamplerAddressMode addressMode);

VkCompareOp ConvertSamplerCompareOp(SamplerCompareOp compareOp);

VkFormat ConvertFormat(Format format);

uint32_t FormatStrideSize(Format format);

uint32_t FormatStrideSize(VkFormat format);

VkImageViewType ConvertImageViewType(ImageViewType imageType);

VkImageAspectFlags ConvertViewAspect(ImageViewAspectFlags aspectFlags);

inline VkExtent3D ConvertExtent(Extent3D extent)
{
    return {extent.sizeX, extent.sizeY, extent.sizeZ};
}

inline VkExtent2D ConvertExtent(Extent2D extent)
{
    return {extent.sizeX, extent.sizeY};
}

inline VkSampleCountFlagBits ConvertSampleCount(SampleCount sampleCount)
{
    return static_cast<VkSampleCountFlagBits>(static_cast<uint32_t>(sampleCount));
}

inline VkViewport ConvertViewport(const Viewport& viewport)
{
    return {float(viewport.drawingArea.x),
            float(viewport.drawingArea.y),
            float(viewport.drawingArea.sizeX),
            float(viewport.drawingArea.sizeY),
            viewport.minDepth,
            viewport.maxDepth};
}

inline VkRect2D ConvertRect(const Rect& rect)
{
    return {{rect.x, rect.y}, {rect.sizeX, rect.sizeY}};
}

// these flags may be refactored
VkImageUsageFlags ConvertImageUsage(ImageUsageFlags usageFlags);

VkBufferUsageFlags ConvertBufferUsage(BufferUsageFlags usageFlags);

VmaMemoryUsage ConvertMemoryUsage(MemoryUsage usage);

namespace Utils
{

inline void AssertSuccess(VkResult result)
{
    assert(result == VK_SUCCESS);
}

inline bool IsSuccess(VkResult result)
{
    return result >= 0;
}

inline bool IsError(VkResult result)
{
    return !(IsSuccess(result));
}

}  // namespace Utils
}  // namespace Vulkan
}  // namespace RHI

#define VK_RETURN_ON_ERROR(result)                                                                                                         \
    if (::RHI::Vulkan::Utils::IsError(result))                                                                                             \
    {                                                                                                                                      \
        return result;                                                                                                                     \
    }