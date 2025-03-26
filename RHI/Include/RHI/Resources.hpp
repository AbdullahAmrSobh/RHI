#pragma once

#include "RHI/Format.hpp"
#include "RHI/Handle.hpp"

#include <TL/Flags.hpp>
#include <TL/Span.hpp>

namespace RHI
{
    RHI_DECLARE_OPAQUE_RESOURCE(Buffer);
    RHI_DECLARE_OPAQUE_RESOURCE(Image);
    RHI_DECLARE_OPAQUE_RESOURCE(Sampler);

    /// @brief Specifies the usage flags for a buffer.
    enum class BufferUsage
    {
        None     = 0 << 0, ///< No usage flags set.
        Storage  = 1 << 1, ///< Buffer used for storage operations.
        Uniform  = 1 << 2, ///< Buffer used for uniform data.
        Vertex   = 1 << 3, ///< Buffer used for vertex data.
        Index    = 1 << 4, ///< Buffer used for index data.
        CopySrc  = 1 << 5, ///< Buffer used as a source in copy operations.
        CopyDst  = 1 << 6, ///< Buffer used as a destination in copy operations.
        Indirect = 1 << 7, ///< Buffer used for indirect draw calls.
    };

    TL_DEFINE_FLAG_OPERATORS(BufferUsage);

