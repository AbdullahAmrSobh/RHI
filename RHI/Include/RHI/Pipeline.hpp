#pragma once
#include "RHI/Common.hpp"
#include "RHI/SampleCount.hpp"
#include "RHI/ShaderModule.hpp"
#include "RHI/Span.hpp"

namespace RHI
{

/// @brief Pipeline vertex
enum class PipelineVertexInputRate
{
    PerInstance,
    PerVertex,
};

/// @brief Pipeline rasterizer state cull mode.
enum class PipelineRasterizerStateCullMode
{
    None,
    FrontFace,
    BackFace,
    Discard,
};

/// @brief Pipeline rasterizer state fill mode.
enum class PipelineRasterizerStateFillMode
{
    Point,
    Triangle,
    Line
};

/// @brief Pipeline topology mode.
enum class PipelineTopologyMode
{
    // ...
};

/// @brief The orientation of triangle fornt faces
enum class PipelineRasterizerStateFrontFace
{
    Clockwise,
    CounterClockwise,
};

/// @brief Operator used to compare two values
enum class CompareOperator
{
    Never,
    Equal,
    NotEqual,
    Greater,
    GreaterOrEqual,
    Less,
    LessOrEqual,
    Always,
};

/// @brief Enumeration representing the sampler filtering.
enum class SamplerFilter
{
    Point,
    Linear,
};

/// @brief Enumeration representing the sampler addressing mode.
enum class SamplerAddressMode
{
    Repeat,
    Clamp,
};

/// @brief Enumeration representing the sampler compare opertion.
enum class SamplerCompareOperation
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

/// @brief Enumerates ...
enum class BlendFactor
{
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
};

/// @brief Enumerates ...
enum class BlendEquation
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
};

/// @brief Structure specifying the blending parameters for an image render target attachment.
struct ColorAttachmentBlendStateDesc
{
    bool          blendEnable = false;
    BlendEquation colorBlendOp;
    BlendFactor   srcColor;
    BlendFactor   dstColor;
    BlendEquation alphaBlendOp;
    BlendFactor   srcAlpha;
    BlendFactor   dstAlpha;

    inline bool operator==(const ColorAttachmentBlendStateDesc& other) const
    {
        return blendEnable == other.blendEnable && colorBlendOp == other.colorBlendOp && srcColor == other.srcColor && dstColor == other.dstColor && alphaBlendOp == other.alphaBlendOp && srcAlpha == other.srcAlpha && dstAlpha == other.dstAlpha;
    }

    inline bool operator!=(const ColorAttachmentBlendStateDesc& other) const
    {
        return !(blendEnable == other.blendEnable && colorBlendOp == other.colorBlendOp && srcColor == other.srcColor && dstColor == other.dstColor && alphaBlendOp == other.alphaBlendOp && srcAlpha == other.srcAlpha && dstAlpha == other.dstAlpha);
    }
};

/// @brief Structure specifying a shader stage state.
struct PipelineShaderStage
{
    std::string entryName;

    ShaderModule* shader;

    ShaderStage stage;
};

/// @brief Structure specifying the render target layout.
struct PipelineRenderTargetLayout
{
    /// @brief List of the formats of color attachments.
    TL::Span<const Format> colorAttachmentsFormat;

    /// @brief Format of an optional depth and/or stencil attachment.
    /// Could be Format::None.
    Format depthAttachmentFormat;
};

/// @brief Structure specifying the vertex input state.
struct PipelineVertexInputStateDesc
{
    TL::Span<const Format> vertexAttributes;
    TL::Span<const Format> instanceAttributes;
};

/// @brief Structure specifying the rasterizer state.
struct PipelineRasterizerStateDesc
{
    PipelineRasterizerStateCullMode cullMode;

    PipelineRasterizerStateFillMode fillMode;

    PipelineRasterizerStateFrontFace frontFace;

    /// in line rendering specifies the width a the rasterized lines.
    float lineWidth;
};

/// @brief Structure specifying the multisample state.
struct PipelineMultisampleStateDesc
{
    SampleCount sampleCount;

    bool sampleShading;
};

/// @brief Structure specifying the depth and stencil state.
struct PipelineDepthStencilStateDesc
{
    bool depthTestEnable;

    bool depthWriteEnable;

    CompareOperator compareOperator;

    bool stencilTestEnable;
};

/// @brief Structure specifying the color attachments blend state.
struct PipelineColorBlendStateDesc
{
    bool independentBlending;

    TL::Span<const ColorAttachmentBlendStateDesc> blendStates;

    float blendConstants[4];
};

/// @brief Description of the graphics pipeline states.
struct GraphicsPipelineCreateInfo
{
    PipelineShaderStage vertexShader;

    PipelineShaderStage pixelShader;

    TL::Span<const ShaderBindGroupLayout> bindGroupLayouts;

    PipelineRenderTargetLayout renderTargetLayout;

    PipelineVertexInputStateDesc vertexInputState;

    PipelineRasterizerStateDesc rasterizationState;

    PipelineMultisampleStateDesc multisampleState;

    PipelineDepthStencilStateDesc depthStencilState;

    PipelineColorBlendStateDesc colorBlendState;
};

/// @brief Description of a compute pipeline state.
struct ComputePipelineCreateInfo
{
    PipelineShaderStage computeShader;

    TL::Span<const ShaderBindGroupLayout> shaderInputLayout;
};

/// @brief Structure describing the creation parameters of a sampler state.
struct SamplerCreateInfo
{
    SamplerCreateInfo() = default;

    SamplerFilter           filterMin  = SamplerFilter::Point;
    SamplerFilter           filterMag  = SamplerFilter::Point;
    SamplerFilter           filterMip  = SamplerFilter::Point;
    SamplerCompareOperation compare    = SamplerCompareOperation::Always;
    float                   mipLodBias = 0.0f;
    SamplerAddressMode      addressU   = SamplerAddressMode::Clamp;
    SamplerAddressMode      addressV   = SamplerAddressMode::Clamp;
    SamplerAddressMode      addressW   = SamplerAddressMode::Clamp;
    float                   minLod     = 0.0f;
    float                   maxLod     = 1.0f;

    inline bool operator==(const SamplerCreateInfo& other) const
    {
        return filterMin == other.filterMin && filterMag == other.filterMag && filterMip == other.filterMip && compare == other.compare && mipLodBias == other.mipLodBias && addressU == other.addressU && addressV == other.addressV && addressW == other.addressW && minLod == other.minLod && maxLod == other.maxLod;
    }

    inline bool operator!=(const SamplerCreateInfo& other) const
    {
        return !(filterMin == other.filterMin && filterMag == other.filterMag && filterMip == other.filterMip && compare == other.compare && mipLodBias == other.mipLodBias && addressU == other.addressU && addressV == other.addressV && addressW == other.addressW && minLod == other.minLod && maxLod == other.maxLod);
    }
};

// clang-format off
struct GraphicsPipeline {};
struct ComputePipeline {};
struct Sampler {};
// clang-format on

}  // namespace RHI