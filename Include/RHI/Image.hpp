#pragma once
#include "RHI/Resource.hpp"

namespace RHI
{
enum class SampleCount
{
    Undefined = 0x0000000,
    Count1    = 0x0000001,
    Count2    = 0x0000002,
    Count4    = 0x0000004,
    Count8    = 0x0000008,
    Count16   = 0x000000F,
    Count32   = 0x0000020,
    Count64   = 0x0000040
};

enum class ImageUsageFlagBits
{
    Undefined    = 0x000000,
    Color        = 0x000001,
    DepthStencil = 0x000002,
    Transfer     = 0x000004,
    ShaderInput  = 0x000008,
};
using ImageUsageFlags = Flags<ImageUsageFlagBits>;

enum class ImageType
{
    Image1D,
    Image2D,
    Image3D,
};

enum class ImageViewType
{
    View1D           = 0,
    View1DArray      = 1,
    View2D           = 2,
    View2DArray      = 3,
    View3D           = 4,
    ViewCubeMap      = 5,
    ViewCubeMapArray = 6
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

struct ImageDesc
{
    ImageDesc() = default;

    static ImageDesc Create(ImageUsageFlags flags,
                            size_t          sizeX,
                            Format          format,
                            SampleCount     sampleCount = SampleCount::Count1,
                            uint32_t        mipLevels   = 1,
                            uint32_t        arraySize   = 0);

    static ImageDesc Create(ImageUsageFlags flags,
                            Extent2D        extent,
                            Format          format,
                            SampleCount     sampleCount = SampleCount::Count1,
                            uint32_t        mipLevels   = 1,
                            uint32_t        arraySize   = 0);

    static ImageDesc Create(ImageUsageFlags flags,
                            Extent3D        extent,
                            Format          format,
                            SampleCount     sampleCount = SampleCount::Count1,
                            uint32_t        mipLevels   = 1,
                            uint32_t        arraySize   = 0);

    size_t GetHash() const
    {
        return 0;
    }

    ImageUsageFlags usage;
    Extent3D        extent;
    ImageType       imageType;
    Format          format;
    SampleCount     sampleCount;
    uint32_t        mipLevelsCount;
    uint32_t        arraySize;
};

class IImage : public IResource
{
public:
    IImage()
        : m_desc(CreateUnique<ImageDesc>())
    {
    }

    virtual ~IImage() = default;

    const ImageDesc& GetDescription() const
    {
        return *m_desc;
    }

protected:
    Unique<ImageDesc> m_desc;
};

struct ImageViewRange
{
    ImageViewRange() = default;

    uint32_t baseArrayElement = 0;
    uint32_t arraySize        = 1;
    uint32_t baseMipLevel     = 0;
    uint32_t mipLevelsCount   = 1;
};

enum class ImageViewAspectFlagBits
{
    Undefined    = 0x00000000,
    Color        = 0x00000001,
    Depth        = 0x00000002,
    Stencil      = 0x00000004,
    DepthStencil = Depth | Stencil
};
using ImageViewAspectFlags = Flags<ImageViewAspectFlagBits>;

ImageViewAspectFlags GetImageAspectFlags(Format format);

struct ImageViewDesc
{
    ImageViewDesc() = default;

    static ImageViewDesc Create(Format format, ImageViewType viewType, ImageViewRange range);
    static ImageViewDesc Create(Format format, ImageViewType viewType, ImageViewAspectFlags flags, ImageViewRange range);

    size_t GetHash() const
    {
        return 0;
    }

    Format               format;
    ImageViewType        type;
    ImageViewAspectFlags viewAspect;
    ImageViewRange       range;
};

class IImageView
{
public:
    IImageView()
        : m_desc(CreateUnique<ImageViewDesc>())
    {
    }

    virtual ~IImageView() = default;

    const ImageViewDesc& GetDescription() const
    {
        return *m_desc;
    }

protected:
    Unique<ImageViewDesc> m_desc;
};
}  // namespace RHI