    enum class IndexType
    {
        uint16,
        uint32,
    };

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
        Resolve         = CopyDst,         ///< Image is used for resolve operations.
    };

    TL_DEFINE_FLAG_OPERATORS(ImageUsage);

    /// @brief Specifies the number of samples for multisampling in rendering.
    enum class SampleCount
    {
        None      = 0 << 0, ///< No multisampling.
        Samples1  = 1 << 0, ///< Single sample per pixel.
        Samples2  = 1 << 1, ///< Two samples per pixel.
        Samples4  = 1 << 2, ///< Four samples per pixel.
        Samples8  = 1 << 3, ///< Eight samples per pixel.
        Samples16 = 1 << 4, ///< Sixteen samples per pixel.
        Samples32 = 1 << 5, ///< Thirty-two samples per pixel.
        Samples64 = 1 << 6, ///< Sixty-four samples per pixel.
    };

    /// @brief Enables bitwise flag operations for the SampleCount enum.
    TL_DEFINE_FLAG_OPERATORS(SampleCount);

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
    enum class ImageAspect : uint8_t
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

    /// @brief Specifies filtering options for a Sampler.
    enum class SamplerFilter
    {
        Point,  ///< Uses nearest neighbor filtering.
        Linear, ///< Uses linear filtering.
    };

    /// @brief Specifies the addressing modes for a Sampler.
    enum class SamplerAddressMode
    {
        Repeat, ///< Repeats the texture when UV coordinates are outside the range [0,1].
        Clamp,  ///< Clamps UV coordinates to the edge of the texture.
    };

    /// @brief Specifies the compare operation used for texture sampling.
    enum class SamplerCompareOperation
    {
        Never,     ///< Comparison always fails.
        Equal,     ///< Comparison passes if the values are equal.
        NotEqual,  ///< Comparison passes if the values are not equal.
        Always,    ///< Comparison always passes.
        Less,      ///< Comparison passes if the sampled value is less than the reference value.
        LessEq,    ///< Comparison passes if the sampled value is less than or equal to the reference value.
        Greater,   ///< Comparison passes if the sampled value is greater than the reference value.
        GreaterEq, ///< Comparison passes if the sampled value is greater than or equal to the reference value.
    };

    /// @brief Describes a subregion of a buffer.
    struct BufferSubregion
    {
        size_t      offset = 0; ///< Offset into the buffer.
        size_t      size   = 0; ///< Size of the subregion.

        /// @brief Compares this subregion with another for equality.
        /// @param other The other subregion to compare with.
        /// @return true if both subregions have the same offset and size, false otherwise.
        inline bool operator==(const BufferSubregion& other) const { return size == other.size && offset == other.offset; }
    };

    /// @brief Describes the parameters required to create a buffer.
    struct BufferCreateInfo
    {
        const char*            name       = nullptr;           ///< Name of the buffer.
        bool                   hostMapped = true;              ///< Buffer will be mappable by host.
        TL::Flags<BufferUsage> usageFlags = BufferUsage::None; ///< Usage flags for the buffer.
        size_t                 byteSize   = 0;                 ///< Size of the buffer in bytes.
    };

    /// @brief 2D offset for images.
    struct ImageOffset2D
    {
        int32_t     x = 0; ///< X coordinate offset.
        int32_t     y = 0; ///< Y coordinate offset.

        inline bool operator==(const ImageOffset2D& other) const { return x == other.x && y == other.y; }
    };

    /// @brief 3D offset for images.
    struct ImageOffset3D
    {
        int32_t     x = 0; ///< X coordinate offset.
        int32_t     y = 0; ///< Y coordinate offset.
        int32_t     z = 0; ///< Z coordinate offset.

        inline bool operator==(const ImageOffset3D& other) const { return x == other.x && y == other.y && z == other.z; }
    };

    /// @brief 2D size for images.
    struct ImageSize2D
    {
        uint32_t    width  = 1; ///< Width of the image.
        uint32_t    height = 1; ///< Height of the image.

        inline bool operator==(const ImageSize2D& other) const { return width == other.width && height == other.height; }
    };

    /// @brief 3D size for images.
    struct ImageSize3D
    {
        uint32_t    width  = 1; ///< Width of the image.
        uint32_t    height = 1; ///< Height of the image.
        uint32_t    depth  = 1; ///< Depth of the image.

        inline bool operator==(const ImageSize3D& other) const
        {
            return width == other.width && height == other.height && depth == other.depth;
        }
    };

    /// @brief Mapping of color components.
    struct ComponentMapping
    {
        ComponentSwizzle r = ComponentSwizzle::Identity; ///< Red component swizzle.
        ComponentSwizzle g = ComponentSwizzle::Identity; ///< Green component swizzle.
        ComponentSwizzle b = ComponentSwizzle::Identity; ///< Blue component swizzle.
        ComponentSwizzle a = ComponentSwizzle::Identity; ///< Alpha component swizzle.

        inline bool operator==(const ComponentMapping& other) const { return r == other.r && g == other.g && b == other.b && a == other.a; }
    };

    /// @brief Describes subresources of an image.
    struct ImageSubresource
    {
        TL::Flags<ImageAspect> imageAspects = ImageAspect::Color; ///< Image aspects to access.
        uint32_t               mipLevel     = 0;                  ///< Mipmap level.
        uint32_t               arrayLayer   = 0;                  ///< Array layer.

        inline bool            operator==(const ImageSubresource& other) const
        {
            return imageAspects == other.imageAspects && mipLevel == other.mipLevel && arrayLayer == other.arrayLayer;
        }
    };

    /// @brief Describes a range of subresources in an image.
    struct ImageSubresourceRange
    {
        TL::Flags<ImageAspect> imageAspects  = ImageAspect::Color; ///< Image aspects to access.
        uint8_t                mipBase       = 0;                  ///< Base mip level.
        uint8_t                mipLevelCount = 1;                  ///< Number of mip levels.
        uint8_t                arrayBase     = 0;                  ///< Base array layer.
        uint8_t                arrayCount    = 1;                  ///< Number of array layers.

        inline bool            operator==(const ImageSubresourceRange& other) const
        {
            return imageAspects == other.imageAspects && mipBase == other.mipBase && mipLevelCount == other.mipLevelCount &&
                   arrayBase == other.arrayBase && arrayCount == other.arrayCount;
        }
    };

    /// @brief Information needed to create an image.
    struct ImageCreateInfo
    {
        const char*           name        = nullptr;               ///< Name of the image.
        TL::Flags<ImageUsage> usageFlags  = ImageUsage::None;      ///< Usage flags for the image.
        ImageType             type        = ImageType::None;       ///< Type of the image.
        ImageSize3D           size        = {};                    ///< Size of the image.
        Format                format      = Format::Unknown;       ///< Format of the image.
        SampleCount           sampleCount = SampleCount::Samples1; ///< Number of samples per pixel.
        uint32_t              mipLevels   = 1;                     ///< Number of mipmap levels.
        uint32_t              arrayCount  = 1;                     ///< Number of array layers.
    };

    /// @brief Describes the parameters required to create a Sampler.
    struct SamplerCreateInfo
    {
        const char*             name       = nullptr;                         ///< Name of the sampler.
        SamplerFilter           filterMin  = SamplerFilter::Linear;           ///< Filter for minification.
        SamplerFilter           filterMag  = SamplerFilter::Linear;           ///< Filter for magnification.
        SamplerFilter           filterMip  = SamplerFilter::Linear;           ///< Filter for mipmap selection.
        SamplerCompareOperation compare    = SamplerCompareOperation::Always; ///< Compare operation for texture comparison.
        float                   mipLodBias = 0.0f;                            ///< Bias applied to the mip level of detail.
        SamplerAddressMode      addressU   = SamplerAddressMode::Repeat;      ///< Addressing mode for the U (horizontal) coordinate.
        SamplerAddressMode      addressV   = SamplerAddressMode::Repeat;      ///< Addressing mode for the V (vertical) coordinate.
        SamplerAddressMode      addressW   = SamplerAddressMode::Repeat;      ///< Addressing mode for the W (depth) coordinate.
        float                   minLod     = 0.0f;                            ///< Minimum level of detail (LOD) that can be used.
        float                   maxLod     = 1000.0f;                         ///< Maximum level of detail (LOD) that can be used.
    };

    /// @brief Information needed to create an image view.
    struct ImageViewCreateInfo
    {
        const char*           name        = nullptr;             ///< Name of the image view.
        Handle<Image>         image       = NullHandle;          ///< Handle to the image.
        Format                format      = Format::Unknown;     ///< Format used to override original image format
        ImageViewType         viewType    = ImageViewType::None; ///< Type of the image view.
        ComponentMapping      components  = {};                  ///< Component mapping.
        ImageSubresourceRange subresource = {};                  ///< Subresource range.

        inline bool           operator==(const ImageViewCreateInfo& other) const
        {
            return components == other.components && viewType == other.viewType && subresource == other.subresource;
        }
    };

    inline static size_t CalcaulteImageSize(Format format, ImageSize3D size, uint32_t mipLevelsCount, uint32_t arrayCount)
    {
        size_t imageSizeBytes = 0;
        size_t formatByteSize = GetFormatByteSize(format);
        for (uint32_t mip = 0; mip < mipLevelsCount; ++mip)
        {
            uint32_t mipWidth  = std::max(1u, size.width >> mip);
            uint32_t mipHeight = std::max(1u, size.height >> mip);
            uint32_t mipDepth  = std::max(1u, size.depth >> mip);
            imageSizeBytes += mipWidth * mipHeight * mipDepth * formatByteSize;
        }
        imageSizeBytes *= arrayCount;
        return imageSizeBytes;
    }

} // namespace RHI