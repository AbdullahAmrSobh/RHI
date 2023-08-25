#pragma once
#include "RHI/Handle.hpp"
#include "RHI/Span.hpp"
#include "RHI/ShaderBindGroup.hpp"

namespace RHI
{

class ShaderModule;

/// @brief Pipeline shader stage type.
enum class ShaderStage
{
    Vertex,
    Pixel,
    Compute,
};

/// @brief Pipeline rasterizer state cull mode.
enum class PipelineRasterizerStateCullMode
{
    None,
    FrontFace,
    BackFace,
};

/// @brief Pipeline rasterizer state fill mode.
enum class PipelineRasterizerStateFillMode
{
    Point,
    Triangle,
    Line
};

/// @brief The orientation of triangle fornt faces
enum class PipelineRasterizerStateFrontFace
{
    Clockwise,
    CounterClockwise,
};

/// @brief The number of samples in multisample state.
enum class PipelineMultisampleSampleCount
{
    Count1,
    Count2,
    Count4,
    Count8,
    Count16,
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
    TL::Span<Format> colorAttachmentsFormat;

    /// @brief Format of an optional depth and/or stencil attachment.
    /// Could be Format::None.
    Format depthAttachmentFormat;
};

/// @brief Structure specifying the vertex input state.
struct PipelineVertexInputStateDesc
{
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
    PipelineMultisampleSampleCount sampleCount;

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

/// @brief Description of a graphics pipeline states.
struct GraphicsPipelineStateCreateInfo
{
    PipelineShaderStage vertexShader;

    PipelineShaderStage pixelShader;

    TL::Span<ShaderBindGroupLayout> bindGroupLayouts;

    PipelineRenderTargetLayout renderTargetLayout;

    PipelineVertexInputStateDesc vertexInputState;

    PipelineRasterizerStateDesc rasterizationState;

    PipelineMultisampleStateDesc multisampleState;

    PipelineDepthStencilStateDesc depthStencilState;
};

/// @brief Description of a compute pipeline state.
struct ComputePipelineStateCreateInfo
{
    PipelineShaderStage computeShader;

    TL::Span<ShaderBindGroupLayout> shaderInputLayout;
};

}  // namespace RHI