#pragma once
#include "RHI/CommandBuffer.hpp"

namespace RHI
{
namespace Vulkan
{

// convert structs
inline constexpr VkExtent3D ConvertExtent(Extent3D extent)
{
    return {extent.sizeX, extent.sizeY, extent.sizeZ};
}

inline constexpr VkExtent2D ConvertExtent(Extent2D extent)
{
    return {extent.sizeX, extent.sizeY};
}

/// convert enums
ResultCode ConvertResult(VkResult resultCode);

VkFormat ConvertFormat(Format format);

VmaMemoryUsage ConvertMemoryUsage(MemoryUsage usage);

VkSampleCountFlagBits ConvertSampleCount(SampleCount sampleCount);

}  // namespace Vulkan
}  // namespace RHI