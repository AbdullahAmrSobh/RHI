#pragma once
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

#include "RHI/Core/Expected.hpp"
#include "RHI/Common.hpp"
#include "RHI/Format.hpp"
#include "RHI/Resource.hpp"

#include "Backend/Vulkan/Vma/vk_mem_alloc.hpp"
#include <vulkan/vulkan_core.h>

#undef SUCCESS

#define RHI_ASSERT_SUCCESS(X) assert(X == VK_SUCCESS);
#define RHI_RETURN_ON_FAIL(X)                                                                                                                                  \
    if (X != VK_SUCCESS)                                                                                                                                       \
    {                                                                                                                                                          \
        return X;                                                                                                                                              \
    }
#define RHI_SUCCESS(X) (X == VK_SUCCESS)

namespace RHI
{

template <typename T>
using Result      = tl::expected<T, VkResult>;
using ResultError = tl::unexpected<VkResult>;
template <class T>
using ResultSuccess = tl::expected<T, VkResult>;

inline EResultCode ConvertResult(VkResult resultCode)
{
    switch (resultCode)
    {
    case VK_SUCCESS: return EResultCode::Success;
    case VK_TIMEOUT: return EResultCode::Timeout;
    case VK_NOT_READY: return EResultCode::NotReady;
    case VK_ERROR_OUT_OF_HOST_MEMORY: return EResultCode::HostOutOfMemory;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return EResultCode::DeviceOutOfMemory;
    case VK_ERROR_EXTENSION_NOT_PRESENT: return EResultCode::ExtensionNotAvailable;
    case VK_ERROR_FEATURE_NOT_PRESENT: return EResultCode::FeatureNotAvailable;
    default: return EResultCode::Fail;
    }
}


// inline VkImageViewType ConvertImageType(EImageType imageType, bool array)
// {
//     static VkImageViewType lookup[] = {VK_IMAGE_VIEW_TYPE_1D, VK_IMAGE_VIEW_TYPE_1D_ARRAY,   VK_IMAGE_VIEW_TYPE_2D,      VK_IMAGE_VIEW_TYPE_2D_ARRAY,
//                                        VK_IMAGE_VIEW_TYPE_3D, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY, VK_IMAGE_VIEW_TYPE_MAX_ENUM};
//     uint32_t               index    = static_cast<uint32_t>(imageType);
//     return lookup[index + (array ? 0 : 1)];
// }

// VkFormat ConvertFormat(EFormat format);

// uint32_t GetTexelSize(EFormat format);

// uint32_t GetTexelSize(VkFormat format);

// VkShaderStageFlags CovnertShaderStages(ShaderStageFlags stages);

// PipelineState

// VkCullModeFlags ConvertCullMode(GraphicsPipelineRasterizationState::ECullMode cullMode);
// VkPolygonMode ConvertFillMode(PipelineStateDesc::Rasterization::EFillMode fillMode);
// VkImageAspectFlags ConvertViewAspect(ImageViewAspectFlags aspectFlags);
// VkSamplerAddressMode ConvertAddressMode(SamplerDesc::EAddressMode addressMode);

// Image Utility functions

inline VkExtent3D ConvertExtent(Extent3D extent)
{
    return {extent.sizeX, extent.sizeY, extent.sizeZ};
}

inline VkExtent2D ConvertExtent(Extent2D extent)
{
    return {extent.sizeX, extent.sizeY};
}

// inline VkSampleCountFlagBits ConvertSampleCount(ESampleCount sampleCount)
// {
//     return static_cast<VkSampleCountFlagBits>(static_cast<uint32_t>(sampleCount));
// }

// inline VkImageType ConvertImageType(Extent3D extent)
// {
//     if (extent.sizeY == 1 && extent.sizeZ == 1)
//     {
//         return VK_IMAGE_TYPE_1D;
//     }
//     else if (extent.sizeZ == 1)
//     {
//         return VK_IMAGE_TYPE_2D;
//     }
//     else
//     {
//         return VK_IMAGE_TYPE_3D;
//     }
// 
//     return VK_IMAGE_TYPE_MAX_ENUM;
// }

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

// VkBufferUsageFlags      ConvertBufferUsage(BufferUsageFlags usageFlags);
// VkImageUsageFlags       ConvertImageUsage(ImageUsageFlags usageFlags);
// VkCompareOp             ConvertCompareOp(SamplerDesc::ECompareOp compareOp);
// VkImageViewType         ConvertImageViewType(EImageType imageType, bool array);

// VmaAllocationCreateFlags ConvertAllocationUsage(RHI::EMemoryUsage usage);

VkFormat ConvertFormat(EFormat format);
uint32_t FormatStrideSize(EFormat format);
uint32_t FormatStrideSize(VkFormat format);
VkSampleCountFlagBits ConvertSampleCount(ESampleCount sampleCount);


} // namespace RHI