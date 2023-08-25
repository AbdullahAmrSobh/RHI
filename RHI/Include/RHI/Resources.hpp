#pragma once

#include <cstdint>
#include <memory>
#include <variant>

#include "RHI/Export.hpp"
#include "RHI/Flags.hpp"
#include "RHI/Format.hpp"
#include "RHI/Object.hpp"
#include "RHI/Result.hpp"

namespace RHI
{

/// @brief  Type of the resource.
enum class ResourceType
{
    Buffer,
    Image,
};

/// @brief Enumeration representing how the buffer resource is intented to used.
enum class BufferUsage
{
    None = 0 << 0,

    /// @brief The buffer will be used as a storage buffer object.
    Storage = 1 << 1,

    /// @brief The buffer will be used as an uniform buffer object.
    Uniform = 1 << 2,

    /// @brief The buffer will be used as a vertex buffer object.
    Vertex = 1 << 3,

    /// @brief The buffer will be used as a index buffer object.
    Index = 1 << 4,

    /// @brief This buffer content will be copied from.
    CopySrc = 1 << 5,

    /// @brief This buffer content will be overwritten by a copy command.
    CopyDst = 1 << 6,
};

/// @brief Enumeration representing how the image resource is intented to used.
enum class ImageUsage
{
    None = 0 << 0,

    /// @brief The image will be used in an shader as bind resource.
    ShaderResource = 1 << 1,

    /// @brief The image will be the render target color attachment.
    Color = 1 << 3,

    /// @brief The image will be the render target depth attachment.
    Depth = 1 << 4,

    /// @brief The image will be the render target stencil attachment.
    Stencil = 1 << 5,

    /// @brief The image content will be copied.
    CopySrc = 1 << 6,

    /// @brief The image content will be overwritten by a copy command.
    CopyDst = 1 << 7,
};

/// @brief Enumeration how many samples a multisampled image contain.
enum class ImageSampleCount
{
    None      = 0 << 0,
    Samples1  = 1 << 1,
    Samples2  = 1 << 2,
    Samples4  = 1 << 3,
    Samples8  = 1 << 4,
    Samples16 = 1 << 5,
    Samples32 = 1 << 6,
};

/// @brief Enumeration representing the dimensions of an image resource.
enum class ImageType
{
    None,

    /// @brief Image is 1 dimentional.
    Image1D,

    /// @brief Image is 2 dimentional.
    Image2D,

    /// @brief Image is 3 dimentional.
    Image3D,

    /// @brief Image is a cube map.
    ImageCubeMap,
};

/// @brief Enumeration representing the aspects of an image resource.
enum class ImageAspect
{
    None         = 0,
    Color        = 1 << 1,
    Depth        = 1 << 2,
    Stencil      = 1 << 3,
    DepthStencil = Depth | Stencil,
    All          = Color | DepthStencil,
};

/// @brief Enumeration representing the component mapping.
enum class ComponentSwizzle
{
    Identity = 0,
    Zero,
    One,
    R,
    G,
    B,
    A,
};

enum class SamplerFilter
{
    Point,
    Linear,
};

enum class SamplerAddressMode
{
    Repeat,
    Clamp,
};

enum class SamplerCompareOp
{
    Never,
    Equal,
    NotEqual,
    Always,
    Less,
    LessEq,
    Greater,
    GreaterEq,
};

/// @brief Represent the offset into an image resource.
struct ImageOffset
{
    /// @brief Offset in the X direction.
    int32_t x;

    /// @brief Offset in the Y direction.
    int32_t y;

    /// @brief Offset in the Z direction.
    int32_t z;
};

/// @brief Represent the size of an image resource or subregion.
struct ImageSize
{
    /// @brief The width of the image.
    uint32_t width;

    /// @brief The height of the image.
    uint32_t height;

    /// @brief The depth of the image.
    uint32_t depth;
};

/// @brief Represent the creation parameters of an image resource.
struct ImageCreateInfo
{
    /// @brief Usage flags.
    Flags<ImageUsage> usageFlags;

    /// @brief The type of the image.
    ImageType type;

    /// @brief The size of the image.
    ImageSize size;

    /// @brief The format of the image.
    Format format;

    /// @brief The number of mip levels in the image.
    uint32_t mipLevels;

    /// @brief The number of images in the images array.
    uint32_t arrayCount;
};

/// @brief Represent the creation parameters of an buffer resource.
struct BufferCreateInfo
{
    /// @brief Usage flags.
    Flags<BufferUsage> usageFlags;

    /// @brief The size of the buffer.
    size_t byteSize;
};

/// @brief Represent a subview into a an image resource.
struct ImageSubresource
{
    uint32_t           arrayBase    = 0;
    uint32_t           arrayCount   = UINT32_MAX;
    uint32_t           mipBase      = 0;
    uint32_t           mipCount     = UINT32_MAX;
    Flags<ImageAspect> imageAspects = ImageAspect::All;

    inline bool operator==(const ImageSubresource& other) const;

    inline bool operator!=(const ImageSubresource& other) const;
};

/// @brief ...
struct ComponentMapping
{
    ComponentSwizzle r;
    ComponentSwizzle g;
    ComponentSwizzle b;
    ComponentSwizzle a;

    inline bool operator==(const ComponentMapping& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    inline bool operator!=(const ComponentMapping& other) const
    {
        return !(r == other.r && g == other.g && b == other.b && a == other.a);
    }
};

/// @brief Structure describing the creation parameters of a sampler state.
struct SamplerStateCreateInfo
{
    SamplerFilter      filterMin  = SamplerFilter::Point;
    SamplerFilter      filterMag  = SamplerFilter::Point;
    SamplerFilter      filterMip  = SamplerFilter::Point;
    SamplerCompareOp   compare    = SamplerCompareOp::Always;
    float              mipLodBias = 0.0f;
    SamplerAddressMode addressU   = SamplerAddressMode::Clamp;
    SamplerAddressMode addressV   = SamplerAddressMode::Clamp;
    SamplerAddressMode addressW   = SamplerAddressMode::Clamp;
    float              minLod     = 0.0f;
    float              maxLod     = 1.0f;
};

}  // namespace RHI