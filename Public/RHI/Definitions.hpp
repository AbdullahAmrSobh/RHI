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
    Texture       = 2,
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
    None,
    RGB,
    RGBA,
    BGRA,
    Depth,
    DepthStencil,
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

enum class ETextureViewDimensions
{
    View3D      = 0x00,
    View2D      = 0x01,
    View1D      = 0x02,
    ViewCubeMap = 0x03,
};

enum class ETextureUsageFlagBits
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
using TextureUsageFlags = Flags<ETextureUsageFlagBits>;

enum class ETextureViewAspectFlagBits
{
    Invalid = 0x00000000,
    Color   = 0x00000001,
    Depth   = 0x00000002,
    Stencil = 0x00000004,
    MaxEnum = 0x7FFFFFFF,
};
using TextureViewAspectFlags = Flags<ETextureViewAspectFlagBits>;

enum class ETextureLayout
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
    explicit Color(float c)
        : r(c)
        , g(c)
        , b(c)
        , a(c)
    {
    }
    explicit Color(float r, float g, float b, float a)
        : r(r)
        , g(g)
        , b(b)
        , a(a)
    {
    }

    bool operator==(const Color& _b) const { return r == _b.r && g == _b.g && b == _b.b && a == _b.a; }
    bool operator!=(const Color& _b) const { return !(*this == _b); }

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

    explicit Rect(uint32_t w, uint32_t h)
        : x(0)
        , y(0)
        , sizeX(w)
        , sizeY(h)
    {
    }

    explicit Rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
        : x(x)
        , y(y)
        , sizeX(w)
        , sizeY(h)
    {
    }

    bool operator==(const Rect& _b) const { return x == _b.x && y == _b.y && sizeX == _b.sizeX && sizeY == _b.sizeY; }
    bool operator!=(const Rect& _b) const { return !(*this == _b); }

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

    explicit Volume(uint32_t sizeX, uint32_t sizeY, uint32_t sizeZ)
        : x(0)
        , y(0)
        , z(0)
        , sizeX(sizeX)
        , sizeY(sizeY)
        , sizeZ(sizeZ)
    {
    }

    explicit Volume(uint32_t x, uint32_t y, uint32_t z, uint32_t sizeX, uint32_t sizeY, uint32_t sizeZ)
        : x(x)
        , y(y)
        , z(z)
        , sizeX(sizeX)
        , sizeY(sizeY)
        , sizeZ(sizeZ)
    {
    }

    bool operator==(const Volume& _b) const { return x == _b.x && y == _b.y && z == _b.z && sizeX == _b.sizeX && sizeY == _b.sizeY && sizeZ == _b.sizeZ; }
    bool operator!=(const Volume& _b) const { return !(*this == _b); }

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

    explicit Extent2D(uint32_t sizeX, uint32_t sizeY)
        : sizeX(sizeX)
        , sizeY(sizeY)
    {
    }

    bool operator==(const Extent2D& _b) const { return sizeX == _b.sizeX && sizeY == _b.sizeY; }
    bool operator!=(const Extent2D& _b) const { return !(*this == _b); }

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

    explicit Extent3D(uint32_t sizeX, uint32_t sizeY = 1, uint32_t sizeZ = 1)
        : sizeX(sizeX)
        , sizeY(sizeY)
        , sizeZ(sizeZ)
    {
    }

    bool operator==(const Extent3D& _b) const { return sizeX == _b.sizeX && sizeY == _b.sizeY && sizeZ == _b.sizeZ; }
    bool operator!=(const Extent3D& _b) const { return !(*this == _b); }

    uint32_t sizeX;
    uint32_t sizeY;
    uint32_t sizeZ;
};

struct Viewport
{
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;

    explicit Viewport()
        : minX(0.f)
        , maxX(0.f)
        , minY(0.f)
        , maxY(0.f)
        , minZ(0.f)
        , maxZ(1.f)
    {
    }

    explicit Viewport(float width, float height)
        : minX(0.f)
        , maxX(width)
        , minY(0.f)
        , maxY(height)
        , minZ(0.f)
        , maxZ(1.f)
    {
    }

    explicit Viewport(float _minX, float _maxX, float _minY, float _maxY, float _minZ, float _maxZ)
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
    bool operator!=(const Viewport& b) const { return !(*this == b); }

    RHI_NODISCARD float GetWidth() const { return maxX - minX; }
    RHI_NODISCARD float GetHeight() const { return maxY - minY; }
};

struct MapableResource
{
    enum class Type
    {
        Buffer,
        Texture,
    };

    Type type;

    union
    {
        class IBuffer*  pBuffer;
        class ITexture* pTexture;
    };

    MapableResource(IBuffer& pBuffer)
        : type(Type::Buffer)
        , pBuffer(&pBuffer)
    {
    }

    MapableResource(ITexture& pTexture)
        : type(Type::Texture)
        , pTexture(&pTexture)
    {
    }
};

} // namespace RHI
