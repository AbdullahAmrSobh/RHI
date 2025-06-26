#pragma once

#include "RHI/Format.hpp"
#include "RHI/Handle.hpp"
#include "RHI/PipelineAccess.hpp"

#include <TL/Flags.hpp>
#include <TL/Span.hpp>

namespace RHI
{
    // TODO: Replace compare operatrs with default

    // Constants
    static constexpr uint64_t WholeSize         = UINT64_MAX;
    static constexpr uint32_t BindlessArraySize = UINT32_MAX;
    static constexpr uint8_t  AllLayers         = UINT8_MAX;
    static constexpr uint8_t  AllMipLevels      = UINT8_MAX;

    // Forward Declarations
    struct Buffer;
    struct Image;
    struct Sampler;

    class ShaderModule;

    // Opaque Resource Declarations
    using DeviceMemoryPtr = void*;
    RHI_DECLARE_OPAQUE_RESOURCE(BindGroupLayout);
    RHI_DECLARE_OPAQUE_RESOURCE(BindGroup);
    RHI_DECLARE_OPAQUE_RESOURCE(Buffer);
    RHI_DECLARE_OPAQUE_RESOURCE(Image);
    RHI_DECLARE_OPAQUE_RESOURCE(Sampler);
    RHI_DECLARE_OPAQUE_RESOURCE(PipelineLayout);
    RHI_DECLARE_OPAQUE_RESOURCE(GraphicsPipeline);
    RHI_DECLARE_OPAQUE_RESOURCE(ComputePipeline);

#define RHI_FLAG_NAME_RESERVED 0

    // Enums (General Purpose)
    enum class BindingType
    {
        None,                                                     ///< No binding.
        Sampler,                                                  ///< Sampler resource.
        SampledImage,                                             ///< Sampled image (read-only texture).
        StorageImage,                                             ///< Storage image (read/write texture).
        UniformBuffer,                                            ///< Uniform buffer (constant data).
        StorageBuffer,                                            ///< Storage buffer (read/write data).
        DynamicUniformBuffer,                                     ///< Dynamic uniform buffer.
        DynamicStorageBuffer,                                     ///< Dynamic storage buffer.
        BufferView,                                               ///< Buffer view.
        StorageBufferView,                                        ///< Storage buffer view.
        InputAttachment                 = RHI_FLAG_NAME_RESERVED, ///<
        RayTracingAccelerationStructure = RHI_FLAG_NAME_RESERVED, ///<
        Count,                                                    ///< Number of binding types.
    };

    enum class ShaderStage
    {
        None          = 0 << 0,                 ///< No shader stage.
        Vertex        = 1 << 1,                 ///< Vertex shader stage.
        Pixel         = 1 << 2,                 ///< Pixel (fragment) shader stage.
        Compute       = 1 << 3,                 ///< Compute shader stage.
        Hull          = RHI_FLAG_NAME_RESERVED, ///< Hull (tessellation control) shader stage.
        Domain        = RHI_FLAG_NAME_RESERVED, ///< Domain (tessellation evaluation) shader stage.
        RayGen        = RHI_FLAG_NAME_RESERVED, ///< Ray generation shader stage.
        RayIntersect  = RHI_FLAG_NAME_RESERVED, ///< Ray intersection shader stage.
        RayAnyHit     = RHI_FLAG_NAME_RESERVED, ///< Ray any-hit shader stage.
        RayClosestHit = RHI_FLAG_NAME_RESERVED, ///< Ray closest-hit shader stage.
        RayMiss       = RHI_FLAG_NAME_RESERVED, ///< Ray miss shader stage.
        RayCallable   = RHI_FLAG_NAME_RESERVED, ///< Ray callable shader stage.
        Mesh          = RHI_FLAG_NAME_RESERVED, ///< Mesh shader stage.
        Amplification = RHI_FLAG_NAME_RESERVED, ///< Amplification shader stage.
        AllGraphics   = Vertex | Pixel,
        AllStages     = AllGraphics | Compute,
    };

    TL_DEFINE_FLAG_OPERATORS(ShaderStage);

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

