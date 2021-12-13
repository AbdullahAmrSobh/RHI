#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

// Structs
struct Color
{
    float r, g, b, a;

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
    Color(float _r, float _g, float _b, float _a)
        : r(_r)
        , g(_g)
        , b(_b)
        , a(_a)
    {
    }

    bool operator==(const Color& _b) const { return r == _b.r && g == _b.g && b == _b.b && a == _b.a; }
    bool operator!=(const Color& _b) const { return !(*this == _b); }
};

struct Rect
{
    uint32_t x;
    uint32_t y;
    uint32_t sizeX;
    uint32_t sizeY;
};

struct Extent2D
{
    uint32_t sizeX;
    uint32_t sizeY;
};

struct Extent3D
{
    uint32_t sizeX;
    uint32_t sizeY;
    uint32_t sizeZ;
};

struct Viewport
{
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;

    Viewport()
        : minX(0.f)
        , maxX(0.f)
        , minY(0.f)
        , maxY(0.f)
        , minZ(0.f)
        , maxZ(1.f)
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
    bool operator!=(const Viewport& b) const { return !(*this == b); }

    [[nodiscard]] float Width() const { return maxX - minX; }
    [[nodiscard]] float Height() const { return maxY - minY; }
};

using DeviceAddress = void*;

enum class EPixelFormat
{
    None,
    RGB,
    RGBA,
    BGRA,
    Depth,
    DepthStencil,
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

enum class ETextureViewDimensions
{
    View3D      = 0x00,
    View2D      = 0x01,
    View1D      = 0x02,
    ViewCubeMap = 0x03,
};

using NativeWindowHandle = void*;

// ------------------------------------------------------------------------------------------------
// Flag enums.                                                                                    -
// ------------------------------------------------------------------------------------------------
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
	Present				   = 0x00000100,
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

enum class EDeviceMemoryAllocationUsage
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

// ------------------------------------------------------------------------------------------------
// Descriptions structs.                                                                          -
// ------------------------------------------------------------------------------------------------

struct TextureSubresourceRange
{
    uint32_t mipLevel, mipLevelsCount;
    uint32_t arrayIndex, arrayCount;
};

// ------------------------------------------------------------------------------------------------
// RHI Objects.                                                                          -
// ------------------------------------------------------------------------------------------------

class IFence
{
public:
    virtual ~IFence() = default;

    virtual EResultCode Wait() const      = 0;
    virtual EResultCode Reset() const     = 0;
    virtual EResultCode GetStatus() const = 0;
};
using FencePtr = Unique<IFence>;

struct DeviceMemoryAllocationDesc
{
    explicit DeviceMemoryAllocationDesc(EDeviceMemoryAllocationUsage usage)
        : usage(usage)
    {
    }

    EDeviceMemoryAllocationUsage usage;
};

class IDeviceMemoryAllocation
{
public:
    virtual ~IDeviceMemoryAllocation() = default;

    virtual size_t                  GetSize() const                        = 0;
    virtual Expected<DeviceAddress> Map(size_t offset, size_t range) const = 0;
    virtual EResultCode             Unmap() const                          = 0;
};
using DeviceMemoryAllocationPtr = Unique<IDeviceMemoryAllocation>;

struct TextureDesc
{
    explicit TextureDesc(EPixelFormat format, TextureUsageFlags usage, Extent3D extent, uint32_t mipLevels = 1, uint32_t arrayLayers = 1)
        : extent(extent)
        , mipLevels(mipLevels)
        , arrayLayers(arrayLayers)
        , format(format)
        , usage(usage)
    {
    }

    Extent3D          extent;
    uint32_t          mipLevels;
    uint32_t          arrayLayers;
    EPixelFormat      format;
    TextureUsageFlags usage;
    ESampleCount      sampleCount;
};

class ITexture
{
public:
    virtual ~ITexture() = default;
    
