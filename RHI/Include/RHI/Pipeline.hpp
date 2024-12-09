#pragma once

#include "RHI/Handle.hpp"
#include "RHI/Format.hpp"
#include "RHI/SampleCount.hpp"

namespace RHI
{
    RHI_DECLARE_OPAQUE_RESOURCE(PipelineLayout);
    RHI_DECLARE_OPAQUE_RESOURCE(GraphicsPipeline);
    RHI_DECLARE_OPAQUE_RESOURCE(ComputePipeline);

    struct BindGroupLayout;
    struct BindGroup;

    class ShaderModule;

    /// @brief Shader stages.
    enum class ShaderStage
    {
        None    = 0 << 0, ///< No shader stage.
        Vertex  = 1 << 1, ///< Vertex shader stage.
        Pixel   = 1 << 2, ///< Pixel (fragment) shader stage.
        Compute = 1 << 3, ///< Compute shader stage.
    };

    TL_DEFINE_FLAG_OPERATORS(ShaderStage);

    /// @brief Vertex input rates for pipelines.
    enum class PipelineVertexInputRate
    {
        None,        ///< No input rate.
        PerInstance, ///< Input rate per instance.
        PerVertex,   ///< Input rate per vertex.
    };

    /// @brief Cull modes for rasterizer state.
    enum class PipelineRasterizerStateCullMode
    {
        None,      ///< No culling.
        FrontFace, ///< Cull front-facing polygons.
        BackFace,  ///< Cull back-facing polygons.
        Discard,   ///< Discard primitives without culling.
    };

    /// @brief Fill modes for rasterizer state.
    enum class PipelineRasterizerStateFillMode
    {
        Point,    ///< Points.
        Triangle, ///< Triangles.
        Line,     ///< Lines.
    };

    /// @brief Topology modes for pipelines.
    enum class PipelineTopologyMode
    {
        Points,    ///< Points.
        Lines,     ///< Lines.
        Triangles, ///< Triangles.
    };

    /// @brief Front face orientations for rasterizer state.
    enum class PipelineRasterizerStateFrontFace
    {
        Clockwise,        ///< Clockwise front face.
        CounterClockwise, ///< Counter-clockwise front face.
    };

    /// @brief Comparison operators for depth/stencil testing.
    enum class CompareOperator
    {
        Never,          ///< Never passes.
        Equal,          ///< Passes if equal.
        NotEqual,       ///< Passes if not equal.
        Greater,        ///< Passes if greater.
        GreaterOrEqual, ///< Passes if greater or equal.
        Less,           ///< Passes if less.
        LessOrEqual,    ///< Passes if less or equal.
        Always,         ///< Always passes.
    };

    /// @brief Blend factors for blending operations.
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

    /// @brief Blend equations for color and alpha blending.
    enum class BlendEquation
    {
        Add,             ///< Addition.
        Subtract,        ///< Subtraction.
        ReverseSubtract, ///< Reverse subtraction.
        Min,             ///< Minimum.
        Max,             ///< Maximum.
    };

    /// @brief Color write masks for color attachments.
    enum class ColorWriteMask
    {
        Red   = 0x01,                       ///< Red channel.
        Green = 0x02,                       ///< Green channel.
        Blue  = 0x04,                       ///< Blue channel.
        Alpha = 0x08,                       ///< Alpha channel.
        All   = Red | Green | Blue | Alpha, ///< All channels.
    };

    TL_DEFINE_FLAG_OPERATORS(ColorWriteMask);

    /// @brief Information required to create a pipeline layout.
    struct PipelineLayoutCreateInfo
    {
        const char*                             name    = nullptr; ///< Debug name of the pipeline layout object.
        TL::Span<const Handle<BindGroupLayout>> layouts = {};      ///< List of bind group layouts.
    };

    /// @brief Description of color attachment blend state.
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

    /// @brief Layout of render targets in a pipeline.
    struct PipelineRenderTargetLayout
    {
        TL::Span<const Format> colorAttachmentsFormats = {};              ///< Formats of color attachments.
        Format                 depthAttachmentFormat   = Format::Unknown; ///< Format of depth attachment.
        Format                 stencilAttachmentFormat = Format::Unknown; ///< Format of stencil attachment.
    };

    /// @brief Description of vertex attributes in a pipeline.
    struct PipelineVertexAttributeDesc
    {
        uint32_t offset = 0;               ///< Offset of the attribute.
        Format   format = Format::Unknown; ///< Format of the attribute.
    };

    /// @brief Description of vertex binding in a pipeline.
    struct PipelineVertexBindingDesc
    {
        uint32_t                                    stride     = 0;                                  ///< Stride between vertex data.
        PipelineVertexInputRate                     stepRate   = PipelineVertexInputRate::PerVertex; ///< Input rate.
        TL::Span<const PipelineVertexAttributeDesc> attributes = {};
    };

    /// @brief Rasterizer state description for pipelines.
    struct PipelineRasterizerStateDesc
    {
        PipelineRasterizerStateCullMode  cullMode  = PipelineRasterizerStateCullMode::BackFace;          ///< Cull mode.
        PipelineRasterizerStateFillMode  fillMode  = PipelineRasterizerStateFillMode::Triangle;          ///< Fill mode.
        PipelineRasterizerStateFrontFace frontFace = PipelineRasterizerStateFrontFace::CounterClockwise; ///< Front face orientation.
        float                            lineWidth = 1.0f;                                               ///< Line width.
    };

    /// @brief Multisample state description for pipelines.
    struct PipelineMultisampleStateDesc
    {
        SampleCount sampleCount   = SampleCount::Samples1; ///< Number of samples per pixel.
        bool        sampleShading = false;                 ///< Enable sample shading.
    };

    /// @brief Depth/stencil state description for pipelines.
    struct PipelineDepthStencilStateDesc
    {
        bool            depthTestEnable   = false;                 ///< Enable depth testing.
        bool            depthWriteEnable  = false;                 ///< Enable depth writing.
        CompareOperator compareOperator   = CompareOperator::Less; ///< Comparison operator for depth testing.
        bool            stencilTestEnable = false;                 ///< Enable stencil testing.
    };

    /// @brief Color blend state description for pipelines.
    struct PipelineColorBlendStateDesc
    {
        TL::Span<const ColorAttachmentBlendStateDesc> blendStates       = {}; ///< Color blend states for each attachment.
        float                                         blendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f}; ///< Blend constants.
    };

    /// @brief Information required to create a graphics pipeline.
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

    /// @brief Information required to create a compute pipeline.
    struct ComputePipelineCreateInfo
    {
        const char*            name         = nullptr;    ///< Name of the pipeline.
        const char*            shaderName   = nullptr;    ///< Name of the compute shader.
        ShaderModule*          shaderModule = nullptr;    ///< Compute shader module.
        Handle<PipelineLayout> layout       = NullHandle; ///< Pipeline layout.
    };
} // namespace RHI
