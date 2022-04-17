#pragma once
#include "RHI/Common.hpp"
#include "RHI/Core/Expected.hpp"
#include "RHI/Core/Span.hpp"
#include <functional>

#include <cassert>

#ifdef _WIN32
#include <windows.h>
namespace RHI
{
using NativeWindowHandle = HWND;
}
#else
#warning "No native window handle type defined for this platform"
using NativeWindowHandle = void*;
#endif

#define RHI_BIT(x) (1 << x)

#define RHI_ASSERT(x) assert(x)

namespace RHI
{

using DeviceAddress = void*;

enum class EBackendType
{
    None,
    Vulkan,
    D3D12,
};

enum class EResultCode
{
    Success,
    NotReady,
    Fail,
    Timeout,
    OutOfMemory,
    DeviceOutOfMemory,

    ExtensionNotPresent,
    LayerNotPresent,

    InvalidArguments,
    FailedToCreateDevice,
    DeviceLost,

    FeatureNotAvailable,
};

enum class EDebugMessageSeverity
{
    Info,
    Warnnig,
    Error,
    Fatel,
};

enum class EShaderStageFlagBits
{
    Vertex      = 0x000001,
    Hull        = 0x000002,
    Domain      = 0x000004,
    Geometry    = 0x000008,
    Pixel       = 0x000010,
    Compute     = 0x000020,
    AllGraphics = 0x0000001F,
    All         = 0x7FFFFFFF,
    MaxEnum     = 0xFFFFFF,
};
using ShaderStageFlags = Flags<EShaderStageFlagBits>;

enum class EPipelineStage
{
    Graphics,
    Compute,
    RayTracing,
};

enum class EDescriptorType
{
    Sampler       = 1,
    Image         = 2,
    UniformBuffer = 3,
    TexelBuffer   = 4,
};

enum class EDescriptorAccessType
{
    Undefined  = 0,
    Unoredered = 1,
    ReadOnly   = 2,
};

enum class ESampleCount
{
    Count1  = 0x01,
    Count2  = 0x02,
    Count4  = 0x04,
    Count8  = 0x08,
    Count16 = 0x10,
    Count32 = 0x20,
    Count64 = 0x40,
};

enum class EPixelFormat
{
    Undefined                = 0,
    R4G4UnormPack8           = 1,
    R4G4B4A4UnormPack16      = 2,
    B4G4R4A4UnormPack16      = 3,
    R5G6B5UnormPack16        = 4,
    B5G6R5UnormPack16        = 5,
    R5G5B5A1UnormPack16      = 6,
    B5G5R5A1UnormPack16      = 7,
    A1R5G5B5UnormPack16      = 8,
    R8Unorm                  = 9,
    R8Snorm                  = 10,
    R8Uscaled                = 11,
    R8Sscaled                = 12,
    R8Uint                   = 13,
    R8Sint                   = 14,
    R8Srgb                   = 15,
    R8G8Unorm                = 16,
    R8G8Snorm                = 17,
    R8G8Uscaled              = 18,
    R8G8Sscaled              = 19,
    R8G8Uint                 = 20,
    R8G8Sint                 = 21,
    R8G8Srgb                 = 22,
    R8G8B8Unorm              = 23,
    R8G8B8Snorm              = 24,
    R8G8B8Uscaled            = 25,
    R8G8B8Sscaled            = 26,
    R8G8B8Uint               = 27,
    R8G8B8Sint               = 28,
    R8G8B8Srgb               = 29,
    B8G8R8Unorm              = 30,
    B8G8R8Snorm              = 31,
    B8G8R8Uscaled            = 32,
    B8G8R8Sscaled            = 33,
    B8G8R8Uint               = 34,
    B8G8R8Sint               = 35,
    B8G8R8Srgb               = 36,
    R8G8B8A8Unorm            = 37,
    R8G8B8A8Snorm            = 38,
    R8G8B8A8Uscaled          = 39,
    R8G8B8A8Sscaled          = 40,
    R8G8B8A8Uint             = 41,
    R8G8B8A8Sint             = 42,
    R8G8B8A8Srgb             = 43,
    B8G8R8A8Unorm            = 44,
    B8G8R8A8Snorm            = 45,
    B8G8R8A8Uscaled          = 46,
    B8G8R8A8Sscaled          = 47,
    B8G8R8A8Uint             = 48,
    B8G8R8A8Sint             = 49,
    B8G8R8A8Srgb             = 50,
    A8B8G8R8UnormPack32      = 51,
    A8B8G8R8SnormPack32      = 52,
    A8B8G8R8UscaledPack32    = 53,
    A8B8G8R8SscaledPack32    = 54,
    A8B8G8R8UintPack32       = 55,
    A8B8G8R8SintPack32       = 56,
    A8B8G8R8SrgbPack32       = 57,
    A2R10G10B10UnormPack32   = 58,
    A2R10G10B10SnormPack32   = 59,
    A2R10G10B10UscaledPack32 = 60,
    A2R10G10B10SscaledPack32 = 61,
    A2R10G10B10UintPack32    = 62,
    A2R10G10B10SintPack32    = 63,
    A2B10G10R10UnormPack32   = 64,
    A2B10G10R10SnormPack32   = 65,
    A2B10G10R10UscaledPack32 = 66,
    A2B10G10R10SscaledPack32 = 67,
    A2B10G10R10UintPack32    = 68,
    A2B10G10R10SintPack32    = 69,
    R16Unorm                 = 70,
    R16Snorm                 = 71,
    R16Uscaled               = 72,
    R16Sscaled               = 73,
    R16Uint                  = 74,
    R16Sint                  = 75,
    R16Sfloat                = 76,
    R16G16Unorm              = 77,
    R16G16Snorm              = 78,
    R16G16Uscaled            = 79,
    R16G16Sscaled            = 80,
    R16G16Uint               = 81,
    R16G16Sint               = 82,
    R16G16Sfloat             = 83,
    R16G16B16Unorm           = 84,
    R16G16B16Snorm           = 85,
    R16G16B16Uscaled         = 86,
    R16G16B16Sscaled         = 87,
    R16G16B16Uint            = 88,
    R16G16B16Sint            = 89,
    R16G16B16Sfloat          = 90,
    R16G16B16A16Unorm        = 91,
    R16G16B16A16Snorm        = 92,
    R16G16B16A16Uscaled      = 93,
    R16G16B16A16Sscaled      = 94,
    R16G16B16A16Uint         = 95,
    R16G16B16A16Sint         = 96,
    R16G16B16A16Sfloat       = 97,
    R32Uint                  = 98,
    R32Sint                  = 99,
    R32Sfloat                = 100,
    R32G32Uint               = 101,
    R32G32Sint               = 102,
    R32G32Sfloat             = 103,
    R32G32B32Uint            = 104,
    R32G32B32Sint            = 105,
    R32G32B32Sfloat          = 106,
    R32G32B32A32Uint         = 107,
    R32G32B32A32Sint         = 108,
    R32G32B32A32Sfloat       = 109,
    R64Uint                  = 110,
    R64Sint                  = 111,
    R64Sfloat                = 112,
    R64G64Uint               = 113,
    R64G64Sint               = 114,
    R64G64Sfloat             = 115,
    R64G64B64Uint            = 116,
    R64G64B64Sint            = 117,
    R64G64B64Sfloat          = 118,
    R64G64B64A64Uint         = 119,
    R64G64B64A64Sint         = 120,
    R64G64B64A64Sfloat       = 121,
    B10G11R11UfloatPack32    = 122,
    E5B9G9R9UfloatPack32     = 123,
    D16Unorm                 = 124,
    X8D24UnormPack32         = 125,
    D32Sfloat                = 126,
    S8Uint                   = 127,
    D16UnormS8Uint           = 128,
    D24UnormS8Uint           = 129,
    D32SfloatS8Uint          = 130,
    Bc1RgbUnormBlock         = 131,
    Bc1RgbSrgbBlock          = 132,
    Bc1RgbaUnormBlock        = 133,
    Bc1RgbaSrgbBlock         = 134,
    Bc2UnormBlock            = 135,
    Bc2SrgbBlock             = 136,
    Bc3UnormBlock            = 137,
    Bc3SrgbBlock             = 138,
    Bc4UnormBlock            = 139,
    Bc4SnormBlock            = 140,
    Bc5UnormBlock            = 141,
    Bc5SnormBlock            = 142,
    Bc6HUfloatBlock          = 143,
    Bc6HSfloatBlock          = 144,
    Bc7UnormBlock            = 145,
    Bc7SrgbBlock             = 146,
    Etc2R8G8B8UnormBlock     = 147,
    Etc2R8G8B8SrgbBlock      = 148,
    Etc2R8G8B8A1UnormBlock   = 149,
    Etc2R8G8B8A1SrgbBlock    = 150,
    Etc2R8G8B8A8UnormBlock   = 151,
    Etc2R8G8B8A8SrgbBlock    = 152,
    EacR11UnormBlock         = 153,
    EacR11SnormBlock         = 154,
    EacR11G11UnormBlock      = 155,
    EacR11G11SnormBlock      = 156,
    Astc4x4UnormBlock        = 157,
    Astc4x4SrgbBlock         = 158,
    Astc5x4UnormBlock        = 159,
    Astc5x4SrgbBlock         = 160,
    Astc5x5UnormBlock        = 161,
    Astc5x5SrgbBlock         = 162,
    Astc6x5UnormBlock        = 163,
    Astc6x5SrgbBlock         = 164,
    Astc6x6UnormBlock        = 165,
    Astc6x6SrgbBlock         = 166,
    Astc8x5UnormBlock        = 167,
    Astc8x5SrgbBlock         = 168,
    Astc8x6UnormBlock        = 169,
    Astc8x6SrgbBlock         = 170,
    Astc8x8UnormBlock        = 171,
    Astc8x8SrgbBlock         = 172,
    Astc10x5UnormBlock       = 173,
    Astc10x5SrgbBlock        = 174,
    Astc10x6UnormBlock       = 175,
    Astc10x6SrgbBlock        = 176,
    Astc10x8UnormBlock       = 177,
    Astc10x8SrgbBlock        = 178,
    Astc10x10UnormBlock      = 179,
    Astc10x10SrgbBlock       = 180,
    Astc12x10UnormBlock      = 181,
    Astc12x10SrgbBlock       = 182,
    Astc12x12UnormBlock      = 183,
    Astc12x12SrgbBlock       = 184,
};

enum class EBufferFormat
{
};

enum class ECompareOp
{
    Never          = 0,
    Less           = 1,
    Equal          = 2,
    LessOrEqual    = 3,
    Greater        = 4,
    NotEqual       = 5,
    GreaterOrEqual = 6,
    Always         = 7,

};

enum class EFilter
{
    Nearest = 0,
    Linear  = 1,
};

enum class ESamplerAddressMode
{
    Repeat         = 0,
    MirroredRepeat = 1,
    ClampToEdge    = 2,
    ClampToBorder  = 3,
};

enum class EBorderColor
{
    FloatTransparentBlack = 0,
    IntTransparentBlack   = 1,
    FloatOpaqueBlack      = 2,
    IntOpaqueBlack        = 3,
    FloatOpaqueWhite      = 4,
    IntOpaqueWhite        = 5,
};

// Resource Usage enums

enum class EResourceType
{
    Buffer,
    Image
};

enum class EResourceUsage
{
    Invalid            = 0,
    GpuOnly            = 1,
    CpuOnly            = 2,
    CpuToGpu           = 3,
    GpuToCpu           = 4,
    CpuCopy            = 5,
    GpuLazilyAllocated = 6,
    MaxEnum            = 0x7FFFFFFF
};

enum class EImageViewDimensions
{
    View3D      = 0x00,
    View2D      = 0x01,
    View1D      = 0x02,
    ViewCubeMap = 0x03,
};

enum class EImageUsageFlagBits
{
    Invalid                = 0x00000000,
    TransferSrc            = 0x00000001,
    TransferDst            = 0x00000002,
    Sampled                = 0x00000004,
    Storge                 = 0x00000008,
    ColorAttachment        = 0x00000010,
    DepthStencilAttachment = 0x00000020,
    TransientAttachment    = 0x00000040,
    InputAttachment        = 0x00000080,
    Present                = 0x00000100,
    SwapchainImage         = 0x00000200,
    MaxEnum                = 0x7FFFFFFF,
};
using ImageUsageFlags = Flags<EImageUsageFlagBits>;

enum class EImageViewAspectFlagBits
{
    Invalid = 0x00000000,
    Color   = 0x00000001,
    Depth   = 0x00000002,
    Stencil = 0x00000004,
    MaxEnum = 0x7FFFFFFF,
};
using ImageViewAspectFlags = Flags<EImageViewAspectFlagBits>;

enum class EImageLayout
{
    Undefined                     = 0,
    General                       = 1,
    ColorAttachmentOptimal        = 2,
    DepthStencilAttachmentOptimal = 3,
    DepthStencilReadOnlyOptimal   = 4,
    ShaderReadOnlyOptimal         = 5,
    TransferSrcOptimal            = 6,
    TransferDstOptimal            = 7,
    Preinitialized                = 8,
};

enum class EBufferUsageFlagBits
{
    Invalid            = 0x00000000,
    TransferSrc        = 0x00000001,
    TransferDst        = 0x00000002,
    UniformTexelBuffer = 0x00000004,
    StorageTexelBuffer = 0x00000008,
    UniformBuffer      = 0x00000010,
    StorageBuffer      = 0x00000020,
    IndexBuffer        = 0x00000040,
    VertexBuffer       = 0x00000080,
    IndirectBuffer     = 0x00000100,
    MaxEnum            = 0x7FFFFFFF,
};
using BufferUsageFlags = Flags<EBufferUsageFlagBits>;

struct Color
{
    Color()
        : r(0.f)
        , g(0.f)
        , b(0.f)
        , a(0.f)
    {
    }
    Color(float c)
        : r(c)
        , g(c)
        , b(c)
        , a(c)
    {
    }
    Color(float r, float g, float b, float a)
        : r(r)
        , g(g)
        , b(b)
        , a(a)
    {
    }

