#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/PipelineLayout.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Shader.hpp"

#include "RHI/RenderGraph.hpp"

namespace RHI
{

enum class EVertexAttributeFormat
{
    Float,
    Float2,
    Float3,
    Float4,
    UInt,
    UInt2,
    UInt3,
    UInt4,
    Int,
    Int2,
    Int3,
    Int4,
    MaxEnum,
};

enum class ERasterizationCullMode
{
    FrontFace,
    BackFace,
    Discard
};
enum class ERasterizationFillMode
{
    Point,
    Triangle,
    Line
};

struct PipelineRasterizationStateDesc
{
    ERasterizationCullMode cullmode = ERasterizationCullMode::BackFace;
    ERasterizationFillMode fillmode = ERasterizationFillMode::Triangle;
};

struct PipelineStateMultisampleStateDesc
{
    ESampleCount sampleCount;
};

struct PipelineDepthStencilStateDesc
{
    bool depthEnabled  = false;
    bool stencilEnable = false;
};

struct PipelineStateColorBlendStateDesc
{
};

struct PipelineStateRenderTargetLayout
{
	PipelineStateRenderTargetLayout() = default;
    PipelineStateRenderTargetLayout(const std::vector<EPixelFormat>& colorFormats, EPixelFormat depthStencilFormat = EPixelFormat::None)
        : colorFormats(std::move(colorFormats))
        , depthStencilFormat(depthStencilFormat)
    {
    }
    
    std::vector<EPixelFormat> colorFormats;
    EPixelFormat              depthStencilFormat;
};

struct GraphicsPipelineStateDesc
{
    explicit GraphicsPipelineStateDesc(const IPipelineLayout& layout, GraphicsPipelineShaders shaders)
        : pPipelineLayout(&layout)
        , shaders(shaders)
    {
    }

    const IPipelineLayout*              pPipelineLayout;
    GraphicsPipelineShaders             shaders;
    std::vector<EVertexAttributeFormat> vertexAttributes;
    PipelineRasterizationStateDesc      rasterizationState;
    PipelineStateMultisampleStateDesc   multisampleState;
    PipelineDepthStencilStateDesc       depthStencilState;
    PipelineStateColorBlendStateDesc    colorBlendState;
    PipelineStateRenderTargetLayout		renderTargetLayout;
};

struct ComputePipelineStateDesc
{
    explicit ComputePipelineStateDesc(const IPipelineLayout& layout, ComputePipelineShader shader)
        : shader(shader)
    {
    }

    const IPipelineLayout* pPipelineLayout;
    ComputePipelineShader  shader;
};

class IPipelineState
{
public:
    virtual ~IPipelineState() = default;
};
using PipelineStatePtr = Unique<IPipelineState>;

} // namespace RHI
