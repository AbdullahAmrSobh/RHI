#pragma once

#include <vulkan/vulkan.h>

#include <RHI/Format.hpp>
#include <RHI/Assert.hpp>
#include <RHI/Result.hpp>

#include <array>

namespace Vulkan
{
    struct FormatMapping
    {
        RHI::Format rhiFormat;
        VkFormat vkFormat;
    };

    // clang-format off
    static const std::array<FormatMapping, size_t(RHI::Format::COUNT)> c_FormatMap = { {
        { RHI::Format::Unkown,           VK_FORMAT_UNDEFINED                },
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

    VkFormat ConvertFormat(RHI::Format format)
    {
        RHI_ASSERT(format < RHI::Format::COUNT);
        RHI_ASSERT(c_FormatMap[uint32_t(format)].rhiFormat == format);

        return c_FormatMap[uint32_t(format)].vkFormat;
    }

} // namespace Vulkan