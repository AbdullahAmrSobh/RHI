#pragma once

#include <cstdint>

namespace RHI
{

    enum class Format
    {
        None = 0,

        R32G32B32A32_FLOAT,
        R32G32B32A32_UINT,
        R32G32B32A32_SINT,

        R32G32B32_FLOAT,
        R32G32B32_UINT,
        R32G32B32_SINT,

        R16G16B16A16_FLOAT,
        R16G16B16A16_UNORM,
        R16G16B16A16_UINT,
        R16G16B16A16_SNORM,
        R16G16B16A16_SINT,

        R32G32_FLOAT,
        R32G32_UINT,
        R32G32_SINT,

        D32_FLOAT_S8X24_UINT,

        R10G10B10A2_UNORM,
        R10G10B10A2_UINT,

        R11G11B10_FLOAT,

        R8G8B8A8_UNORM,
        R8G8B8A8_UNORM_SRGB,
        R8G8B8A8_UINT,
        R8G8B8A8_SNORM,
        R8G8B8A8_SINT,

        R16G16_FLOAT,
        R16G16_UNORM,
        R16G16_UINT,
        R16G16_SNORM,
        R16G16_SINT,

        D32_FLOAT,
        R32_FLOAT,
        R32_UINT,
        R32_SINT,

        D24_UNORM_S8_UINT,

        R8G8_UNORM,
        R8G8_UNORM_SRGB,
        R8G8_UINT,
        R8G8_SNORM,
        R8G8_SINT,

        R16_FLOAT,
        D16_UNORM,
        R16_UNORM,
        R16_UINT,
        R16_SNORM,
        R16_SINT,

        R8_UNORM,
        R8_UNORM_SRGB,
        R8_UINT,
        R8_SNORM,
        R8_SINT,
        A8_UNORM,
        R1_UNORM,

        B8G8R8A8_UNORM, // swapchain color format
    };

    uint32_t GetFormatByteSize(Format format);

    uint32_t GetFormatComponentCount(Format format);

    uint32_t GetFormatComponentByteSize(Format format);

    bool IsDepthFormat(Format format);

} // namespace RHI