    bool operator==(const Color& _b) const
    {
        return r == _b.r && g == _b.g && b == _b.b && a == _b.a;
    }
    bool operator!=(const Color& _b) const
    {
        return !(*this == _b);
    }

    float r, g, b, a;
};

struct Rect
{
    Rect()
        : x(0)
        , y(0)
        , sizeX(0)
        , sizeY(0)
    {
    }

    Rect(uint32_t w, uint32_t h)
        : x(0)
        , y(0)
        , sizeX(w)
        , sizeY(h)
    {
    }

    Rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
        : x(x)
        , y(y)
        , sizeX(w)
        , sizeY(h)
    {
    }

    bool operator==(const Rect& _b) const
    {
        return x == _b.x && y == _b.y && sizeX == _b.sizeX && sizeY == _b.sizeY;
    }
    bool operator!=(const Rect& _b) const
    {
        return !(*this == _b);
    }

    uint32_t x;
    uint32_t y;
    uint32_t sizeX;
    uint32_t sizeY;
};

struct Volume
{
    Volume()
        : x(0)
        , y(0)
        , z(0)
        , sizeX(0)
        , sizeY(0)
        , sizeZ(0)
    {
    }

    Volume(uint32_t sizeX, uint32_t sizeY, uint32_t sizeZ)
        : x(0)
        , y(0)
        , z(0)
        , sizeX(sizeX)
        , sizeY(sizeY)
        , sizeZ(sizeZ)
    {
    }

