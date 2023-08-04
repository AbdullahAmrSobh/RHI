#include "RHI/Format.hpp"

namespace RHI
{

uint32_t GetFormatByteSize(Format format)
{
    switch (format)
    {
        case RHI::Format::RGBA8: return 4;

        case RHI::Format::RGBA32Float: return 4 * 4;
        case RHI::Format::RGBA32Snorm: return 4 * 4;
        case RHI::Format::RGBA32Unorm: return 4 * 4;
        case RHI::Format::RGBA32Sscaled: return 4 * 4;
        case RHI::Format::RGBA32Uscaled: return 4 * 4;
        case RHI::Format::RGBA32Sint: return 4 * 4;
        case RHI::Format::RGBA32Uint: return 4 * 4;

        case RHI::Format::D24S8: return 4;
        case RHI::Format::D32: return 4;

        default: return 0;
    };
}

uint32_t GetFormatComponentCount(Format format)
{
    switch (format)
    {
        case RHI::Format::RGBA8:
        case RHI::Format::RGBA32Float:
        case RHI::Format::RGBA32Snorm:
        case RHI::Format::RGBA32Unorm:
        case RHI::Format::RGBA32Sscaled:
        case RHI::Format::RGBA32Uscaled:
        case RHI::Format::RGBA32Sint:
        case RHI::Format::RGBA32Uint: return 4;

        default: return 0;
    };
}

}  // namespace RHI