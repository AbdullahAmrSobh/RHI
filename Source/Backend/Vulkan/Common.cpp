#include "RHI/Pch.hpp"

#include "Backend/Vulkan/Common.hpp"

#include "RHI/Buffer.hpp"
#include "RHI/Format.hpp"
#include "RHI/Image.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

ResultCode ConvertResult(VkResult resultCode)
{
    switch (resultCode)
    {
        case VK_SUCCESS: return ResultCode::Success;
        case VK_TIMEOUT: return ResultCode::Timeout;
        case VK_NOT_READY: return ResultCode::NotReady;
        case VK_ERROR_OUT_OF_HOST_MEMORY: return ResultCode::HostOutOfMemory;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return ResultCode::DeviceOutOfMemory;
        case VK_ERROR_EXTENSION_NOT_PRESENT: return ResultCode::ExtensionNotAvailable;
        case VK_ERROR_FEATURE_NOT_PRESENT: return ResultCode::FeatureNotAvailable;
        default: return ResultCode::Fail;
    }
}

VkShaderStageFlags CovnertShaderStages(ShaderStageFlags stages)
{
    VkShaderStageFlags flags {};

    if (stages & ShaderStageFlagBits::Vertex)
    {
        flags |= VK_SHADER_STAGE_VERTEX_BIT;
    }

    if (stages & ShaderStageFlagBits::Vertex)
    {
        flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    return flags;
}

VkShaderStageFlagBits CovnertShaderStages(ShaderStageFlagBits stages)
{
    VkShaderStageFlagBits lookup[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    return lookup[static_cast<uint32_t>(stages)];
}

VkCullModeFlags ConvertRasterizationStateCullMode(RasterizationCullMode cullMode)
{
    VkCullModeFlagBits lookup[] = {
        VK_CULL_MODE_NONE,
        VK_CULL_MODE_FRONT_BIT,
        VK_CULL_MODE_BACK_BIT,
    };

    return lookup[static_cast<uint32_t>(cullMode)];
}

VkPolygonMode ConvertRasterizationStateFillMode(RasterizationFillMode fillMode)
{
    VkPolygonMode polygonMode[] = {
        VK_POLYGON_MODE_POINT,
        VK_POLYGON_MODE_LINE,
        VK_POLYGON_MODE_FILL,
    };

    return polygonMode[static_cast<uint32_t>(fillMode)];
}

VkFilter ConvertFilter(SamplerFilter filter)
{
    return filter == SamplerFilter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
}

VkSamplerMipmapMode ConvertMipMapMode(SamplerFilter filter)
{
    return filter == SamplerFilter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
}

VkSamplerAddressMode ConvertAddressMode(SamplerAddressMode addressMode)
{
    return addressMode == SamplerAddressMode::Repeat ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
}

VkCompareOp ConvertCompareOp(SamplerCompareOp compareOp)
{
    switch (compareOp)
    {
        case SamplerCompareOp::Equal: return VK_COMPARE_OP_EQUAL;
        case SamplerCompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
        case SamplerCompareOp::Always: return VK_COMPARE_OP_ALWAYS;
        case SamplerCompareOp::Greater: return VK_COMPARE_OP_GREATER;
        case SamplerCompareOp::GreaterEq: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case SamplerCompareOp::Less: return VK_COMPARE_OP_LESS;
        case SamplerCompareOp::LessEq: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case SamplerCompareOp::Never: return VK_COMPARE_OP_NEVER;
    }
    return VK_COMPARE_OP_MAX_ENUM;
}

VkSamplerMipmapMode ConvertSamplerMipMapMode(SamplerFilter filter)
{
    VkSamplerMipmapMode lookup[] = {VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST};
    return lookup[static_cast<uint32_t>(filter)];
}

VkSamplerAddressMode ConvertSamplerAddressMode(SamplerAddressMode addressMode)
{
    VkSamplerAddressMode lookup[] = {
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    };

    return lookup[static_cast<uint32_t>(addressMode)];
}

VkCompareOp ConvertSamplerCompareOp(SamplerCompareOp compareOp)
{
    VkCompareOp lookup[] = {
        VK_COMPARE_OP_NEVER,
        VK_COMPARE_OP_LESS,
        VK_COMPARE_OP_LESS_OR_EQUAL,
        VK_COMPARE_OP_GREATER,
        VK_COMPARE_OP_GREATER_OR_EQUAL,
    };

    return lookup[static_cast<uint32_t>(compareOp)];
}

VkFormat ConvertFormat(Format format)
{
    switch (format)
    {
        case Format::Uninitialized: return VK_FORMAT_UNDEFINED;
        case Format::R32G32B32A32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Format::R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
        case Format::R32G32B32A32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
        case Format::R32G32B32_FLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
        case Format::R32G32B32_UINT: return VK_FORMAT_R32G32B32_UINT;
        case Format::R32G32B32_SINT: return VK_FORMAT_R32G32B32_SINT;
        case Format::R16G16B16A16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case Format::R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
        case Format::R16G16B16A16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
        case Format::R16G16B16A16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;
        case Format::R16G16B16A16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
        case Format::R32G32_FLOAT: return VK_FORMAT_R32G32_SFLOAT;
        case Format::R32G32_UINT: return VK_FORMAT_R32G32_UINT;
        case Format::R32G32_SINT: return VK_FORMAT_R32G32_SINT;
        case Format::D32_FLOAT_S8X24_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case Format::R10G10B10A2_UNORM: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        case Format::R10G10B10A2_UINT: return VK_FORMAT_A2R10G10B10_UINT_PACK32;
        case Format::R11G11B10_FLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case Format::R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::R10G10B10_XR_BIAS_A2_UNORM: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case Format::R8G8B8A8_UNORM_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
        case Format::R8G8B8A8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
        case Format::R8G8B8A8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;
        case Format::R8G8B8A8_SINT: return VK_FORMAT_R8G8B8A8_SINT;
        case Format::A8B8G8R8_UNORM: return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
        case Format::A8B8G8R8_SNORM: return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
        case Format::A8B8G8R8_UNORM_SRGB: return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
        case Format::R16G16_FLOAT: return VK_FORMAT_R16G16_SFLOAT;
        case Format::R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
        case Format::R16G16_UINT: return VK_FORMAT_R16G16_UINT;
        case Format::R16G16_SNORM: return VK_FORMAT_R16G16_SNORM;
        case Format::R16G16_SINT: return VK_FORMAT_R16G16_SINT;
        case Format::D32_FLOAT: return VK_FORMAT_D32_SFLOAT;
        case Format::R32_FLOAT: return VK_FORMAT_R32_SFLOAT;
        case Format::R32_UINT: return VK_FORMAT_R32_UINT;
        case Format::R32_SINT: return VK_FORMAT_R32_SINT;
        case Format::D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
        case Format::R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
        case Format::R8G8_UINT: return VK_FORMAT_R8G8_UINT;
        case Format::R8G8_SNORM: return VK_FORMAT_R8G8_SNORM;
        case Format::R8G8_SINT: return VK_FORMAT_R8G8_SINT;
        case Format::R16_FLOAT: return VK_FORMAT_R16_SFLOAT;
        case Format::D16_UNORM: return VK_FORMAT_D16_UNORM;
        case Format::R16_UNORM: return VK_FORMAT_R16_UNORM;
        case Format::R16_UINT: return VK_FORMAT_R16_UINT;
        case Format::R16_SNORM: return VK_FORMAT_R16_SNORM;
        case Format::R16_SINT: return VK_FORMAT_R16_SINT;
        case Format::R8_UNORM: return VK_FORMAT_R8_UNORM;
        case Format::R8_UINT: return VK_FORMAT_R8_UINT;
        case Format::R8_SNORM: return VK_FORMAT_R8_SNORM;
        case Format::R8_SINT: return VK_FORMAT_R8_SINT;
        case Format::R9G9B9E5_SHAREDEXP: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
        case Format::R8G8_B8G8_UNORM: return VK_FORMAT_G8B8G8R8_422_UNORM;
        case Format::G8R8_G8B8_UNORM: return VK_FORMAT_B8G8R8G8_422_UNORM;
        case Format::BC1_UNORM: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case Format::BC1_UNORM_SRGB: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case Format::BC2_UNORM: return VK_FORMAT_BC2_UNORM_BLOCK;
        case Format::BC2_UNORM_SRGB: return VK_FORMAT_BC2_SRGB_BLOCK;
        case Format::BC3_UNORM: return VK_FORMAT_BC3_UNORM_BLOCK;
        case Format::BC3_UNORM_SRGB: return VK_FORMAT_BC3_SRGB_BLOCK;
        case Format::BC4_UNORM: return VK_FORMAT_BC4_UNORM_BLOCK;
        case Format::BC4_SNORM: return VK_FORMAT_BC4_SNORM_BLOCK;
        case Format::BC5_UNORM: return VK_FORMAT_BC5_UNORM_BLOCK;
        case Format::BC5_SNORM: return VK_FORMAT_BC5_SNORM_BLOCK;
        case Format::R5G6B5_UNORM: return VK_FORMAT_R5G6B5_UNORM_PACK16;
        case Format::B5G6R5_UNORM: return VK_FORMAT_B5G6R5_UNORM_PACK16;
        case Format::B5G5R5A1_UNORM: return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
        case Format::B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::B8G8R8A8_SNORM: return VK_FORMAT_B8G8R8A8_SNORM;
        case Format::B8G8R8A8_UNORM_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
        case Format::BC6H_UF16: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case Format::BC6H_SF16: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case Format::BC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
        case Format::BC7_UNORM_SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
        case Format::NV12: return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        case Format::P010: return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;
        case Format::P016: return VK_FORMAT_G16_B16R16_2PLANE_420_UNORM;
        case Format::B4G4R4A4_UNORM: return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
        case Format::R4G4B4A4_UNORM: return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        case Format::D16_UNORM_S8_UINT: return VK_FORMAT_D16_UNORM_S8_UINT;
        case Format::EAC_R11_UNORM: return VK_FORMAT_EAC_R11_UNORM_BLOCK;
        case Format::EAC_R11_SNORM: return VK_FORMAT_EAC_R11_SNORM_BLOCK;
        case Format::EAC_RG11_UNORM: return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
        case Format::EAC_RG11_SNORM: return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
        case Format::ETC2_UNORM: return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case Format::ETC2_UNORM_SRGB: return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
        case Format::ETC2A1_UNORM: return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        case Format::ETC2A1_UNORM_SRGB: return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
        case Format::ETC2A_UNORM: return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        case Format::ETC2A_UNORM_SRGB: return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
        case Format::PVRTC2_UNORM: return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
        case Format::PVRTC2_UNORM_SRGB: return VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG;
        case Format::PVRTC4_UNORM: return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
        case Format::PVRTC4_UNORM_SRGB: return VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG;
        case Format::ASTC_4x4_UNORM: return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case Format::ASTC_4x4_UNORM_SRGB: return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
        case Format::ASTC_5x4_UNORM: return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
        case Format::ASTC_5x4_UNORM_SRGB: return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
        case Format::ASTC_5x5_UNORM: return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
        case Format::ASTC_5x5_UNORM_SRGB: return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
        case Format::ASTC_6x5_UNORM: return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
        case Format::ASTC_6x5_UNORM_SRGB: return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
        case Format::ASTC_6x6_UNORM: return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
        case Format::ASTC_6x6_UNORM_SRGB: return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
        case Format::ASTC_8x5_UNORM: return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
        case Format::ASTC_8x5_UNORM_SRGB: return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
        case Format::ASTC_8x6_UNORM: return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
        case Format::ASTC_8x6_UNORM_SRGB: return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
        case Format::ASTC_8x8_UNORM: return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        case Format::ASTC_8x8_UNORM_SRGB: return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
        case Format::ASTC_10x5_UNORM: return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
        case Format::ASTC_10x5_UNORM_SRGB: return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
        case Format::ASTC_10x6_UNORM: return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
        case Format::ASTC_10x6_UNORM_SRGB: return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
        case Format::ASTC_10x8_UNORM: return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
        case Format::ASTC_10x8_UNORM_SRGB: return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
        case Format::ASTC_10x10_UNORM: return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
        case Format::ASTC_10x10_UNORM_SRGB: return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
        case Format::ASTC_12x10_UNORM: return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
        case Format::ASTC_12x10_UNORM_SRGB: return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
        case Format::ASTC_12x12_UNORM: return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
        case Format::ASTC_12x12_UNORM_SRGB: return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;

        default: break;
    }

    return VK_FORMAT_UNDEFINED;
}

VkImageViewType ConvertImageViewType(ImageViewType imageType)
{
    VkImageViewType views[] = {
        VK_IMAGE_VIEW_TYPE_1D,
        VK_IMAGE_VIEW_TYPE_1D_ARRAY,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        VK_IMAGE_VIEW_TYPE_3D,
        VK_IMAGE_VIEW_TYPE_CUBE,
        VK_IMAGE_VIEW_TYPE_CUBE_ARRAY
    };

    return views[static_cast<uint32_t>(imageType)];
}

VkImageAspectFlags ConvertViewAspect(ImageViewAspectFlags aspectFlags)
{
    VkImageAspectFlags flags {};
    if (aspectFlags & ImageViewAspectFlagBits::Color)
    {
        flags |= VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (aspectFlags & ImageViewAspectFlagBits::Depth)
    {
        flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    if (aspectFlags & ImageViewAspectFlagBits::Stencil)
    {
        flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    return flags;
}

VkImageUsageFlags ConvertImageUsage(ImageUsageFlags usageFlags)
{
    VkImageUsageFlags flags = 0;
    if (usageFlags & ImageUsageFlagBits::Color)
    {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (usageFlags & ImageUsageFlagBits::DepthStencil)
    {
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (usageFlags & ImageUsageFlagBits::ShaderInput)
    {
        flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    if (usageFlags & ImageUsageFlagBits::Transfer)
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    return flags;
}

VkBufferUsageFlags ConvertBufferUsage(BufferUsageFlags usageFlags)
{
    VkBufferUsageFlags flags = 0;
    if (usageFlags & BufferUsageFlagBits::Index)
    {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (usageFlags & BufferUsageFlagBits::Vertex)
    {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (usageFlags & BufferUsageFlagBits::Transfer)
    {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    return flags;
}

VmaMemoryUsage ConvertMemoryUsage(MemoryUsage usage)
{
    switch (usage)
    {
        case MemoryUsage::Stream: return VMA_MEMORY_USAGE_CPU_COPY;
        case MemoryUsage::Hosted: return VMA_MEMORY_USAGE_CPU_ONLY;
        case MemoryUsage::Stage: return VMA_MEMORY_USAGE_CPU_TO_GPU;
        case MemoryUsage::Local: return VMA_MEMORY_USAGE_GPU_ONLY;
    };

    return VMA_MEMORY_USAGE_UNKNOWN;
}

}  // namespace Vulkan
}  // namespace RHI