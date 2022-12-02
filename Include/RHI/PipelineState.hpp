#pragma once
#include "RHI/Common.hpp"
#include "RHI/Resource.hpp"

namespace RHI
{

enum class EFormat;

class ShaderResourceGroupLayout;
class IShaderProgram;
class IRenderPass;

enum class ERasterizationCullMode
{
    None,
    FrontFace,
    BackFace,
};

enum class ERasterizationFillMode
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
    EFormat     format;
};

struct GraphicsPipelineRasterizationState
{
    ERasterizationCullMode cullMode = ERasterizationCullMode::BackFace;
    ERasterizationFillMode fillMode = ERasterizationFillMode::Triangle;
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

} // namespace RHI
