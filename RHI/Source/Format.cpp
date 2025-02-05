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
        {  "Unknown",          Format::Unknown,           0,  0, FormatType::Integer,      false, false, false, false, false, false, false, false },
        {  "R8_UINT",          Format::R8_UINT,           1,  1, FormatType::Integer,      true,  false, false, false, false, false, false, false },
        {  "R8_SINT",          Format::R8_SINT,           1,  1, FormatType::Integer,      true,  false, false, false, false, false, true,  false },
        {  "R8_UNORM",         Format::R8_UNORM,          1,  1, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        {  "R8_SNORM",         Format::R8_SNORM,          1,  1, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        {  "RG8_UINT",         Format::RG8_UINT,          2,  1, FormatType::Integer,      true,  true,  false, false, false, false, false, false },
        {  "RG8_SINT",         Format::RG8_SINT,          2,  1, FormatType::Integer,      true,  true,  false, false, false, false, true,  false },
        {  "RG8_UNORM",        Format::RG8_UNORM,         2,  1, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        {  "RG8_SNORM",        Format::RG8_SNORM,         2,  1, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        {  "R16_UINT",         Format::R16_UINT,          2,  1, FormatType::Integer,      true,  false, false, false, false, false, false, false },
        {  "R16_SINT",         Format::R16_SINT,          2,  1, FormatType::Integer,      true,  false, false, false, false, false, true,  false },
        {  "R16_UNORM",        Format::R16_UNORM,         2,  1, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        {  "R16_SNORM",        Format::R16_SNORM,         2,  1, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        {  "R16_FLOAT",        Format::R16_FLOAT,         2,  1, FormatType::Float,        true,  false, false, false, false, false, true,  false },
        {  "BGRA4_UNORM",      Format::BGRA4_UNORM,       2,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "B5G6R5_UNORM",     Format::B5G6R5_UNORM,      2,  1, FormatType::Normalized,   true,  true,  true,  false, false, false, false, false },
        {  "B5G5R5A1_UNORM",   Format::B5G5R5A1_UNORM,    2,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "RGBA8_UINT",       Format::RGBA8_UINT,        4,  1, FormatType::Integer,      true,  true,  true,  true,  false, false, false, false },
        {  "RGBA8_SINT",       Format::RGBA8_SINT,        4,  1, FormatType::Integer,      true,  true,  true,  true,  false, false, true,  false },
        {  "RGBA8_UNORM",      Format::RGBA8_UNORM,       4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "RGBA8_SNORM",      Format::RGBA8_SNORM,       4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "BGRA8_UNORM",      Format::BGRA8_UNORM,       4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "SRGBA8_UNORM",     Format::SRGBA8_UNORM,      4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        {  "SBGRA8_UNORM",     Format::SBGRA8_UNORM,      4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "R10G10B10A2_UNORM",Format::R10G10B10A2_UNORM, 4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "R11G11B10_FLOAT",  Format::R11G11B10_FLOAT,   4,  1, FormatType::Float,        true,  true,  true,  false, false, false, false, false },
        {  "RG16_UINT",        Format::RG16_UINT,         4,  1, FormatType::Integer,      true,  true,  false, false, false, false, false, false },
        {  "RG16_SINT",        Format::RG16_SINT,         4,  1, FormatType::Integer,      true,  true,  false, false, false, false, true,  false },
        {  "RG16_UNORM",       Format::RG16_UNORM,        4,  1, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        {  "RG16_SNORM",       Format::RG16_SNORM,        4,  1, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        {  "RG16_FLOAT",       Format::RG16_FLOAT,        4,  1, FormatType::Float,        true,  true,  false, false, false, false, true,  false },
        {  "R32_UINT",         Format::R32_UINT,          4,  1, FormatType::Integer,      true,  false, false, false, false, false, false, false },
        {  "R32_SINT",         Format::R32_SINT,          4,  1, FormatType::Integer,      true,  false, false, false, false, false, true,  false },
        {  "R32_FLOAT",        Format::R32_FLOAT,         4,  1, FormatType::Float,        true,  false, false, false, false, false, true,  false },
        {  "RGBA16_UINT",      Format::RGBA16_UINT,       8,  1, FormatType::Integer,      true,  true,  true,  true,  false, false, false, false },
        {  "RGBA16_SINT",      Format::RGBA16_SINT,       8,  1, FormatType::Integer,      true,  true,  true,  true,  false, false, true,  false },
        {  "RGBA16_FLOAT",     Format::RGBA16_FLOAT,      8,  1, FormatType::Float,        true,  true,  true,  true,  false, false, true,  false },
        {  "RGBA16_UNORM",     Format::RGBA16_UNORM,      8,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "RGBA16_SNORM",     Format::RGBA16_SNORM,      8,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "RG32_UINT",        Format::RG32_UINT,         8,  1, FormatType::Integer,      true,  true,  false, false, false, false, false, false },
        {  "RG32_SINT",        Format::RG32_SINT,         8,  1, FormatType::Integer,      true,  true,  false, false, false, false, true,  false },
        {  "RG32_FLOAT",       Format::RG32_FLOAT,        8,  1, FormatType::Float,        true,  true,  false, false, false, false, true,  false },
        {  "RGB32_UINT",       Format::RGB32_UINT,        12, 1, FormatType::Integer,      true,  true,  true,  false, false, false, false, false },
        {  "RGB32_SINT",       Format::RGB32_SINT,        12, 1, FormatType::Integer,      true,  true,  true,  false, false, false, true,  false },
        {  "RGB32_FLOAT",      Format::RGB32_FLOAT,       12, 1, FormatType::Float,        true,  true,  true,  false, false, false, true,  false },
        {  "RGBA32_UINT",      Format::RGBA32_UINT,       16, 1, FormatType::Integer,      true,  true,  true,  true,  false, false, false, false },
        {  "RGBA32_SINT",      Format::RGBA32_SINT,       16, 1, FormatType::Integer,      true,  true,  true,  true,  false, false, true,  false },
        {  "RGBA32_FLOAT",     Format::RGBA32_FLOAT,      16, 1, FormatType::Float,        true,  true,  true,  true,  false, false, true,  false },
        {  "D16",              Format::D16,               2,  1, FormatType::DepthStencil, false, false, false, false, true,  false, false, false },
        {  "D24S8",            Format::D24S8,             4,  1, FormatType::DepthStencil, false, false, false, false, true,  true,  false, false },
        {  "X24G8_UINT",       Format::X24G8_UINT,        4,  1, FormatType::Integer,      false, false, false, false, false, true,  false, false },
        {  "D32",              Format::D32,               4,  1, FormatType::DepthStencil, false, false, false, false, true,  false, false, false },
        {  "D32S8",            Format::D32S8,             8,  1, FormatType::DepthStencil, false, false, false, false, true,  true,  false, false },
        {  "X32G8_UINT",       Format::X32G8_UINT,        8,  1, FormatType::Integer,      false, false, false, false, false, true,  false, false },
        {  "BC1_UNORM",        Format::BC1_UNORM,         8,  4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "BC1_UNORM_SRGB",   Format::BC1_UNORM_SRGB,    8,  4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        {  "BC2_UNORM",        Format::BC2_UNORM,         16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "BC2_UNORM_SRGB",   Format::BC2_UNORM_SRGB,    16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        {  "BC3_UNORM",        Format::BC3_UNORM,         16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "BC3_UNORM_SRGB",   Format::BC3_UNORM_SRGB,    16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        {  "BC4_UNORM",        Format::BC4_UNORM,         8,  4, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        {  "BC4_SNORM",        Format::BC4_SNORM,         8,  4, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        {  "BC5_UNORM",        Format::BC5_UNORM,         16, 4, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        {  "BC5_SNORM",        Format::BC5_SNORM,         16, 4, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        {  "BC6H_UFLOAT",      Format::BC6H_UFLOAT,       16, 4, FormatType::Float,        true,  true,  true,  false, false, false, false, false },
        {  "BC6H_SFLOAT",      Format::BC6H_SFLOAT,       16, 4, FormatType::Float,        true,  true,  true,  false, false, false, true,  false },
        {  "BC7_UNORM",        Format::BC7_UNORM,         16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "BC7_UNORM_SRGB",   Format::BC7_UNORM_SRGB,    16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
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