    Volume(uint32_t x, uint32_t y, uint32_t z, uint32_t sizeX, uint32_t sizeY, uint32_t sizeZ)
        : x(x)
        , y(y)
        , z(z)
        , sizeX(sizeX)
        , sizeY(sizeY)
        , sizeZ(sizeZ)
    {
    }

    bool operator==(const Volume& _b) const
    {
        return x == _b.x && y == _b.y && z == _b.z && sizeX == _b.sizeX && sizeY == _b.sizeY && sizeZ == _b.sizeZ;
    }
    bool operator!=(const Volume& _b) const
    {
        return !(*this == _b);
    }

    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t sizeX;
    uint32_t sizeY;
    uint32_t sizeZ;
};

struct Extent2D
{
    Extent2D()
        : sizeX(0)
        , sizeY(0)
    {
    }

    Extent2D(uint32_t sizeX, uint32_t sizeY)
        : sizeX(sizeX)
        , sizeY(sizeY)
    {
    }

    bool operator==(const Extent2D& _b) const
    {
        return sizeX == _b.sizeX && sizeY == _b.sizeY;
    }
    bool operator!=(const Extent2D& _b) const
    {
        return !(*this == _b);
    }

    uint32_t sizeX;
    uint32_t sizeY;
};

struct Extent3D
{
    Extent3D()
        : sizeX(0)
        , sizeY(0)
        , sizeZ(0)
    {
    }

