#include "Backend/Vulkan/Common.hpp"

#include "RHI/Format.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Resource.hpp"
#include <vulkan/vulkan_core.h>

namespace RHI
{
namespace Vulkan
{

    EResultCode ConvertResult(VkResult resultCode)
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

    VkShaderStageFlags CovnertShaderStages(ShaderStageFlags stages)
    {
        VkShaderStageFlags flags{};

        if (stages & EShaderStageFlagBits::Vertex)
        {
            flags |= VK_SHADER_STAGE_VERTEX_BIT;
        }

        if (stages & EShaderStageFlagBits::Vertex)
        {
            flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        return flags;
    }

    VkShaderStageFlagBits CovnertShaderStages(EShaderStageFlagBits stages)
    {
        VkShaderStageFlagBits lookup[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
        return lookup[static_cast<uint32_t>(stages)];
    }

    VkCullModeFlags ConvertRasterizationStateCullMode(ERasterizationCullMode cullMode)
    {

        VkCullModeFlagBits lookup[] = {
            VK_CULL_MODE_NONE,
            VK_CULL_MODE_FRONT_BIT,
            VK_CULL_MODE_BACK_BIT,
        };

        return lookup[static_cast<uint32_t>(cullMode)];
    }

    VkPolygonMode ConvertRasterizationStateFillMode(ERasterizationFillMode fillMode)
    {
        VkPolygonMode polygonMode[] = {
            VK_POLYGON_MODE_POINT,
            VK_POLYGON_MODE_LINE,
            VK_POLYGON_MODE_FILL,
        };

        return polygonMode[static_cast<uint32_t>(fillMode)];
    }

    VkFilter ConvertFilter(ESamplerFilter filter)
    {
        return filter == ESamplerFilter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
    }

    VkSamplerMipmapMode ConvertMipMapMode(ESamplerFilter filter)
    {
        return filter == ESamplerFilter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }

    VkSamplerAddressMode ConvertAddressMode(ESamplerAddressMode addressMode)
    {
        return addressMode == ESamplerAddressMode::Repeat ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }

    VkCompareOp ConvertCompareOp(ESamplerCompareOp compareOp)
    {
        switch (compareOp)
        {
        case ESamplerCompareOp::Equal: return VK_COMPARE_OP_EQUAL;
        case ESamplerCompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
        case ESamplerCompareOp::Always: return VK_COMPARE_OP_ALWAYS;
        case ESamplerCompareOp::Greater: return VK_COMPARE_OP_GREATER;
        case ESamplerCompareOp::GreaterEq: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case ESamplerCompareOp::Less: return VK_COMPARE_OP_LESS;
        case ESamplerCompareOp::LessEq: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case ESamplerCompareOp::Never: return VK_COMPARE_OP_NEVER;
        }
        return VK_COMPARE_OP_MAX_ENUM;
    }

    VkSamplerMipmapMode ConvertSamplerMipMapMode(ESamplerFilter filter)
    {
        VkSamplerMipmapMode lookup[] = {VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST};
        return lookup[static_cast<uint32_t>(filter)];
    }

    VkSamplerAddressMode ConvertSamplerAddressMode(ESamplerAddressMode addressMode)
    {
        VkSamplerAddressMode lookup[] = {
            VK_SAMPLER_ADDRESS_MODE_REPEAT,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        };

        return lookup[static_cast<uint32_t>(addressMode)];
    }

    VkCompareOp ConvertSamplerCompareOp(ESamplerCompareOp compareOp)
    {
        VkCompareOp lookup[] = {
            VK_COMPARE_OP_NEVER, VK_COMPARE_OP_LESS, VK_COMPARE_OP_LESS_OR_EQUAL, VK_COMPARE_OP_GREATER, VK_COMPARE_OP_GREATER_OR_EQUAL,
        };

        return lookup[static_cast<uint32_t>(compareOp)];
    }

    VkFormat ConvertFormat(EFormat format)
    {
        return static_cast<VkFormat>(static_cast<uint32_t>(format));
    }

    uint32_t FormatStrideSize(EFormat format)
    {
        return FormatStrideSize(ConvertFormat(format));
    }

    uint32_t FormatStrideSize(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_USCALED: return (1 * 4);

        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_USCALED: return (2 * 4);

        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_USCALED: return (3 * 4);

        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_USCALED: return (4 * 4);
        default:
        {
            assert(false); // this format is either undefined or unimplemented
            return UINT32_MAX;
        };
        }
    }

    VkImageViewType ConvertImageViewType(EImageViewType imageType)
    {
        return static_cast<VkImageViewType>(static_cast<uint32_t>(imageType));
    }

    VkImageAspectFlags ConvertViewAspect(ImageViewAspectFlags aspectFlags)
    {
        VkImageAspectFlags flags{};
        if (aspectFlags & EImageViewAspectFlagBits::Color)
        {
            flags |= VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (aspectFlags & EImageViewAspectFlagBits::Depth)
        {
            flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        if (aspectFlags & EImageViewAspectFlagBits::Stencil)
        {
            flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        return flags;
    }

    VkImageUsageFlags ConvertImageUsage(ImageUsageFlags usageFlags)
    {
        VkImageUsageFlags flags = 0;
        if (usageFlags & EImageUsageFlagBits::Color)
        {
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        if (usageFlags & EImageUsageFlagBits::DepthStencil)
        {
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        if (usageFlags & EImageUsageFlagBits::ShaderInput)
        {
            flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }
        if (usageFlags & EImageUsageFlagBits::Transfer)
        {
            flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }
        return flags;
    }

    VkBufferUsageFlags ConvertBufferUsage(BufferUsageFlags usageFlags)
    {
        VkBufferUsageFlags flags = 0;
        if (usageFlags & EBufferUsageFlagBits::Index)
        {
            flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        if (usageFlags & EBufferUsageFlagBits::Vertex)
        {
            flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }
        if (usageFlags & EBufferUsageFlagBits::Transfer)
        {
            flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }
        return flags;
    }

    VmaMemoryUsage ConvertMemoryUsage(EMemoryUsage usage)
    {
        switch (usage)
        {
        case EMemoryUsage::Stream: return VMA_MEMORY_USAGE_CPU_COPY;
        case EMemoryUsage::Hosted: return VMA_MEMORY_USAGE_CPU_ONLY;
        case EMemoryUsage::Stage: return VMA_MEMORY_USAGE_CPU_TO_GPU;
        case EMemoryUsage::Local: return VMA_MEMORY_USAGE_GPU_ONLY;
        };

        return VMA_MEMORY_USAGE_UNKNOWN;
    }

} // namespace Vulkan
} // namespace RHI