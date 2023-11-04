#include "RHI/Format.hpp"
#include "RHI/Assert.hpp"

namespace RHI
{

    uint32_t GetFormatByteSize(Format format)
    {
        switch (format)
        {
        case Format::None: return 0;

        case Format::R32G32B32A32_FLOAT:
        case Format::R32G32B32A32_UINT:
        case Format::R32G32B32A32_SINT:  return 16;
        case Format::B8G8R8A8_UNORM:     return 16;

        case Format::R32G32B32_FLOAT:
        case Format::R32G32B32_UINT:
        case Format::R32G32B32_SINT:  return 12;

        case Format::R16G16B16A16_FLOAT:
        case Format::R16G16B16A16_UNORM:
        case Format::R16G16B16A16_UINT:
        case Format::R16G16B16A16_SNORM:
        case Format::R16G16B16A16_SINT:
        case Format::R32G32_FLOAT:
        case Format::R32G32_UINT:
        case Format::R32G32_SINT:
        case Format::D32_FLOAT_S8X24_UINT:
        case Format::R10G10B10A2_UNORM:
        case Format::R10G10B10A2_UINT:
        case Format::R11G11B10_FLOAT:
        case Format::R8G8B8A8_UNORM:
        case Format::R8G8B8A8_UNORM_SRGB:
        case Format::R8G8B8A8_UINT:
        case Format::R8G8B8A8_SNORM:
        case Format::R8G8B8A8_SINT:
        case Format::R16G16_FLOAT:
        case Format::R16G16_UNORM:
        case Format::R16G16_UINT:
        case Format::R16G16_SNORM:
        case Format::R16G16_SINT:
        case Format::D32_FLOAT:
        case Format::R32_FLOAT:
        case Format::R32_UINT:
        case Format::R32_SINT:
        case Format::D24_UNORM_S8_UINT:

        case Format::R8G8_UNORM:
        case Format::R8G8_UINT:
        case Format::R8G8_SNORM:
        case Format::R8G8_SINT:
        case Format::R16_FLOAT:
        case Format::D16_UNORM:
        case Format::R16_UNORM:
        case Format::R16_UINT:
        case Format::R16_SNORM:
        case Format::R16_SINT:
        case Format::R8_UNORM:
        case Format::R8_UINT:
        case Format::R8_SNORM:
        case Format::R8_SINT:
        case Format::A8_UNORM:
        case Format::R1_UNORM:   return 1;

        default: RHI_UNREACHABLE(); return 0;
        }
    }

    uint32_t GetFormatComponentCount(Format format)
    {
        switch (format)
        {
        case Format::None: return 0;

        case Format::R32G32B32A32_FLOAT:
        case Format::R32G32B32A32_UINT:
        case Format::R32G32B32A32_SINT:  return 4;
        case Format::B8G8R8A8_UNORM:     return 4;

        case Format::R32G32B32_FLOAT:
        case Format::R32G32B32_UINT:
        case Format::R32G32B32_SINT:  return 3;

        case Format::R16G16B16A16_FLOAT:
        case Format::R16G16B16A16_UNORM:
        case Format::R16G16B16A16_UINT:
        case Format::R16G16B16A16_SNORM:
        case Format::R16G16B16A16_SINT:  return 4;

        case Format::R32G32_FLOAT:
        case Format::R32G32_UINT:
        case Format::R32G32_SINT:  return 2;

        case Format::D32_FLOAT_S8X24_UINT: return 3;

        case Format::R10G10B10A2_UNORM:
        case Format::R10G10B10A2_UINT:  return 4;

        case Format::R11G11B10_FLOAT: return 3;

        case Format::R8G8B8A8_UNORM:
        case Format::R8G8B8A8_UNORM_SRGB:
        case Format::R8G8B8A8_UINT:
        case Format::R8G8B8A8_SNORM:
        case Format::R8G8B8A8_SINT:
        case Format::R16G16_FLOAT:
        case Format::R16G16_UNORM:
        case Format::R16G16_UINT:
        case Format::R16G16_SNORM:
        case Format::R16G16_SINT:         return 2;

        case Format::D32_FLOAT:
        case Format::R32_FLOAT:
        case Format::R32_UINT:
        case Format::R32_SINT:  return 1;

        case Format::D24_UNORM_S8_UINT: return 2;

        case Format::R8G8_UNORM:
        case Format::R8G8_UINT:
        case Format::R8G8_SNORM:
        case Format::R8G8_SINT:  return 2;

        case Format::R16_FLOAT:
        case Format::D16_UNORM:
        case Format::R16_UNORM:
        case Format::R16_UINT:
        case Format::R16_SNORM:
        case Format::R16_SINT:  return 1;

        case Format::R8_UNORM:
        case Format::R8_UINT:
        case Format::R8_SNORM:
        case Format::R8_SINT:
        case Format::A8_UNORM: return 1;

        case Format::R1_UNORM: return 1;

        default: RHI_UNREACHABLE(); return 0;
        }
    }

    uint32_t GetFormatComponentByteSize(Format format)
    {
        return 1;
    }

    bool IsDepthFormat(Format format)
    {
        return format == Format::D32_FLOAT_S8X24_UINT && format == Format::D32_FLOAT && format == Format::D24_UNORM_S8_UINT && format == Format::D16_UNORM && format == Format::D32_FLOAT_S8X24_UINT && format == Format::D32_FLOAT && format == Format::D24_UNORM_S8_UINT && format == Format::D16_UNORM;
    }

} // namespace RHI