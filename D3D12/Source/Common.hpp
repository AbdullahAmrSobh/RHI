#pragma once

#include <RHI/RHI.hpp>

#include <TL/Assert.hpp>

#include <d3d12.h>

namespace RHI::D3D12
{
#define VALIDATE_D3D12_RESULT(result)      \
    if (FAILED(result))                    \
    {                                      \
        Shutdown();                        \
        return ConvertFromHRESULT(result); \
    }

    inline static LPCSTR ConvertToLPCSTR(const char* name)
    {
        return name;
    }

    inline static LPWSTR ConvertToLPWSTR(const char* name)
    {
        return nullptr;
    }

    inline static ResultCode ConvertFromHRESULT(HRESULT hr)
    {
        switch (hr)
        {
        case S_OK:                      return ResultCode::Success;
        case E_FAIL:                    return ResultCode::ErrorUnknown;
        case E_OUTOFMEMORY:             return ResultCode::ErrorOutOfMemory;
        case DXGI_ERROR_DEVICE_REMOVED: return ResultCode::ErrorDeviceRemoved;
        default:                        return ResultCode::ErrorUnknown;
        }
    }

    inline static DXGI_FORMAT ConvertToFormat(Format format)
    {
        switch (format)
        {
        case Format::Unknown:           return DXGI_FORMAT_UNKNOWN;
        case Format::R8_UINT:           return DXGI_FORMAT_R8_UINT;
        case Format::R8_SINT:           return DXGI_FORMAT_R8_SINT;
        case Format::R8_UNORM:          return DXGI_FORMAT_R8_UNORM;
        case Format::R8_SNORM:          return DXGI_FORMAT_R8_SNORM;
        case Format::RG8_UINT:          return DXGI_FORMAT_R8G8_UINT;
        case Format::RG8_SINT:          return DXGI_FORMAT_R8G8_SINT;
        case Format::RG8_UNORM:         return DXGI_FORMAT_R8G8_UNORM;
        case Format::RG8_SNORM:         return DXGI_FORMAT_R8G8_SNORM;
        case Format::R16_UINT:          return DXGI_FORMAT_R16_UINT;
        case Format::R16_SINT:          return DXGI_FORMAT_R16_SINT;
        case Format::R16_UNORM:         return DXGI_FORMAT_R16_UNORM;
        case Format::R16_SNORM:         return DXGI_FORMAT_R16_SNORM;
        case Format::R16_FLOAT:         return DXGI_FORMAT_R16_FLOAT;
        case Format::BGRA4_UNORM:       return DXGI_FORMAT_B4G4R4A4_UNORM;
        case Format::B5G6R5_UNORM:      return DXGI_FORMAT_B5G6R5_UNORM;
        case Format::B5G5R5A1_UNORM:    return DXGI_FORMAT_B5G5R5A1_UNORM;
        case Format::RGBA8_UINT:        return DXGI_FORMAT_R8G8B8A8_UINT;
        case Format::RGBA8_SINT:        return DXGI_FORMAT_R8G8B8A8_SINT;
        case Format::RGBA8_UNORM:       return DXGI_FORMAT_R8G8B8A8_UNORM;
        case Format::RGBA8_SNORM:       return DXGI_FORMAT_R8G8B8A8_SNORM;
        case Format::BGRA8_UNORM:       return DXGI_FORMAT_B8G8R8A8_UNORM;
        case Format::SRGBA8_UNORM:      return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case Format::SBGRA8_UNORM:      return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        case Format::R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
        case Format::R11G11B10_FLOAT:   return DXGI_FORMAT_R11G11B10_FLOAT;
        case Format::RG16_UINT:         return DXGI_FORMAT_R16G16_UINT;
        case Format::RG16_SINT:         return DXGI_FORMAT_R16G16_SINT;
        case Format::RG16_UNORM:        return DXGI_FORMAT_R16G16_UNORM;
        case Format::RG16_SNORM:        return DXGI_FORMAT_R16G16_SNORM;
        case Format::RG16_FLOAT:        return DXGI_FORMAT_R16G16_FLOAT;
        case Format::R32_UINT:          return DXGI_FORMAT_R32_UINT;
        case Format::R32_SINT:          return DXGI_FORMAT_R32_SINT;
        case Format::R32_FLOAT:         return DXGI_FORMAT_R32_FLOAT;
        case Format::RGBA16_UINT:       return DXGI_FORMAT_R16G16B16A16_UINT;
        case Format::RGBA16_SINT:       return DXGI_FORMAT_R16G16B16A16_SINT;
        case Format::RGBA16_FLOAT:      return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case Format::RGBA16_UNORM:      return DXGI_FORMAT_R16G16B16A16_UNORM;
        case Format::RGBA16_SNORM:      return DXGI_FORMAT_R16G16B16A16_SNORM;
        case Format::RG32_UINT:         return DXGI_FORMAT_R32G32_UINT;
        case Format::RG32_SINT:         return DXGI_FORMAT_R32G32_SINT;
        case Format::RG32_FLOAT:        return DXGI_FORMAT_R32G32_FLOAT;
        case Format::RGB32_UINT:        return DXGI_FORMAT_R32G32B32_UINT;
        case Format::RGB32_SINT:        return DXGI_FORMAT_R32G32B32_SINT;
        case Format::RGB32_FLOAT:       return DXGI_FORMAT_R32G32B32_FLOAT;
        case Format::RGBA32_UINT:       return DXGI_FORMAT_R32G32B32A32_UINT;
        case Format::RGBA32_SINT:       return DXGI_FORMAT_R32G32B32A32_SINT;
        case Format::RGBA32_FLOAT:      return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case Format::D16:               return DXGI_FORMAT_D16_UNORM;
        case Format::D24S8:             return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case Format::X24G8_UINT:        return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
        case Format::D32:               return DXGI_FORMAT_D32_FLOAT;
        case Format::D32S8:             return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        case Format::X32G8_UINT:        return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
        case Format::BC1_UNORM:         return DXGI_FORMAT_BC1_UNORM;
        case Format::BC1_UNORM_SRGB:    return DXGI_FORMAT_BC1_UNORM_SRGB;
        case Format::BC2_UNORM:         return DXGI_FORMAT_BC2_UNORM;
        case Format::BC2_UNORM_SRGB:    return DXGI_FORMAT_BC2_UNORM_SRGB;
        case Format::BC3_UNORM:         return DXGI_FORMAT_BC3_UNORM;
        case Format::BC3_UNORM_SRGB:    return DXGI_FORMAT_BC3_UNORM_SRGB;
        case Format::BC4_UNORM:         return DXGI_FORMAT_BC4_UNORM;
        case Format::BC4_SNORM:         return DXGI_FORMAT_BC4_SNORM;
        case Format::BC5_UNORM:         return DXGI_FORMAT_BC5_UNORM;
        case Format::BC5_SNORM:         return DXGI_FORMAT_BC5_SNORM;
        case Format::BC6H_UFLOAT:       return DXGI_FORMAT_BC6H_UF16;
        case Format::BC6H_SFLOAT:       return DXGI_FORMAT_BC6H_SF16;
        case Format::BC7_UNORM:         return DXGI_FORMAT_BC7_UNORM;
        case Format::BC7_UNORM_SRGB:    return DXGI_FORMAT_BC7_UNORM_SRGB;
        case Format::COUNT:             return DXGI_FORMAT_UNKNOWN;
        default:                        TL_UNREACHABLE(); return DXGI_FORMAT_UNKNOWN;
        }
    }
} // namespace RHI::D3D12