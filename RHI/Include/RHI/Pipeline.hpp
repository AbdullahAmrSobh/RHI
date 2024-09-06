#pragma once

#include "RHI/Handle.hpp"
#include "RHI/Format.hpp"
#include "RHI/SampleCount.hpp"

namespace RHI
{
    /// @brief Maximum number of render target attachments.
    inline static constexpr uint32_t c_MaxRenderTargetAttachmentsCount = 16u;

    /// @brief Maximum number of vertex bindings in a pipeline.
    inline static constexpr uint32_t c_MaxPipelineVertexBindings = 32u;

    /// @brief Maximum number of vertex attributes in a pipeline.
    inline static constexpr uint32_t c_MaxPipelineVertexAttributes = 32u;

    /// @brief Maximum number of bind groups in a pipeline.
    inline static constexpr uint32_t c_MaxPipelineBindGroupsCount = 4u;

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
        Handle<BindGroupLayout> layouts[c_MaxPipelineBindGroupsCount]; ///< Array of bind group layouts.
    };

    /// @brief Description of color attachment blend state.
    struct ColorAttachmentBlendStateDesc
    {
        bool                      blendEnable;  ///< Enable blending.
        BlendEquation             colorBlendOp; ///< Color blend equation.
        BlendFactor               srcColor;     ///< Source color blend factor.
        BlendFactor               dstColor;     ///< Destination color blend factor.
        BlendEquation             alphaBlendOp; ///< Alpha blend equation.
        BlendFactor               srcAlpha;     ///< Source alpha blend factor.
        BlendFactor               dstAlpha;     ///< Destination alpha blend factor.
        TL::Flags<ColorWriteMask> writeMask;    ///< Color write mask.

        inline bool               operator==(const ColorAttachmentBlendStateDesc& other) const
        {
            return blendEnable == other.blendEnable &&
                   colorBlendOp == other.colorBlendOp &&
                   srcColor == other.srcColor &&
                   dstColor == other.dstColor &&
                   alphaBlendOp == other.alphaBlendOp &&
                   srcAlpha == other.srcAlpha &&
                   dstAlpha == other.dstAlpha;
        }
    };

    /// @brief Layout of render targets in a pipeline.
    struct PipelineRenderTargetLayout
    {
        Format colorAttachmentsFormats[c_MaxRenderTargetAttachmentsCount]; ///< Formats of color attachments.
        Format depthAttachmentFormat;                                      ///< Format of depth attachment.
        Format stencilAttachmentFormat;                                    ///< Format of stencil attachment.
    };

    /// @brief Description of vertex binding in a pipeline.
    struct PipelineVertexBindingDesc
    {
        uint32_t                binding;  ///< Binding index.
        uint32_t                stride;   ///< Stride between vertex data.
        PipelineVertexInputRate stepRate; ///< Input rate.
    };

    /// @brief Description of vertex attributes in a pipeline.
    struct PipelineVertexAttributeDesc
    {
        uint32_t location; ///< Attribute location.
        uint32_t binding;  ///< Binding index.
        Format   format;   ///< Format of the attribute.
        uint32_t offset;   ///< Offset of the attribute.
    };

    /// @brief Input assembler state description for pipelines.
    struct PipelineInputAssemblerStateDesc
    {
        PipelineVertexBindingDesc   bindings[c_MaxPipelineVertexBindings];     ///< Vertex bindings.
        PipelineVertexAttributeDesc attributes[c_MaxPipelineVertexAttributes]; ///< Vertex attributes.
    };

    /// @brief Rasterizer state description for pipelines.
    struct PipelineRasterizerStateDesc
    {
        PipelineRasterizerStateCullMode  cullMode;  ///< Cull mode.
        PipelineRasterizerStateFillMode  fillMode;  ///< Fill mode.
        PipelineRasterizerStateFrontFace frontFace; ///< Front face orientation.
        float                            lineWidth; ///< Line width.
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
        bool            depthTestEnable;   ///< Enable depth testing.
        bool            depthWriteEnable;  ///< Enable depth writing.
        CompareOperator compareOperator;   ///< Comparison operator for depth testing.
        bool            stencilTestEnable; ///< Enable stencil testing.
    };

    /// @brief Color blend state description for pipelines.
    struct PipelineColorBlendStateDesc
    {
        ColorAttachmentBlendStateDesc blendStates[c_MaxRenderTargetAttachmentsCount]; ///< Color blend states for each attachment.
        float                         blendConstants[4];                              ///< Blend constants.
    };

    /// @brief Information required to create a graphics pipeline.
    struct GraphicsPipelineCreateInfo
    {
        const char*                     name;                ///< Name of the pipeline.
        const char*                     vertexShaderName;    ///< Name of the vertex shader.
        ShaderModule*                   vertexShaderModule;  ///< Vertex shader module.
        const char*                     pixelShaderName;     ///< Name of the pixel shader.
        ShaderModule*                   pixelShaderModule;   ///< Pixel shader module.
        Handle<PipelineLayout>          layout;              ///< Pipeline layout.
        PipelineInputAssemblerStateDesc inputAssemblerState; ///< Input assembler state.
        PipelineRenderTargetLayout      renderTargetLayout;  ///< Render target layout.
        PipelineColorBlendStateDesc     colorBlendState;     ///< Color blend state.
        PipelineTopologyMode            topologyMode;        ///< Topology mode.
        PipelineRasterizerStateDesc     rasterizationState;  ///< Rasterizer state.
        PipelineMultisampleStateDesc    multisampleState;    ///< Multisample state.
        PipelineDepthStencilStateDesc   depthStencilState;   ///< Depth/stencil state.
    };

    /// @brief Information required to create a compute pipeline.
    struct ComputePipelineCreateInfo
    {
        const char*            name;         ///< Name of the pipeline.
        const char*            shaderName;   ///< Name of the compute shader.
        ShaderModule*          shaderModule; ///< Compute shader module.
        Handle<PipelineLayout> layout;       ///< Pipeline layout.
    };
} // namespace RHI
