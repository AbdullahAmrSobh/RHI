#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

struct ImageDesc
{
    Extent3D        extent;
    uint32_t        mipLevels;
    uint32_t        arraySize;
    EPixelFormat    format;
    ESampleCount    sampleCount;
    ImageUsageFlags usage;
};

class IImage
{
public:
    virtual ~IImage() = default;

	virtual size_t GetMemorySize() const { return 0; };
};
using ImagePtr = Unique<IImage>;

struct ImageSubresourceRange
{
	ImageSubresourceRange() = default;
	ImageSubresourceRange(uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount)
	: mipLevelsCount(baseMipLevel)
	, mipLevel(levelCount)
	, arrayCount(layerCount)
	, arrayIndex(baseArrayLayer)
	{}
    
    uint32_t mipLevelsCount;
    uint32_t mipLevel;
    uint32_t arrayCount;
    uint32_t arrayIndex;
};

struct ImageViewDesc
{
	ImageViewDesc() = default;

    IImage*               pImage;
    EPixelFormat          format;
    EImageViewDimensions  viewDimension;
    ImageViewAspectFlags  aspectFlags;
    ImageSubresourceRange viewRange;
};

class IImageView
{
public:
    virtual ~IImageView() = default;

    inline IImage&               GetImage() const { return *m_pImage; }
    inline EPixelFormat          GetFormat() const { return m_format; }
    inline ImageViewAspectFlags  GetAspectFlags() const { return m_aspectFlags; }
    inline ImageSubresourceRange GetSubresourceRange() const { return m_viewRange; }

private:
    IImage*               m_pImage;
    EPixelFormat          m_format;
    ImageViewAspectFlags  m_aspectFlags;
    ImageSubresourceRange m_viewRange;
};
using ImageViewPtr = Unique<IImageView>;

} // namespace RHI
