#pragma once

#include <cstdint>

namespace RHI
{

// Format of a Texel unit
enum class Format
{
    None = 0,

    RGBA8,
    RGBA32Float,
    RGBA32Snorm,
    RGBA32Unorm,
    RGBA32Sscaled,
    RGBA32Uscaled,
    RGBA32Sint,
    RGBA32Uint,

    // Depth formats
    D32,
    D24S8,
};

// Return the szie of the given format in bytes
uint32_t GetFormatByteSize(Format format);

// Return the number of component/channels in the given format
uint32_t GetFormatComponentCount(Format format);

// Return the memory alignment of the format
uint32_t GetFormatDimensionAlignment(Format format);

}  // namespace RHI
