#pragma once

#include "RHI/Resource.hpp"

namespace RHI
{

enum class Format;

class ShaderResourceGroupLayout;
class IShaderProgram;
class IRenderPass;

enum class RasterizationCullMode
{
    None,
    FrontFace,
    BackFace,
};

enum class RasterizationFillMode
{
    Point,
    Triangle,
    Line
};

struct GraphicsPipelineShaderStages
{
    IShaderProgram* pVertexShader;
    IShaderProgram* pTessControlShader;
    IShaderProgram* pTessEvalShader;
    IShaderProgram* pGeometryShader;
    IShaderProgram* pPixelShader;
};

struct GraphicsPipelineVertexAttributeState
{
    std::string name;
    Format      format;
};

struct GraphicsPipelineRasterizationState
{
    RasterizationFillMode fillMode = RasterizationFillMode::Triangle;
    RasterizationCullMode cullMode = RasterizationCullMode::BackFace;
};

struct GraphicsPipelineDepthStencilState
{
    bool depthEnabled  = false;
    bool stencilEnable = false;
};

struct GraphicsPipelineColorBlendState
{
    // !TODO
};

struct PipelineLayoutDesc
{
    std::vector<ShaderResourceGroupLayout> shaderBindingGroupLayouts;
};

struct GraphicsPipelineStateDesc
{
    GraphicsPipelineStateDesc() = default;

    PipelineLayoutDesc                                pipelineLayoutDesc;
    const IRenderPass*                                pRenderPass;
    GraphicsPipelineShaderStages                      shaderStages;
    std::vector<GraphicsPipelineVertexAttributeState> vertexInputAttributes;
    GraphicsPipelineRasterizationState                rasterizationState;
    GraphicsPipelineColorBlendState                   colorBlendState;
    GraphicsPipelineDepthStencilState                 depthStencil;
};

class IPipelineState
{
public:
    virtual ~IPipelineState() = default;
};

}  // namespace RHI
