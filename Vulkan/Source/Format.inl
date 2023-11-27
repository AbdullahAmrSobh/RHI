#pragma once

#include <RHI/Common/Assert.hpp>
#include <RHI/Format.hpp>

#include <array>
#include <vulkan/vulkan.h>

namespace Vulkan
{
    struct FormatMapping
    {
        RHI::Format rhiFormat;
        VkFormat    vkFormat;
    };

    // clang-format off
    static const std::array<FormatMapping, size_t(RHI::Format::COUNT)> c_formatLUT = { {
        { RHI::Format::Unknown,           VK_FORMAT_UNDEFINED                },
        { RHI::Format::R8_UINT,           VK_FORMAT_R8_UINT                  },
        { RHI::Format::R8_SINT,           VK_FORMAT_R8_SINT                  },
        { RHI::Format::R8_UNORM,          VK_FORMAT_R8_UNORM                 },
        { RHI::Format::R8_SNORM,          VK_FORMAT_R8_SNORM                 },
        { RHI::Format::RG8_UINT,          VK_FORMAT_R8G8_UINT                },
        { RHI::Format::RG8_SINT,          VK_FORMAT_R8G8_SINT                },
        { RHI::Format::RG8_UNORM,         VK_FORMAT_R8G8_UNORM               },
        { RHI::Format::RG8_SNORM,         VK_FORMAT_R8G8_SNORM               },
        { RHI::Format::R16_UINT,          VK_FORMAT_R16_UINT                 },
        { RHI::Format::R16_SINT,          VK_FORMAT_R16_SINT                 },
        { RHI::Format::R16_UNORM,         VK_FORMAT_R16_UNORM                },
        { RHI::Format::R16_SNORM,         VK_FORMAT_R16_SNORM                },
        { RHI::Format::R16_FLOAT,         VK_FORMAT_R16_SFLOAT               },
        { RHI::Format::BGRA4_UNORM,       VK_FORMAT_B4G4R4A4_UNORM_PACK16    },
        { RHI::Format::B5G6R5_UNORM,      VK_FORMAT_B5G6R5_UNORM_PACK16      },
        { RHI::Format::B5G5R5A1_UNORM,    VK_FORMAT_B5G5R5A1_UNORM_PACK16    },
        { RHI::Format::RGBA8_UINT,        VK_FORMAT_R8G8B8A8_UINT            },
        { RHI::Format::RGBA8_SINT,        VK_FORMAT_R8G8B8A8_SINT            },
        { RHI::Format::RGBA8_UNORM,       VK_FORMAT_R8G8B8A8_UNORM           },
        { RHI::Format::RGBA8_SNORM,       VK_FORMAT_R8G8B8A8_SNORM           },
        { RHI::Format::BGRA8_UNORM,       VK_FORMAT_B8G8R8A8_UNORM           },
        { RHI::Format::SRGBA8_UNORM,      VK_FORMAT_R8G8B8A8_SRGB            },
        { RHI::Format::SBGRA8_UNORM,      VK_FORMAT_B8G8R8A8_SRGB            },
        { RHI::Format::R10G10B10A2_UNORM, VK_FORMAT_A2B10G10R10_UNORM_PACK32 },
        { RHI::Format::R11G11B10_FLOAT,   VK_FORMAT_B10G11R11_UFLOAT_PACK32  },
        { RHI::Format::RG16_UINT,         VK_FORMAT_R16G16_UINT              },
        { RHI::Format::RG16_SINT,         VK_FORMAT_R16G16_SINT              },
        { RHI::Format::RG16_UNORM,        VK_FORMAT_R16G16_UNORM             },
        { RHI::Format::RG16_SNORM,        VK_FORMAT_R16G16_SNORM             },
        { RHI::Format::RG16_FLOAT,        VK_FORMAT_R16G16_SFLOAT            },
        { RHI::Format::R32_UINT,          VK_FORMAT_R32_UINT                 },
        { RHI::Format::R32_SINT,          VK_FORMAT_R32_SINT                 },
        { RHI::Format::R32_FLOAT,         VK_FORMAT_R32_SFLOAT               },
        { RHI::Format::RGBA16_UINT,       VK_FORMAT_R16G16B16A16_UINT        },
        { RHI::Format::RGBA16_SINT,       VK_FORMAT_R16G16B16A16_SINT        },
        { RHI::Format::RGBA16_FLOAT,      VK_FORMAT_R16G16B16A16_SFLOAT      },
        { RHI::Format::RGBA16_UNORM,      VK_FORMAT_R16G16B16A16_UNORM       },
        { RHI::Format::RGBA16_SNORM,      VK_FORMAT_R16G16B16A16_SNORM       },
        { RHI::Format::RG32_UINT,         VK_FORMAT_R32G32_UINT              },
        { RHI::Format::RG32_SINT,         VK_FORMAT_R32G32_SINT              },
        { RHI::Format::RG32_FLOAT,        VK_FORMAT_R32G32_SFLOAT            },
        { RHI::Format::RGB32_UINT,        VK_FORMAT_R32G32B32_UINT           },
        { RHI::Format::RGB32_SINT,        VK_FORMAT_R32G32B32_SINT           },
        { RHI::Format::RGB32_FLOAT,       VK_FORMAT_R32G32B32_SFLOAT         },
        { RHI::Format::RGBA32_UINT,       VK_FORMAT_R32G32B32A32_UINT        },
        { RHI::Format::RGBA32_SINT,       VK_FORMAT_R32G32B32A32_SINT        },
        { RHI::Format::RGBA32_FLOAT,      VK_FORMAT_R32G32B32A32_SFLOAT      },
        { RHI::Format::D16,               VK_FORMAT_D16_UNORM                },
        { RHI::Format::D24S8,             VK_FORMAT_D24_UNORM_S8_UINT        },
        { RHI::Format::X24G8_UINT,        VK_FORMAT_D24_UNORM_S8_UINT        },
        { RHI::Format::D32,               VK_FORMAT_D32_SFLOAT               },
        { RHI::Format::D32S8,             VK_FORMAT_D32_SFLOAT_S8_UINT       },
        { RHI::Format::X32G8_UINT,        VK_FORMAT_D32_SFLOAT_S8_UINT       },
        { RHI::Format::BC1_UNORM,         VK_FORMAT_BC1_RGBA_UNORM_BLOCK     },
        { RHI::Format::BC1_UNORM_SRGB,    VK_FORMAT_BC1_RGBA_SRGB_BLOCK      },
        { RHI::Format::BC2_UNORM,         VK_FORMAT_BC2_UNORM_BLOCK          },
        { RHI::Format::BC2_UNORM_SRGB,    VK_FORMAT_BC2_SRGB_BLOCK           },
        { RHI::Format::BC3_UNORM,         VK_FORMAT_BC3_UNORM_BLOCK          },
        { RHI::Format::BC3_UNORM_SRGB,    VK_FORMAT_BC3_SRGB_BLOCK           },
        { RHI::Format::BC4_UNORM,         VK_FORMAT_BC4_UNORM_BLOCK          },
        { RHI::Format::BC4_SNORM,         VK_FORMAT_BC4_SNORM_BLOCK          },
        { RHI::Format::BC5_UNORM,         VK_FORMAT_BC5_UNORM_BLOCK          },
        { RHI::Format::BC5_SNORM,         VK_FORMAT_BC5_SNORM_BLOCK          },
        { RHI::Format::BC6H_UFLOAT,       VK_FORMAT_BC6H_UFLOAT_BLOCK        },
        { RHI::Format::BC6H_SFLOAT,       VK_FORMAT_BC6H_SFLOAT_BLOCK        },
        { RHI::Format::BC7_UNORM,         VK_FORMAT_BC7_UNORM_BLOCK          },
        { RHI::Format::BC7_UNORM_SRGB,    VK_FORMAT_BC7_SRGB_BLOCK           },
    } };