    virtual Extent3D                 GetExtent() const           = 0;
    virtual uint32_t                 GetMipLevelsCount() const   = 0;
    virtual uint32_t                 GetArrayLayersCount() const = 0;
    virtual EPixelFormat             GetPixelFormat() const      = 0;
	virtual TextureUsageFlags		 GetUsage() const			 = 0;
	virtual ESampleCount             GetSampleCount() const      = 0;
    virtual IDeviceMemoryAllocation& GetAllocation()             = 0;
};
using TexturePtr = Unique<ITexture>;

struct TextureViewDesc
{
    ITexture*               pTexture;
    ETextureViewDimensions  dimensions;
    EPixelFormat            format;
    TextureViewAspectFlags  aspectFlags;
    TextureSubresourceRange subresourceRange;
};

class ITextureView
{
public:
    virtual ~ITextureView() = default;

    virtual ITexture&               GetUnderlyingTexture() const = 0;
    virtual TextureSubresourceRange GetSubresourceRange() const  = 0;
};
using TextureViewPtr = Unique<ITextureView>;

struct BufferDesc
{
    explicit BufferDesc(size_t size, BufferUsageFlags usage)
        : size(size)
        , usage(usage)
    {
    }

    size_t           size;
    BufferUsageFlags usage;
};

class IBuffer
{
public:
    virtual ~IBuffer() = default;

    virtual size_t                   GetSize() const = 0;
    virtual IDeviceMemoryAllocation& GetAllocation() = 0;
};
using BufferPtr = Unique<IBuffer>;

struct BufferViewDesc
{
    explicit BufferViewDesc(IBuffer& buffer, size_t offset, size_t size)
        : pBuffer(&buffer)
        , offset(offset)
        , size(size)
    {
    }

    IBuffer* pBuffer;
    size_t   offset;
    size_t   size;
};

class IBufferView
{
public:
    virtual ~IBufferView() = default;

    virtual IBuffer& GetUnderlyingBuffer() = 0;
    virtual size_t   GetOffset() const     = 0;
    virtual size_t   GetSize() const       = 0;
};
using BufferViewPtr = Unique<IBufferView>;

struct SwapChainDesc
{
    explicit SwapChainDesc(NativeWindowHandle window, Extent2D extent)
        : extent(extent)
        , backBuffersCount(2)
        , windowHandle(window)
        , usage(ETextureUsageFlagBits::ColorAttachment)
    {
    }

    explicit SwapChainDesc(NativeWindowHandle window, Extent2D extent, TextureUsageFlags usage, uint32_t backBuffersCount)
        : extent(extent)
        , backBuffersCount(backBuffersCount)
        , windowHandle(window)
        , usage(usage)
    {
    }

    Extent2D           extent;
    uint32_t           backBuffersCount;
    NativeWindowHandle windowHandle;
    TextureUsageFlags  usage;
};

class ISwapChain
{
public:
    virtual ~ISwapChain() = default;

    virtual EResultCode SwapBackBuffers()                 = 0;
    virtual ITexture*   GetBackBuffers()                  = 0;
    virtual uint32_t    GetBackBufferCount() const        = 0;
    virtual uint32_t    GetCurrentBackBufferIndex() const = 0;
};
using SwapChainPtr = Unique<ISwapChain>;

struct RenderTargetDesc
{
    RenderTargetDesc(Extent2D extent, std::vector<ITextureView*> colorAttachments, ITextureView* pDepthStencilAttachment);
    std::vector<ITextureView*> colorAttachments;
    ITextureView*              pDepthStencilAttachment;
    Extent2D                   extent;
};

class IRenderTarget
{
public:
    virtual ~IRenderTarget() = default;

    virtual std::vector<EPixelFormat> GetColorAttachmentFormats() const       = 0;
    virtual EPixelFormat              GetDepthStencilAttachmentFormat() const = 0;
    virtual Extent2D                  GetExtent() const                       = 0;
    virtual uint32_t                  GetCount() const                        = 0;
    virtual ITextureView**            GetAttachments()                        = 0;
};
using RenderTargetPtr = Unique<IRenderTarget>;

// ------------------------------------------------------------------------------------------------
// WIP.                                                                                           -
// ------------------------------------------------------------------------------------------------

struct SamplerDesc
{
};
class ISampler
{
public:
    virtual ~ISampler() = default;
};
using SamplerPtr = Unique<ISampler>;

} // namespace RHI