    Extent3D(uint32_t sizeX, uint32_t sizeY = 1, uint32_t sizeZ = 1)
        : sizeX(sizeX)
        , sizeY(sizeY)
        , sizeZ(sizeZ)
    {
    }

    bool operator==(const Extent3D& _b) const
    {
        return sizeX == _b.sizeX && sizeY == _b.sizeY && sizeZ == _b.sizeZ;
    }
    bool operator!=(const Extent3D& _b) const
    {
        return !(*this == _b);
    }

    uint32_t sizeX;
    uint32_t sizeY;
    uint32_t sizeZ;
};

struct Viewport
{
    Viewport()
        : minX(0.f)
        , maxX(0.f)
        , minY(0.f)
        , maxY(0.f)
        , minZ(0.f)
        , maxZ(0.f)
    {
    }

    Viewport(float width, float height)
        : minX(0.f)
        , maxX(width)
        , minY(0.f)
        , maxY(height)
        , minZ(0.f)
        , maxZ(1.f)
    {
    }

    Viewport(float _minX, float _maxX, float _minY, float _maxY, float _minZ, float _maxZ)
        : minX(_minX)
        , maxX(_maxX)
        , minY(_minY)
        , maxY(_maxY)
        , minZ(_minZ)
        , maxZ(_maxZ)
    {
    }

