#pragma once
#include "RHI/Handle.hpp"
#include "RHI/ShaderBindGroup.hpp"
#include "RHI/Span.hpp"
#include "RHI/Format.hpp"
#include "RHI/Flags.hpp"

namespace RHI
{

class GraphicsPipeline;
class ComputePipeline;
class Sampler;

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

class ShaderModule : public Object
{
public:
    using Object::Object;
    virtual ~ShaderModule() = default;

    const Flags<ShaderStage> m_shaderStages;

    const char* const m_vertexShaderName;
    const char* const m_pixelShaderName;
    const char* const m_computeShaderName;

    const std::vector<ShaderBindGroupLayout> m_bindGroupLayouts; // reflected from the actual shader data
};

}  // namespace RHI