    // Enums (Image Related)
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
        Present         = 1 << 8,          ///< Image is used for presentation.
    };

    TL_DEFINE_FLAG_OPERATORS(ImageUsage);

    enum class ImageType
    {
        None,    ///< No image type.
        Image1D, ///< 1D image.
        Image2D, ///< 2D image.
        Image3D, ///< 3D image.
    };

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

    // Enums (Pipeline Related)
    enum class PipelineVertexInputRate
    {
        None,        ///< No input rate.
        PerInstance, ///< Input rate per instance.
        PerVertex,   ///< Input rate per vertex.
    };

    enum class PipelineRasterizerStateCullMode
    {
        None,      ///< No culling.
        FrontFace, ///< Cull front-facing polygons.
        BackFace,  ///< Cull back-facing polygons.
        Discard,   ///< Discard primitives without culling.
    };

    enum class PipelineRasterizerStateFillMode
    {
        Point,    ///< Points.
        Triangle, ///< Triangles.
        Line,     ///< Lines.
    };

    enum class PipelineTopologyMode
    {
        Points,    ///< Points.
        Lines,     ///< Lines.
        Triangles, ///< Triangles.
    };

    enum class PipelineRasterizerStateFrontFace
    {
        Clockwise,        ///< Clockwise front face.
        CounterClockwise, ///< Counter-clockwise front face.
    };

    enum class BlendFactor
    {
        Zero,                  ///< Zero.
        One,                   ///< One.
        SrcColor,              ///< Source color.
        OneMinusSrcColor,      ///< One minus source color.
        DstColor,              ///< Destination color.
        OneMinusDstColor,      ///< One minus destination color.
        SrcAlpha,              ///< Source alpha.
        OneMinusSrcAlpha,      ///< One minus source alpha.
        DstAlpha,              ///< Destination alpha.
        OneMinusDstAlpha,      ///< One minus destination alpha.
        ConstantColor,         ///< Constant color.
        OneMinusConstantColor, ///< One minus constant color.
        ConstantAlpha,         ///< Constant alpha.
        OneMinusConstantAlpha, ///< One minus constant alpha.
    };

    enum class BlendEquation
    {
        Add,             ///< Addition.
        Subtract,        ///< Subtraction.
        ReverseSubtract, ///< Reverse subtraction.
        Min,             ///< Minimum.
        Max,             ///< Maximum.
    };

    enum class ColorWriteMask
    {
        Red   = 0x01,                       ///< Red channel.
        Green = 0x02,                       ///< Green channel.
        Blue  = 0x04,                       ///< Blue channel.
        Alpha = 0x08,                       ///< Alpha channel.
        All   = Red | Green | Blue | Alpha, ///< All channels.
    };

    TL_DEFINE_FLAG_OPERATORS(ColorWriteMask);

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

    TL_DEFINE_FLAG_OPERATORS(SampleCount);

    // Enums (Sampler Related)
    enum class SamplerFilter
    {
        Point,  ///< Uses nearest neighbor filtering.
        Linear, ///< Uses linear filtering.
    };

    enum class SamplerAddressMode
    {
        Repeat, ///< Repeats the texture when UV coordinates are outside the range [0,1].
        Clamp,  ///< Clamps UV coordinates to the edge of the texture.
    };

    enum class CompareOperator
    {
        Undefined,      ///< Undefined.
        Never,          ///< Never passes.
        Equal,          ///< Passes if equal.
        NotEqual,       ///< Passes if not equal.
        Greater,        ///< Passes if greater.
        GreaterOrEqual, ///< Passes if greater or equal.
        Less,           ///< Passes if less.
        LessOrEqual,    ///< Passes if less or equal.
        Always,         ///< Always passes.
    };

    // Structs (General Purpose)
    struct BufferSubregion
    {
        size_t offset = 0;         ///< Offset into the buffer.
        size_t size   = WholeSize; ///< Size of the subregion.

        bool   operator==(const BufferSubregion& other) const = default;
    };

    /// @brief Contains information about binding a buffer.
    struct BufferBindingInfo
    {
        Handle<Buffer> buffer = NullHandle; ///< Handle to the buffer.
        size_t         offset = 0;          ///< Offset into the buffer.
    };

    struct ShaderBinding
    {
        BindingType            type         = BindingType::None; ///< Type of the binding.
        Access                 access       = Access::Read;      ///< Access type (read/write) for the resource.
        uint32_t               arrayCount   = 1;                 ///< Number of elements in the array for this binding.
        TL::Flags<ShaderStage> stages       = ShaderStage::None; ///< Shader stages where this binding is accessible.
        size_t                 bufferStride = 0;                 ///< Stride for buffer bindings.
    };

    // Structs (Bind Group Related)
    struct BindGroupLayoutCreateInfo
    {
        const char*                   name     = nullptr; ///< Name of the bind group layout.
        TL::Span<const ShaderBinding> bindings = {};      ///< Span of shader bindings for this layout.
    };

    struct BindGroupCreateInfo
    {
        const char*             name               = nullptr;    ///!< Name of the bind group.
        Handle<BindGroupLayout> layout             = NullHandle; // !< The layout of the bind group.
        uint32_t                bindlessArrayCount = 0;          //!< Count of bindless array elements.
    };

    struct BindGroupImagesUpdateInfo
    {
        uint32_t                      dstBinding      = 0;  ///< Target binding index.
        uint32_t                      dstArrayElement = 0;  ///< Target array element for binding.
        TL::Span<const Handle<Image>> images          = {}; ///< Span of image views to bind.
    };

    struct BindGroupBuffersUpdateInfo
    {
        uint32_t                          dstBinding      = 0;  ///< Target binding index.
        uint32_t                          dstArrayElement = 0;  ///< Target array element for binding.
        TL::Span<const BufferBindingInfo> buffers         = {}; ///< Span of buffer handles to bind.
    };

    struct BindGroupSamplersUpdateInfo
    {
        uint32_t                        dstBinding      = 0;  ///< Target binding index.
        uint32_t                        dstArrayElement = 0;  ///< Target array element for binding.
        TL::Span<const Handle<Sampler>> samplers        = {}; ///< Span of sampler handles to bind.
    };

    struct BindGroupUpdateInfo
    {
        TL::Span<const BindGroupBuffersUpdateInfo>  buffers  = {}; ///< Buffer updates for the bind group.
        TL::Span<const BindGroupImagesUpdateInfo>   images   = {}; ///< Image updates for the bind group.
        TL::Span<const BindGroupSamplersUpdateInfo> samplers = {}; ///< Sampler updates for the bind group.
    };

    // Structs (Buffer Related)
    struct BufferCreateInfo
    {
        const char*            name       = nullptr;           ///< Name of the buffer.
        bool                   hostMapped = true;              ///< Buffer will be mappable by host.
        TL::Flags<BufferUsage> usageFlags = BufferUsage::None; ///< Usage flags for the buffer.
        size_t                 byteSize   = 0;                 ///< Size of the buffer in bytes.
    };

    // Structs (Image Related)
    struct ImageOffset2D
    {
        int32_t x = 0; ///< X coordinate offset.
        int32_t y = 0; ///< Y coordinate offset.

        bool    operator==(const ImageOffset2D& other) const = default;
    };

    struct ImageOffset3D
    {
        int32_t x = 0; ///< X coordinate offset.
        int32_t y = 0; ///< Y coordinate offset.
        int32_t z = 0; ///< Z coordinate offset.

        bool    operator==(const ImageOffset3D& other) const = default;
    };

    struct ImageSize2D
    {
        uint32_t width  = 1; ///< Width of the image.
        uint32_t height = 1; ///< Height of the image.

        bool     operator==(const ImageSize2D& other) const = default;
    };

    struct ImageSize3D
    {
        uint32_t width  = 1; ///< Width of the image.
        uint32_t height = 1; ///< Height of the image.
        uint32_t depth  = 1; ///< Depth of the image.

        bool     operator==(const ImageSize3D& other) const = default;
    };

    struct ComponentMapping
    {
        ComponentSwizzle r = ComponentSwizzle::Identity; ///< Red component swizzle.
        ComponentSwizzle g = ComponentSwizzle::Identity; ///< Green component swizzle.
        ComponentSwizzle b = ComponentSwizzle::Identity; ///< Blue component swizzle.
        ComponentSwizzle a = ComponentSwizzle::Identity; ///< Alpha component swizzle.

        bool             operator==(const ComponentMapping& other) const = default;
    };

    struct ImageSubresourceRange
    {
        TL::Flags<ImageAspect>       imageAspects  = ImageAspect::All; ///< Image aspects to access.
        uint8_t                      mipBase       = 0;                ///< Base mip level.
        uint8_t                      mipLevelCount = 1;                ///< Number of mip levels.
        uint8_t                      arrayBase     = 0;                ///< Base array layer.
        uint8_t                      arrayCount    = 1;                ///< Number of array layers.

        bool                         operator==(const ImageSubresourceRange& other) const = default;

        static ImageSubresourceRange All() { return {ImageAspect::All, 0, AllMipLevels, 0, AllLayers}; }
    };

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

    struct ImageViewCreateInfo
    {
        const char*           name        = nullptr;             ///< Name of the image view.
        Handle<Image>         image       = NullHandle;          ///< Handle to the image.
        Format                format      = Format::Unknown;     ///< Format used to override original image format
        ImageViewType         viewType    = ImageViewType::None; ///< Type of the image view.
        ComponentMapping      components  = {};                  ///< Component mapping.
        ImageSubresourceRange subresource = {};                  ///< Subresource range.

        bool                  operator==(const ImageViewCreateInfo& other) const = default;
    };

    // Structs (Sampler Related)
    struct SamplerCreateInfo
    {
        const char*        name       = nullptr;                    ///< Name of the sampler.
        SamplerFilter      filterMin  = SamplerFilter::Linear;      ///< Filter for minification.
        SamplerFilter      filterMag  = SamplerFilter::Linear;      ///< Filter for magnification.
        SamplerFilter      filterMip  = SamplerFilter::Linear;      ///< Filter for mipmap selection.
        CompareOperator    compare    = CompareOperator::Undefined; ///< Compare operation for texture comparison.
        float              mipLodBias = 0.0f;                       ///< Bias applied to the mip level of detail.
        SamplerAddressMode addressU   = SamplerAddressMode::Repeat; ///< Addressing mode for the U (horizontal) coordinate.
        SamplerAddressMode addressV   = SamplerAddressMode::Repeat; ///< Addressing mode for the V (vertical) coordinate.
        SamplerAddressMode addressW   = SamplerAddressMode::Repeat; ///< Addressing mode for the W (depth) coordinate.
        float              minLod     = 0.0f;                       ///< Minimum level of detail (LOD) that can be used.
        float              maxLod     = 1000.0f;                    ///< Maximum level of detail (LOD) that can be used.
    };

    // Structs (Pipeline Related)
    struct PipelineLayoutCreateInfo
    {
        const char*                             name    = nullptr; ///< Debug name of the pipeline layout object.
        TL::Span<const Handle<BindGroupLayout>> layouts = {};      ///< List of bind group layouts.
    };

    struct ColorAttachmentBlendStateDesc
    {
        bool                      blendEnable  = false;               ///< Enable blending.
        BlendEquation             colorBlendOp = BlendEquation::Add;  ///< Color blend equation.
        BlendFactor               srcColor     = BlendFactor::One;    ///< Source color blend factor.
        BlendFactor               dstColor     = BlendFactor::Zero;   ///< Destination color blend factor.
        BlendEquation             alphaBlendOp = BlendEquation::Add;  ///< Alpha blend equation.
        BlendFactor               srcAlpha     = BlendFactor::One;    ///< Source alpha blend factor.
        BlendFactor               dstAlpha     = BlendFactor::Zero;   ///< Destination alpha blend factor.
        TL::Flags<ColorWriteMask> writeMask    = ColorWriteMask::All; ///< Color write mask.
    };

    struct PipelineRenderTargetLayout
    {
        TL::Span<const Format> colorAttachmentsFormats = {};              ///< Formats of color attachments.
        Format                 depthAttachmentFormat   = Format::Unknown; ///< Format of depth attachment.
        Format                 stencilAttachmentFormat = Format::Unknown; ///< Format of stencil attachment.
    };

    struct PipelineVertexAttributeDesc
    {
        uint32_t offset = 0;               ///< Offset of the attribute.
        Format   format = Format::Unknown; ///< Format of the attribute.
    };

    struct PipelineVertexBindingDesc
    {
        uint32_t                                    stride     = 0;                                  ///< Stride between vertex data.
        PipelineVertexInputRate                     stepRate   = PipelineVertexInputRate::PerVertex; ///< Input rate.
        TL::Span<const PipelineVertexAttributeDesc> attributes = {};
    };

    struct PipelineRasterizerStateDesc
    {
        PipelineRasterizerStateCullMode  cullMode  = PipelineRasterizerStateCullMode::BackFace;          ///< Cull mode.
        PipelineRasterizerStateFillMode  fillMode  = PipelineRasterizerStateFillMode::Triangle;          ///< Fill mode.
        PipelineRasterizerStateFrontFace frontFace = PipelineRasterizerStateFrontFace::CounterClockwise; ///< Front face orientation.
        float                            lineWidth = 1.0f;                                               ///< Line width.
    };

    struct PipelineMultisampleStateDesc
    {
        SampleCount sampleCount   = SampleCount::Samples1; ///< Number of samples per pixel.
        bool        sampleShading = false;                 ///< Enable sample shading.
    };

    struct PipelineDepthStencilStateDesc
    {
        bool            depthTestEnable   = false;                 ///< Enable depth testing.
        bool            depthWriteEnable  = false;                 ///< Enable depth writing.
        CompareOperator compareOperator   = CompareOperator::Less; ///< Comparison operator for depth testing.
        bool            stencilTestEnable = false;                 ///< Enable stencil testing.
    };

    struct PipelineColorBlendStateDesc
    {
        TL::Span<const ColorAttachmentBlendStateDesc> blendStates       = ColorAttachmentBlendStateDesc{}; ///< Color blend states for each attachment.
        float                                         blendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f};        ///< Blend constants.
    };

    struct GraphicsPipelineCreateInfo
    {
        const char*                               name                 = nullptr;                         ///< Name of the pipeline.
        const char*                               vertexShaderName     = nullptr;                         ///< Name of the vertex shader.
        ShaderModule*                             vertexShaderModule   = nullptr;                         ///< Vertex shader module.
        const char*                               pixelShaderName      = nullptr;                         ///< Name of the pixel shader.
        ShaderModule*                             pixelShaderModule    = nullptr;                         ///< Pixel shader module.
        Handle<PipelineLayout>                    layout               = NullHandle;                      ///< Pipeline layout.
        TL::Span<const PipelineVertexBindingDesc> vertexBufferBindings = {};                              ///< Input assembler state.
        PipelineRenderTargetLayout                renderTargetLayout   = {};                              ///< Render target layout.
        PipelineColorBlendStateDesc               colorBlendState      = {};                              ///< Color blend state.
        PipelineTopologyMode                      topologyMode         = PipelineTopologyMode::Triangles; ///< Topology mode.
        PipelineRasterizerStateDesc               rasterizationState   = {};                              ///< Rasterizer state.
        PipelineMultisampleStateDesc              multisampleState     = {};                              ///< Multisample state.
        PipelineDepthStencilStateDesc             depthStencilState    = {};                              ///< Depth/stencil state.
    };

    struct ComputePipelineCreateInfo
    {
        const char*            name         = nullptr;    ///< Name of the pipeline.
        const char*            shaderName   = nullptr;    ///< Name of the compute shader.
        ShaderModule*          shaderModule = nullptr;    ///< Compute shader module.
        Handle<PipelineLayout> layout       = NullHandle; ///< Pipeline layout.
    };

    // Structs (Shader Related)
    struct ShaderModuleCreateInfo
    {
        const char*              name = nullptr;
        TL::Span<const uint32_t> code = {};
    };

    // Classes
    class RHI_EXPORT ShaderModule
    {
    public:
        RHI_INTERFACE_BOILERPLATE(ShaderModule);
    };

    inline static ImageSize3D CalcaulteImageMipSize(ImageSize3D size, uint32_t mipLevel)
    {
        return {
            std::max(1u, size.width >> mipLevel),
            std::max(1u, size.height >> mipLevel),
            std::max(1u, size.depth >> mipLevel),
        };
    }
} // namespace RHI
