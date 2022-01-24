#pragma once
#include "RHI/Common.hpp"
#include "RHI/Core/Expected.hpp"
#include "RHI/Core/Span.hpp"
#include <functional>

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

namespace RHI
{

using DeviceAddress = void*;

enum class EBackendType
{
    None,
    Vulkan,
    D3D12,
    Metal,
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

enum class EBufferFormat {};

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

// Structs
struct Color
{
    explicit Color()
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

    uint32_t x;
    uint32_t y;
    uint32_t sizeX;
    uint32_t sizeY;
};

struct Volume
{
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

    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t sizeX;
    uint32_t sizeY;
    uint32_t sizeZ;
};

struct Extent2D
{
	Extent2D() = default;
    explicit Extent2D(uint32_t sizeX, uint32_t sizeY)
        : sizeX(sizeX)
        , sizeY(sizeY)
    {
    }

    uint32_t sizeX;
    uint32_t sizeY;
};

struct Extent3D
{
	Extent3D() = default;
    explicit Extent3D(uint32_t sizeX, uint32_t sizeY = 1, uint32_t sizeZ = 1)
        : sizeX(sizeX)
        , sizeY(sizeY)
        , sizeZ(sizeZ)
    {
    }

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

} // namespace RHI
