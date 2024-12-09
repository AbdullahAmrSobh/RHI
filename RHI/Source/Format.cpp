#include "RHI/Format.hpp"

#include "RHI/Image.hpp"

#include <TL/Assert.hpp>

namespace RHI
{
    // clang-format off
    // Copied from nvrhi
    // Format mapping table. The rows must be in the exactly same order as Format enum members are defined.
    static const FormatInfo k_FormatInfoLUT[] = {
        // format                    name              bytes blk         type               red   green   blue  alpha  depth  stencl signed  srgb
        { Format::Unknown,           "Unknown",           0,  0, FormatType::Integer,      false, false, false, false, false, false, false, false },
        { Format::R8_UINT,           "R8_UINT",           1,  1, FormatType::Integer,      true,  false, false, false, false, false, false, false },
        { Format::R8_SINT,           "R8_SINT",           1,  1, FormatType::Integer,      true,  false, false, false, false, false, true,  false },
        { Format::R8_UNORM,          "R8_UNORM",          1,  1, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::R8_SNORM,          "R8_SNORM",          1,  1, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::RG8_UINT,          "RG8_UINT",          2,  1, FormatType::Integer,      true,  true,  false, false, false, false, false, false },
        { Format::RG8_SINT,          "RG8_SINT",          2,  1, FormatType::Integer,      true,  true,  false, false, false, false, true,  false },
        { Format::RG8_UNORM,         "RG8_UNORM",         2,  1, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::RG8_SNORM,         "RG8_SNORM",         2,  1, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::R16_UINT,          "R16_UINT",          2,  1, FormatType::Integer,      true,  false, false, false, false, false, false, false },
        { Format::R16_SINT,          "R16_SINT",          2,  1, FormatType::Integer,      true,  false, false, false, false, false, true,  false },
        { Format::R16_UNORM,         "R16_UNORM",         2,  1, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::R16_SNORM,         "R16_SNORM",         2,  1, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::R16_FLOAT,         "R16_FLOAT",         2,  1, FormatType::Float,        true,  false, false, false, false, false, true,  false },
        { Format::BGRA4_UNORM,       "BGRA4_UNORM",       2,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::B5G6R5_UNORM,      "B5G6R5_UNORM",      2,  1, FormatType::Normalized,   true,  true,  true,  false, false, false, false, false },
        { Format::B5G5R5A1_UNORM,    "B5G5R5A1_UNORM",    2,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA8_UINT,        "RGBA8_UINT",        4,  1, FormatType::Integer,      true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA8_SINT,        "RGBA8_SINT",        4,  1, FormatType::Integer,      true,  true,  true,  true,  false, false, true,  false },
        { Format::RGBA8_UNORM,       "RGBA8_UNORM",       4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA8_SNORM,       "RGBA8_SNORM",       4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BGRA8_UNORM,       "BGRA8_UNORM",       4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::SRGBA8_UNORM,      "SRGBA8_UNORM",      4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { Format::SBGRA8_UNORM,      "SBGRA8_UNORM",      4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::R10G10B10A2_UNORM, "R10G10B10A2_UNORM", 4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::R11G11B10_FLOAT,   "R11G11B10_FLOAT",   4,  1, FormatType::Float,        true,  true,  true,  false, false, false, false, false },
        { Format::RG16_UINT,         "RG16_UINT",         4,  1, FormatType::Integer,      true,  true,  false, false, false, false, false, false },
        { Format::RG16_SINT,         "RG16_SINT",         4,  1, FormatType::Integer,      true,  true,  false, false, false, false, true,  false },
        { Format::RG16_UNORM,        "RG16_UNORM",        4,  1, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::RG16_SNORM,        "RG16_SNORM",        4,  1, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::RG16_FLOAT,        "RG16_FLOAT",        4,  1, FormatType::Float,        true,  true,  false, false, false, false, true,  false },
        { Format::R32_UINT,          "R32_UINT",          4,  1, FormatType::Integer,      true,  false, false, false, false, false, false, false },
        { Format::R32_SINT,          "R32_SINT",          4,  1, FormatType::Integer,      true,  false, false, false, false, false, true,  false },
        { Format::R32_FLOAT,         "R32_FLOAT",         4,  1, FormatType::Float,        true,  false, false, false, false, false, true,  false },
        { Format::RGBA16_UINT,       "RGBA16_UINT",       8,  1, FormatType::Integer,      true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA16_SINT,       "RGBA16_SINT",       8,  1, FormatType::Integer,      true,  true,  true,  true,  false, false, true,  false },
        { Format::RGBA16_FLOAT,      "RGBA16_FLOAT",      8,  1, FormatType::Float,        true,  true,  true,  true,  false, false, true,  false },
        { Format::RGBA16_UNORM,      "RGBA16_UNORM",      8,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA16_SNORM,      "RGBA16_SNORM",      8,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RG32_UINT,         "RG32_UINT",         8,  1, FormatType::Integer,      true,  true,  false, false, false, false, false, false },
        { Format::RG32_SINT,         "RG32_SINT",         8,  1, FormatType::Integer,      true,  true,  false, false, false, false, true,  false },
        { Format::RG32_FLOAT,        "RG32_FLOAT",        8,  1, FormatType::Float,        true,  true,  false, false, false, false, true,  false },
        { Format::RGB32_UINT,        "RGB32_UINT",        12, 1, FormatType::Integer,      true,  true,  true,  false, false, false, false, false },
        { Format::RGB32_SINT,        "RGB32_SINT",        12, 1, FormatType::Integer,      true,  true,  true,  false, false, false, true,  false },
        { Format::RGB32_FLOAT,       "RGB32_FLOAT",       12, 1, FormatType::Float,        true,  true,  true,  false, false, false, true,  false },
        { Format::RGBA32_UINT,       "RGBA32_UINT",       16, 1, FormatType::Integer,      true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA32_SINT,       "RGBA32_SINT",       16, 1, FormatType::Integer,      true,  true,  true,  true,  false, false, true,  false },
        { Format::RGBA32_FLOAT,      "RGBA32_FLOAT",      16, 1, FormatType::Float,        true,  true,  true,  true,  false, false, true,  false },
        { Format::D16,               "D16",               2,  1, FormatType::DepthStencil, false, false, false, false, true,  false, false, false },
        { Format::D24S8,             "D24S8",             4,  1, FormatType::DepthStencil, false, false, false, false, true,  true,  false, false },
        { Format::X24G8_UINT,        "X24G8_UINT",        4,  1, FormatType::Integer,      false, false, false, false, false, true,  false, false },
        { Format::D32,               "D32",               4,  1, FormatType::DepthStencil, false, false, false, false, true,  false, false, false },
        { Format::D32S8,             "D32S8",             8,  1, FormatType::DepthStencil, false, false, false, false, true,  true,  false, false },
        { Format::X32G8_UINT,        "X32G8_UINT",        8,  1, FormatType::Integer,      false, false, false, false, false, true,  false, false },
        { Format::BC1_UNORM,         "BC1_UNORM",         8,  4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BC1_UNORM_SRGB,    "BC1_UNORM_SRGB",    8,  4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { Format::BC2_UNORM,         "BC2_UNORM",         16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BC2_UNORM_SRGB,    "BC2_UNORM_SRGB",    16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { Format::BC3_UNORM,         "BC3_UNORM",         16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BC3_UNORM_SRGB,    "BC3_UNORM_SRGB",    16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { Format::BC4_UNORM,         "BC4_UNORM",         8,  4, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::BC4_SNORM,         "BC4_SNORM",         8,  4, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::BC5_UNORM,         "BC5_UNORM",         16, 4, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::BC5_SNORM,         "BC5_SNORM",         16, 4, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::BC6H_UFLOAT,       "BC6H_UFLOAT",       16, 4, FormatType::Float,        true,  true,  true,  false, false, false, false, false },
        { Format::BC6H_SFLOAT,       "BC6H_SFLOAT",       16, 4, FormatType::Float,        true,  true,  true,  false, false, false, true,  false },
        { Format::BC7_UNORM,         "BC7_UNORM",         16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BC7_UNORM_SRGB,    "BC7_UNORM_SRGB",    16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
    };

    // clang-format on

    const FormatInfo& GetFormatInfo(Format format)
    {
        static_assert(
            sizeof(k_FormatInfoLUT) / sizeof(FormatInfo) == size_t(Format::COUNT),
            "The format info table doesn't have the right number of elements");

        if (uint32_t(format) >= uint32_t(Format::COUNT)) return k_FormatInfoLUT[0]; // UNKNOWN

        const FormatInfo& info = k_FormatInfoLUT[uint32_t(format)];
        TL_ASSERT(info.format == format);
        return info;
    }

    uint32_t GetFormatByteSize(Format format)
    {
        auto info = GetFormatInfo(format);
        return info.bytesPerBlock;
    }

    uint32_t GetFormatComponentByteSize(Format format)
    {
        auto info = GetFormatInfo(format);
        return info.bytesPerBlock;
    }

    FormatType GetFormatType(Format format)
    {
        auto info = GetFormatInfo(format);
        return info.type;
    }

    TL::Flags<ImageAspect> GetFormatAspects(Format format)
    {
        TL::Flags<ImageAspect> flags;

        auto formatInfo = GetFormatInfo(format);

        if (formatInfo.hasDepth) flags |= ImageAspect::Depth;
        if (formatInfo.hasStencil) flags |= ImageAspect::Stencil;
        if (formatInfo.hasRed || formatInfo.hasGreen || formatInfo.hasBlue || formatInfo.hasAlpha) flags |= ImageAspect::Color;

        return flags;
    }

    ImageUsage GetImageUsage(TL::Flags<ImageAspect> aspect)
    {
        if (aspect & ImageAspect::Color) return ImageUsage::Color;
        else if (aspect & ImageAspect::DepthStencil) return ImageUsage::DepthStencil;
        else if (aspect & ImageAspect::Depth) return ImageUsage::Depth;
        else if (aspect & ImageAspect::Stencil) return ImageUsage::Stencil;
        TL_UNREACHABLE();
        return ImageUsage::None;
    }

    ImageUsage GetImageUsage(FormatInfo info)
    {
        if (info.hasDepth && info.hasStencil)
            return ImageUsage::DepthStencil;
        else if (info.hasDepth)
            return ImageUsage::Depth;
        else if (info.hasStencil)
            return ImageUsage::Stencil;
        return ImageUsage::Color;
    }
} // namespace RHI