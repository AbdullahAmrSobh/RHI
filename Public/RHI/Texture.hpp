#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

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

struct TextureDesc
{
    Extent3D          extent;
    uint32_t          mipLevels;
    uint32_t          arraySize;
    EPixelFormat      format;
    ESampleCount      sampleCount;
    TextureUsageFlags usage;
};

class ITexture : public IResource
{
public:
    virtual ~ITexture() = default;
    
    inline const TextureDesc& GetDesc() const { return m_desc; };
	
protected:
    TextureDesc m_desc;
};
using TexturePtr = Unique<ITexture>;

struct TextureSubresourceRange
{
    uint32_t mipLevelsCount;
    uint32_t mipLevel;
    uint32_t arrayCount;
    uint32_t arrayIndex;
};

struct TextureViewDesc
{
    ITexture*               pTexture;
    EPixelFormat            format;
    ETextureViewDimensions  viewDimension;
    TextureViewAspectFlags  aspectFlags;
    TextureSubresourceRange viewRange;
};

class ITextureView
{
public:
    virtual ~ITextureView() = default;
    
    inline const TextureViewDesc& GetDesc() const { return m_desc; };

protected:
    TextureViewDesc m_desc;
};
using TextureViewPtr = Unique<ITextureView>;

} // namespace RHI
