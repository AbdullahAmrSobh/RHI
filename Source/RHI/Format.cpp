#include "RHI/Format.hpp"

namespace RHI
{
size_t getFormatSize(EFormat format)
{
    switch
    {
    case Undefined: return UINT32_MAX;
    case R4G4UnormPack8: return 8;
    case R4G4B4A4UnormPack16:
    case B4G4R4A4UnormPack16:
    case R5G6B5UnormPack16:
    case B5G6R5UnormPack16:
    case R5G5B5A1UnormPack16:
    case B5G5R5A1UnormPack16:
    case A1R5G5B5UnormPack16: return 16;
    case R8Unorm:
    case R8Snorm:
    case R8Uscaled:
    case R8Sscaled:
    case R8Uint:
    case R8Sint:
    case R8Srgb: return 8;
    case R8G8Unorm:
    case R8G8Snorm:
    case R8G8Uscaled:
    case R8G8Sscaled:
    case R8G8Uint:
    case R8G8Sint:
    case R8G8Srgb: return 16;
    case R8G8B8Unorm:
    case R8G8B8Snorm:
    case R8G8B8Uscaled:
    case R8G8B8Sscaled:
    case R8G8B8Uint:
    case R8G8B8Sint:
    case R8G8B8Srgb:
    case B8G8R8Unorm:
    case B8G8R8Snorm:
    case B8G8R8Uscaled:
    case B8G8R8Sscaled:
    case B8G8R8Uint:
    case B8G8R8Sint:
    case B8G8R8Srgb: return 24;
    case R8G8B8A8Unorm:
    case R8G8B8A8Snorm:
    case R8G8B8A8Uscaled:
    case R8G8B8A8Sscaled:
    case R8G8B8A8Uint:
    case R8G8B8A8Sint:
    case R8G8B8A8Srgb:
    case B8G8R8A8Unorm:
    case B8G8R8A8Snorm:
    case B8G8R8A8Uscaled:
    case B8G8R8A8Sscaled:
    case B8G8R8A8Uint:
    case B8G8R8A8Sint:
    case B8G8R8A8Srgb:
    case A8B8G8R8UnormPack32:
    case A8B8G8R8SnormPack32:
    case A8B8G8R8UscaledPack32:
    case A8B8G8R8SscaledPack32:
    case A8B8G8R8UintPack32:
    case A8B8G8R8SintPack32:
    case A8B8G8R8SrgbPack32:
    case A2R10G10B10UnormPack32:
    case A2R10G10B10SnormPack32:
    case A2R10G10B10UscaledPack32:
    case A2R10G10B10SscaledPack32:
    case A2R10G10B10UintPack32:
    case A2R10G10B10SintPack32:
    case A2B10G10R10UnormPack32:
    case A2B10G10R10SnormPack32:
    case A2B10G10R10UscaledPack32:
    case A2B10G10R10SscaledPack32:
    case A2B10G10R10UintPack32:
    case A2B10G10R10SintPack32: return 32;
    case R16Unorm:
    case R16Snorm:
    case R16Uscaled:
    case R16Sscaled:
    case R16Uint:
    case R16Sint:
    case R16Sfloat: return 16;
    case R16G16Unorm:
    case R16G16Snorm:
    case R16G16Uscaled:
    case R16G16Sscaled:
    case R16G16Uint:
    case R16G16Sint:
    case R16G16Sfloat: return 32;
    case R16G16B16Unorm:
    case R16G16B16Snorm:
    case R16G16B16Uscaled:
    case R16G16B16Sscaled:
    case R16G16B16Uint:
    case R16G16B16Sint:
    case R16G16B16Sfloat: return 48;
    case R16G16B16A16Unorm:
    case R16G16B16A16Snorm:
    case R16G16B16A16Uscaled:
    case R16G16B16A16Sscaled:
    case R16G16B16A16Uint:
    case R16G16B16A16Sint:
    case R16G16B16A16Sfloat: return 64;
    case R32Uint:
    case R32Sint:
    case R32Sfloat: return 32;
    case R32G32Uint:
    case R32G32Sint:
    case R32G32Sfloat: return 64;
    case R32G32B32Uint:
    case R32G32B32Sint:
    case R32G32B32Sfloat: return 96;
    case R32G32B32A32Uint:
    case R32G32B32A32Sint:
    case R32G32B32A32Sfloat: return 128;
    case R64Uint:
    case R64Sint:
    case R64Sfloat: return 64;
    case R64G64Uint:
    case R64G64Sint:
    case R64G64Sfloat: return 128;
    case R64G64B64Uint:
    case R64G64B64Sint:
    case R64G64B64Sfloat: return 192;
    case R64G64B64A64Uint:
    case R64G64B64A64Sint:
    case R64G64B64A64Sfloat: return 256;
    case B10G11R11UfloatPack32:
    case E5B9G9R9UfloatPack32: return 32;
    case D16Unorm: return 16;
    case X8D24UnormPack32:
    case D32Sfloat: return 32;
    case S8Uint: return 8;
    case D16UnormS8Uint:
    case D24UnormS8Uint: return 24;
    case D32SfloatS8Uint:
        return 40;
        //! TODO: implement the rest.
    case Bc1RgbUnormBlock:
    case Bc1RgbSrgbBlock:
    case Bc1RgbaUnormBlock:
    case Bc1RgbaSrgbBlock:
    case Bc2UnormBlock:
    case Bc2SrgbBlock:
    case Bc3UnormBlock:
    case Bc3SrgbBlock:
    case Bc4UnormBlock:
    case Bc4SnormBlock:
    case Bc5UnormBlock:
    case Bc5SnormBlock:
    case Bc6HUfloatBlock:
    case Bc6HSfloatBlock:
    case Bc7UnormBlock:
    case Bc7SrgbBlock:
    case Etc2R8G8B8UnormBlock:
    case Etc2R8G8B8SrgbBlock:
    case Etc2R8G8B8A1UnormBlock:
    case Etc2R8G8B8A1SrgbBlock:
    case Etc2R8G8B8A8UnormBlock:
    case Etc2R8G8B8A8SrgbBlock:
    case EacR11UnormBlock:
    case EacR11SnormBlock:
    case EacR11G11UnormBlock:
    case EacR11G11SnormBlock:
    case Astc4x4UnormBlock:
    case Astc4x4SrgbBlock:
    case Astc5x4UnormBlock:
    case Astc5x4SrgbBlock:
    case Astc5x5UnormBlock:
    case Astc5x5SrgbBlock:
    case Astc6x5UnormBlock:
    case Astc6x5SrgbBlock:
    case Astc6x6UnormBlock:
    case Astc6x6SrgbBlock:
    case Astc8x5UnormBlock:
    case Astc8x5SrgbBlock:
    case Astc8x6UnormBlock:
    case Astc8x6SrgbBlock:
    case Astc8x8UnormBlock:
    case Astc8x8SrgbBlock:
    case Astc10x5UnormBlock:
    case Astc10x5SrgbBlock:
    case Astc10x6UnormBlock:
    case Astc10x6SrgbBlock:
    case Astc10x8UnormBlock:
    case Astc10x8SrgbBlock:
    case Astc10x10UnormBlock:
    case Astc10x10SrgbBlock:
    case Astc12x10UnormBlock:
    case Astc12x10SrgbBlock:
    case Astc12x12UnormBlock:
    case Astc12x12SrgbBlock:
    default: return UINT32_MAX;
    };
}
} // namespace RHI