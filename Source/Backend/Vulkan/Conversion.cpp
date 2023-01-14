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