    // clang-format on

    VkFormat ConvertFormat(RHI::Format format)
    {
        RHI_ASSERT(format < RHI::Format::COUNT);
        RHI_ASSERT(c_formatLUT[uint32_t(format)].rhiFormat == format);

        return c_formatLUT[uint32_t(format)].vkFormat;
    }

    RHI::Format ConvertFormat(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_UNDEFINED:                return RHI::Format::Unknown;
        case VK_FORMAT_R8_UINT:                  return RHI::Format::R8_UINT;
        case VK_FORMAT_R8_SINT:                  return RHI::Format::R8_SINT;
        case VK_FORMAT_R8_UNORM:                 return RHI::Format::R8_UNORM;
        case VK_FORMAT_R8_SNORM:                 return RHI::Format::R8_SNORM;
        case VK_FORMAT_R8G8_UINT:                return RHI::Format::RG8_UINT;
        case VK_FORMAT_R8G8_SINT:                return RHI::Format::RG8_SINT;
        case VK_FORMAT_R8G8_UNORM:               return RHI::Format::RG8_UNORM;
        case VK_FORMAT_R8G8_SNORM:               return RHI::Format::RG8_SNORM;
        case VK_FORMAT_R16_UINT:                 return RHI::Format::R16_UINT;
        case VK_FORMAT_R16_SINT:                 return RHI::Format::R16_SINT;
        case VK_FORMAT_R16_UNORM:                return RHI::Format::R16_UNORM;
        case VK_FORMAT_R16_SNORM:                return RHI::Format::R16_SNORM;
        case VK_FORMAT_R16_SFLOAT:               return RHI::Format::R16_FLOAT;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:    return RHI::Format::BGRA4_UNORM;
        case VK_FORMAT_B5G6R5_UNORM_PACK16:      return RHI::Format::B5G6R5_UNORM;
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:    return RHI::Format::B5G5R5A1_UNORM;
        case VK_FORMAT_R8G8B8A8_UINT:            return RHI::Format::RGBA8_UINT;
        case VK_FORMAT_R8G8B8A8_SINT:            return RHI::Format::RGBA8_SINT;
        case VK_FORMAT_R8G8B8A8_UNORM:           return RHI::Format::RGBA8_UNORM;
        case VK_FORMAT_R8G8B8A8_SNORM:           return RHI::Format::RGBA8_SNORM;
        case VK_FORMAT_B8G8R8A8_UNORM:           return RHI::Format::BGRA8_UNORM;
        case VK_FORMAT_R8G8B8A8_SRGB:            return RHI::Format::SRGBA8_UNORM;
        case VK_FORMAT_B8G8R8A8_SRGB:            return RHI::Format::SBGRA8_UNORM;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return RHI::Format::R10G10B10A2_UNORM;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:  return RHI::Format::R11G11B10_FLOAT;
        case VK_FORMAT_R16G16_UINT:              return RHI::Format::RG16_UINT;
        case VK_FORMAT_R16G16_SINT:              return RHI::Format::RG16_SINT;
        case VK_FORMAT_R16G16_UNORM:             return RHI::Format::RG16_UNORM;
        case VK_FORMAT_R16G16_SNORM:             return RHI::Format::RG16_SNORM;
        case VK_FORMAT_R16G16_SFLOAT:            return RHI::Format::RG16_FLOAT;
        case VK_FORMAT_R32_UINT:                 return RHI::Format::R32_UINT;
        case VK_FORMAT_R32_SINT:                 return RHI::Format::R32_SINT;
        case VK_FORMAT_R32_SFLOAT:               return RHI::Format::R32_FLOAT;
        case VK_FORMAT_R16G16B16A16_UINT:        return RHI::Format::RGBA16_UINT;
        case VK_FORMAT_R16G16B16A16_SINT:        return RHI::Format::RGBA16_SINT;
        case VK_FORMAT_R16G16B16A16_SFLOAT:      return RHI::Format::RGBA16_FLOAT;
        case VK_FORMAT_R16G16B16A16_UNORM:       return RHI::Format::RGBA16_UNORM;
        case VK_FORMAT_R16G16B16A16_SNORM:       return RHI::Format::RGBA16_SNORM;
        case VK_FORMAT_R32G32_UINT:              return RHI::Format::RG32_UINT;
        case VK_FORMAT_R32G32_SINT:              return RHI::Format::RG32_SINT;
        case VK_FORMAT_R32G32_SFLOAT:            return RHI::Format::RG32_FLOAT;
        case VK_FORMAT_R32G32B32_UINT:           return RHI::Format::RGB32_UINT;
        case VK_FORMAT_R32G32B32_SINT:           return RHI::Format::RGB32_SINT;
        case VK_FORMAT_R32G32B32_SFLOAT:         return RHI::Format::RGB32_FLOAT;
        case VK_FORMAT_R32G32B32A32_UINT:        return RHI::Format::RGBA32_UINT;
        case VK_FORMAT_R32G32B32A32_SINT:        return RHI::Format::RGBA32_SINT;
        case VK_FORMAT_R32G32B32A32_SFLOAT:      return RHI::Format::RGBA32_FLOAT;
        case VK_FORMAT_D16_UNORM:                return RHI::Format::D16;
        case VK_FORMAT_D24_UNORM_S8_UINT:        return RHI::Format::D24S8;
        case VK_FORMAT_D32_SFLOAT:               return RHI::Format::D32;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:       return RHI::Format::D32S8;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:     return RHI::Format::BC1_UNORM;
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:      return RHI::Format::BC1_UNORM_SRGB;
        case VK_FORMAT_BC2_UNORM_BLOCK:          return RHI::Format::BC2_UNORM;
        case VK_FORMAT_BC2_SRGB_BLOCK:           return RHI::Format::BC2_UNORM_SRGB;
        case VK_FORMAT_BC3_UNORM_BLOCK:          return RHI::Format::BC3_UNORM;
        case VK_FORMAT_BC3_SRGB_BLOCK:           return RHI::Format::BC3_UNORM_SRGB;
        case VK_FORMAT_BC4_UNORM_BLOCK:          return RHI::Format::BC4_UNORM;
        case VK_FORMAT_BC4_SNORM_BLOCK:          return RHI::Format::BC4_SNORM;
        case VK_FORMAT_BC5_UNORM_BLOCK:          return RHI::Format::BC5_UNORM;
        case VK_FORMAT_BC5_SNORM_BLOCK:          return RHI::Format::BC5_SNORM;
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:        return RHI::Format::BC6H_UFLOAT;
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:        return RHI::Format::BC6H_SFLOAT;
        case VK_FORMAT_BC7_UNORM_BLOCK:          return RHI::Format::BC7_UNORM;
        case VK_FORMAT_BC7_SRGB_BLOCK:           return RHI::Format::BC7_UNORM_SRGB;
        default:                                 RHI_UNREACHABLE(); return RHI::Format::Unknown;
        }
    }

} // namespace Vulkan