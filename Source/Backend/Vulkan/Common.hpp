#pragma once
#include "RHI/Common.hpp"
#include "RHI/Format.hpp"
#include "RHI/Resource.hpp"
#include "RHI/PipelineState.hpp"

#include "Backend/Vulkan/Vma/vk_mem_alloc.hpp"

#undef SUCCESS

#define RHI_ASSERT_SUCCESS(X) assert(X == VK_SUCCESS);

#define RHI_SUCCESS(X) (X == VK_SUCCESS)

#define RHI_RETURN_ON_FAIL(X)                                                                                                                                  \
    if (RHI_SUCCESS(X))                                                                                                                                        \
    {                                                                                                                                                          \
        return X;                                                                                                                                              \
    }

namespace RHI
{
namespace Vulkan
{
    template <typename T>
    using Result = tl::expected<T, VkResult>;

    using ResultError = tl::unexpected<VkResult>;

    template <class T>
    using ResultSuccess = tl::expected<T, VkResult>;

    EResultCode ConvertResult(VkResult resultCode);

    VkShaderStageFlags CovnertShaderStages(ShaderStageFlags stages);

    VkShaderStageFlagBits CovnertShaderStages(EShaderStageFlagBits stages);

    VkCullModeFlags ConvertRasterizationStateCullMode(ERasterizationCullMode cullMode);
    
    VkPolygonMode ConvertRasterizationStateFillMode(ERasterizationFillMode fillMode);

    VkFilter ConvertFilter(ESamplerFilter filter);

    VkSamplerMipmapMode ConvertSamplerMipMapMode(ESamplerFilter filter);

    VkSamplerAddressMode ConvertSamplerAddressMode(ESamplerAddressMode addressMode);

    VkCompareOp ConvertSamplerCompareOp(ESamplerCompareOp compareOp);

    VkFormat ConvertFormat(EFormat format);

    uint32_t FormatStrideSize(EFormat format);

    uint32_t FormatStrideSize(VkFormat format);

    VkImageViewType ConvertImageType(EImageViewType imageType);

    VkImageAspectFlags ConvertViewAspect(ImageViewAspectFlags aspectFlags);

    inline VkExtent3D ConvertExtent(Extent3D extent)
    {
        return {extent.sizeX, extent.sizeY, extent.sizeZ};
    }

    inline VkExtent2D ConvertExtent(Extent2D extent)
    {
        return {extent.sizeX, extent.sizeY};
    }

    inline VkSampleCountFlagBits ConvertSampleCount(ESampleCount sampleCount)
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

    VmaMemoryUsage ConvertAllocationUsage(EMemoryUsage usage);

} // namespace Vulkan
} // namespace RHI
