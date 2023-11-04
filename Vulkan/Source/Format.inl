#pragma once

#include <vulkan/vulkan.h>

#include <RHI/Format.hpp>
#include <RHI/Assert.hpp>
#include <RHI/Result.hpp>

namespace Vulkan
{
    inline static RHI::ResultCode ConvertToRhiResult(VkResult result)
    {
        switch (result)
        {
        case VK_SUCCESS:
            return RHI::ResultCode::Success;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return RHI::ResultCode::ErrorOutOfMemory;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return RHI::ResultCode::ErrorDeviceOutOfMemory;
        default:
            return RHI::ResultCode::ErrorUnkown;
        }
    }

    inline static VkFormat GetFormat(RHI::Format format)
    {
        switch (format)
        {
        case RHI::Format::None:
            return VK_FORMAT_UNDEFINED;
        case RHI::Format::B8G8R8A8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case RHI::Format::R32G32B32A32_FLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case RHI::Format::R32G32B32A32_UINT:
            return VK_FORMAT_R32G32B32A32_UINT;
        case RHI::Format::R32G32B32A32_SINT:
            return VK_FORMAT_R32G32B32A32_SINT;
        case RHI::Format::R32G32B32_FLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case RHI::Format::R32G32B32_UINT:
            return VK_FORMAT_R32G32B32_UINT;
        case RHI::Format::R32G32B32_SINT:
            return VK_FORMAT_R32G32B32_SINT;
        case RHI::Format::R16G16B16A16_FLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case RHI::Format::R16G16B16A16_UNORM:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case RHI::Format::R16G16B16A16_UINT:
            return VK_FORMAT_R16G16B16A16_UINT;
        case RHI::Format::R16G16B16A16_SNORM:
            return VK_FORMAT_R16G16B16A16_SNORM;
        case RHI::Format::R16G16B16A16_SINT:
            return VK_FORMAT_R16G16B16A16_SINT;
        case RHI::Format::R32G32_FLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
        case RHI::Format::R32G32_UINT:
            return VK_FORMAT_R32G32_UINT;
        case RHI::Format::R32G32_SINT:
            return VK_FORMAT_R32G32_SINT;
        case RHI::Format::D32_FLOAT_S8X24_UINT:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case RHI::Format::R10G10B10A2_UNORM:
            return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        case RHI::Format::R10G10B10A2_UINT:
            return VK_FORMAT_A2R10G10B10_UINT_PACK32;
        case RHI::Format::R11G11B10_FLOAT:
            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case RHI::Format::R8G8B8A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case RHI::Format::R8G8B8A8_UNORM_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case RHI::Format::R8G8B8A8_UINT:
            return VK_FORMAT_R8G8B8A8_UINT;
        case RHI::Format::R8G8B8A8_SNORM:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case RHI::Format::R8G8B8A8_SINT:
            return VK_FORMAT_R8G8B8A8_SINT;
        case RHI::Format::R16G16_FLOAT:
            return VK_FORMAT_R16G16_SFLOAT;
        case RHI::Format::R16G16_UNORM:
            return VK_FORMAT_R16G16_UNORM;
        case RHI::Format::R16G16_UINT:
            return VK_FORMAT_R16G16_UINT;
        case RHI::Format::R16G16_SNORM:
            return VK_FORMAT_R16G16_SNORM;
        case RHI::Format::R16G16_SINT:
            return VK_FORMAT_R16G16_SINT;
        case RHI::Format::D32_FLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case RHI::Format::R32_FLOAT:
            return VK_FORMAT_R32_SFLOAT;
        case RHI::Format::R32_UINT:
            return VK_FORMAT_R32_UINT;
        case RHI::Format::R32_SINT:
            return VK_FORMAT_R32_SINT;
        case RHI::Format::D24_UNORM_S8_UINT:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case RHI::Format::R8G8_UNORM:
            return VK_FORMAT_R8G8_UNORM;
        case RHI::Format::R8G8_UNORM_SRGB:
            return VK_FORMAT_R8G8_SRGB;
        case RHI::Format::R8G8_UINT:
            return VK_FORMAT_R8G8_UINT;
        case RHI::Format::R8G8_SNORM:
            return VK_FORMAT_R8G8_SNORM;
        case RHI::Format::R8G8_SINT:
            return VK_FORMAT_R8G8_SINT;
        case RHI::Format::R16_FLOAT:
            return VK_FORMAT_R16_SFLOAT;
        case RHI::Format::D16_UNORM:
            return VK_FORMAT_D16_UNORM;
        case RHI::Format::R16_UNORM:
            return VK_FORMAT_R16_UNORM;
        case RHI::Format::R16_UINT:
            return VK_FORMAT_R16_UINT;
        case RHI::Format::R16_SNORM:
            return VK_FORMAT_R16_SNORM;
        case RHI::Format::R16_SINT:
            return VK_FORMAT_R16_SINT;
        case RHI::Format::R8_UNORM:
            return VK_FORMAT_R8_UNORM;
        case RHI::Format::R8_UNORM_SRGB:
            return VK_FORMAT_R8_SRGB;
        case RHI::Format::R8_UINT:
            return VK_FORMAT_R8_UINT;
        case RHI::Format::R8_SNORM:
            return VK_FORMAT_R8_SNORM;
        case RHI::Format::R8_SINT:
            return VK_FORMAT_R8_SINT;
        default:
            RHI_UNREACHABLE();
            return VK_FORMAT_UNDEFINED;
        }
    }

} // namespace Vulkan