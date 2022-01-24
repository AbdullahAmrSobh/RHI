#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

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
    
    inline ITexture&               GetTexture() const { return *m_pTexture; }
    inline EPixelFormat            GetFormat() const { return m_format; }
    inline TextureViewAspectFlags  GetAspectFlags() const { return m_aspectFlags; }
    inline TextureSubresourceRange GetSubresourceRange() const { return m_viewRange; }

private:
    ITexture*               m_pTexture;
    EPixelFormat            m_format;
    TextureViewAspectFlags  m_aspectFlags;
    TextureSubresourceRange m_viewRange;
};
using TextureViewPtr = Unique<ITextureView>;

} // namespace RHI
