#pragma once
#include "RHI/Common.hpp"
#include <vulkan/vulkan.h>

#define RHI_ASSERT_SUCCESS(X) assert(X == VK_SUCCESS);
#define RHI_RETURN_ON_FAIL(X) if(X != VK_SUCCESS) { return X; }
#define RHI_SUCCESS(X)        (X == VK_SUCCESS)

namespace RHI
{

EResultCode ConvertResult(VkResult resultCode);

VkImageViewType ConvertImageType(EImageType imageType);

VkFormat ConvertFormat(EFormat format);

uint32_t GetTexelSize(EFormat format);

uint32_t GetTexelSize(VkFormat format);

VkShaderStageFlagBits CovnertShaderStages(EShaderStageFlagBits stages);

// PipelineState

VkCullModeFlags ConvertCullMode(PipelineStateDesc::Rasterization::ECullMode cullMode);
VkPolygonMode ConvertFillMode(PipelineStateDesc::Rasterization::EFillMode fillMode);

VkImageAspectFlags ConvertViewAspect(ImageViewAspectFlags aspectFlags);

VkSamplerAddressMode ConvertAddressMode(SamplerDesc::EAddressMode addressMode);

// Sampler Utility functions 

VkBool32 IsCompareEnable(SamplerDesc::ECompareOp comapreOp);

VkCompareOp ConvertCompareOp(SamplerDesc::ECompareOp compareOp);

VkSamplerMipmapMode ConvertFilterToSamplerMipmapMode(SamplerDesc::EFilter filter);

VkFilter ConvertFilter(SamplerDesc::EFilter filter);

// Buffer Utility functions 

VkBufferUsageFlags ConvertBufferUsage(BufferUsageFlags usageFlags);

// Image Utility functions 

VkExtent3D ConvertExtent(Extent3D extent);

VkExtent2D ConvertExtent(Extent2D extent);

VkSampleCountFlagBits ConvertSampleCount(ESampleCount sampleCount);

VkImageUsageFlags ConvertImageUsage(ImageUsageFlags usageFlags);

VkImageType ConvertImageType(Extent3D extent);

VmaMemoryUsage ConvertMemoryUsage(EMemoryUsage usage);

VkViewport ConvertViewport(const Viewport& viewport);

VkRect2D ConvertRect(const Rect& rect);
} // namespace RHI