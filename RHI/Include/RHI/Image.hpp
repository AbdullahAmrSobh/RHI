#pragma once

#include "RHI/Format.hpp"
#include "RHI/Handle.hpp"
#include "RHI/SampleCount.hpp"

#include <TL/Flags.hpp>
#include <TL/Span.hpp>
#include <TL/Utils.hpp>

namespace RHI
{
    RHI_DECLARE_OPAQUE_RESOURCE(Image);
    RHI_DECLARE_OPAQUE_RESOURCE(ImageView);

    /// @brief Flags representing image usage.
    enum class ImageUsage
    {
        None            = 0 << 0,          ///< No usage.
        ShaderResource  = 1 << 1,          ///< Image is used as a shader resource.
        StorageResource = 1 << 2,          ///< Image is used as a storage resource.
        Color           = 1 << 3,          ///< Image is used for color attachments.
        Depth           = 1 << 4,          ///< Image is used for depth attachments.
        Stencil         = 1 << 5,          ///< Image is used for stencil attachments.
        DepthStencil    = Depth | Stencil, ///< Image is used for depth-stencil attachments.
        CopySrc         = 1 << 6,          ///< Image is used as a source in copy operations.
        CopyDst         = 1 << 7,          ///< Image is used as a destination in copy operations.
        Resolve         = CopyDst          ///< Image is used for resolve operations.
    };

    TL_DEFINE_FLAG_OPERATORS(ImageUsage);

    /// @brief Types of images.
    enum class ImageType
    {
        None,    ///< No image type.
        Image1D, ///< 1D image.
        Image2D, ///< 2D image.
        Image3D, ///< 3D image.
    };

    /// @brief Types of image views.
    enum class ImageViewType
    {
        None,        ///< No image view type.
        View1D,      ///< 1D image view.
        View1DArray, ///< 1D array image view.
        View2D,      ///< 2D image view.
        View2DArray, ///< 2D array image view.
        View3D,      ///< 3D image view.
        CubeMap,     ///< Cube map image view.
    };

    /// @brief Aspects of an image.
    enum class ImageAspect
    {
        None         = 0,                    ///< No aspect.
        Color        = 1 << 1,               ///< Color aspect.
        Depth        = 1 << 2,               ///< Depth aspect.
        Stencil      = 1 << 3,               ///< Stencil aspect.
        DepthStencil = Depth | Stencil,      ///< Depth-stencil aspect.
        All          = Color | DepthStencil, ///< All aspects.
    };

    TL_DEFINE_FLAG_OPERATORS(ImageAspect);

    /// @brief Component swizzle options for image formats.
    enum class ComponentSwizzle
    {
        Identity = 0, ///< No swizzling, retains original component.
        Zero,         ///< Component is set to zero.
        One,          ///< Component is set to one.
        R,            ///< Red component.
        G,            ///< Green component.
        B,            ///< Blue component.
        A,            ///< Alpha component.
    };

    /// @brief 2D offset for images.
    struct ImageOffset2D
    {
        int32_t     x; ///< X coordinate offset.
        int32_t     y; ///< Y coordinate offset.

        inline bool operator==(const ImageOffset2D& other) const { return x == other.x && y == other.y; }
    };

    /// @brief 3D offset for images.
    struct ImageOffset3D
    {
        int32_t     x; ///< X coordinate offset.
        int32_t     y; ///< Y coordinate offset.
        int32_t     z; ///< Z coordinate offset.

        inline bool operator==(const ImageOffset3D& other) const { return x == other.x && y == other.y && z == other.z; }
    };

    /// @brief 2D size for images.
    struct ImageSize2D
    {
        uint32_t    width;  ///< Width of the image.
        uint32_t    height; ///< Height of the image.

        inline bool operator==(const ImageSize2D& other) const { return width == other.width && height == other.height; }
    };

    /// @brief 3D size for images.
    struct ImageSize3D
    {
        uint32_t    width;  ///< Width of the image.
        uint32_t    height; ///< Height of the image.
        uint32_t    depth;  ///< Depth of the image.

        inline bool operator==(const ImageSize3D& other) const { return width == other.width && height == other.height && depth == other.depth; }
    };

    /// @brief Mapping of color components.
    struct ComponentMapping
    {
        ComponentSwizzle r; ///< Red component swizzle.
        ComponentSwizzle g; ///< Green component swizzle.
        ComponentSwizzle b; ///< Blue component swizzle.
        ComponentSwizzle a; ///< Alpha component swizzle.

        inline bool      operator==(const ComponentMapping& other) const { return r == other.r && g == other.g && b == other.b && a == other.a; }
    };

    /// @brief Describes subresources of an image.
    struct ImageSubresource
    {
        TL::Flags<ImageAspect> imageAspects; ///< Image aspects to access.
        uint32_t               mipLevel;     ///< Mipmap level.
        uint32_t               arrayLayer;   ///< Array layer.

        inline bool            operator==(const ImageSubresource& other) const { return imageAspects == other.imageAspects && mipLevel == other.mipLevel && arrayLayer == other.arrayLayer; }
    };

    /// @brief Describes a range of subresources in an image.
    struct ImageSubresourceRange
    {
        TL::Flags<ImageAspect> imageAspects;  ///< Image aspects to access.
        uint32_t               mipBase;       ///< Base mip level.
        uint32_t               mipLevelCount; ///< Number of mip levels.
        uint32_t               arrayBase;     ///< Base array layer.
        uint32_t               arrayCount;    ///< Number of array layers.

        inline bool            operator==(const ImageSubresourceRange& other) const { return imageAspects == other.imageAspects && mipBase == other.mipBase && mipLevelCount == other.mipLevelCount && arrayBase == other.arrayBase && arrayCount == other.arrayCount; }
    };

    /// @brief Information needed to create an image.
    struct ImageCreateInfo
    {
        const char*           name;        ///< Name of the image.
        TL::Flags<ImageUsage> usageFlags;  ///< Usage flags for the image.
        ImageType             type;        ///< Type of the image.
        ImageSize3D           size;        ///< Size of the image.
        Format                format;      ///< Format of the image.
        SampleCount           sampleCount; ///< Number of samples per pixel.
        uint32_t              mipLevels;   ///< Number of mipmap levels.
        uint32_t              arrayCount;  ///< Number of array layers.
    };

    /// @brief Information needed to create an image view.
    struct ImageViewCreateInfo
    {
        const char*           name;        ///< Name of the image view.
        Handle<Image>         image;       ///< Handle to the image.
        ImageViewType         viewType;    ///< Type of the image view.
        ComponentMapping      components;  ///< Component mapping.
        ImageSubresourceRange subresource; ///< Subresource range.

        inline bool           operator==(const ImageViewCreateInfo& other) const { return components == other.components && viewType == other.viewType && subresource == other.subresource; }
    };

} // namespace RHI

namespace std
{
    TL_DEFINE_POD_HASH(RHI::ImageCreateInfo);
    TL_DEFINE_POD_HASH(RHI::ImageViewCreateInfo);
} // namespace std