    bool operator==(const Viewport& b) const
    {
        return minX == b.minX && minY == b.minY && minZ == b.minZ && maxX == b.maxX && maxY == b.maxY && maxZ == b.maxZ;
    }
    bool operator!=(const Viewport& b) const
    {
        return !(*this == b);
    }

    RHI_NODISCARD float GetWidth() const
    {
        return maxX - minX;
    }
    RHI_NODISCARD float GetHeight() const
    {
        return maxY - minY;
    }

    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;
};

struct MapableResource
{
    enum class Type
    {
        Buffer,
        Image,
    };

    Type type;

    union
    {
        class IBuffer* pBuffer;
        class IImage*  pImage;
    };

    MapableResource(IBuffer& pBuffer)
        : type(Type::Buffer)
        , pBuffer(&pBuffer)
    {
    }

    MapableResource(IImage& pImage)
        : type(Type::Image)
        , pImage(&pImage)
    {
    }
};

struct Id
{
    Id()
        : id(0)
    {
    }

    Id(uint32_t _id)
        : id(_id)
    {
    }

    bool operator==(const Id& _b) const
    {
        return id == _b.id;
    }
    bool operator!=(const Id& _b) const
    {
        return !(*this == _b);
    }

    uint32_t operator()() const
    {
        return id;
    }
    
    uint32_t id;
};

} // namespace RHI
