#pragma once
#include <string>
#include <vector>

#include "RHI/Format.hpp"
#include "RHI/ShaderResourceBindings.hpp"

namespace RHI
{

class IShaderProgram;

namespace PipelineStateDesc
{
    struct GraphicsShaderStages
    {
        IShaderProgram* pVertexShader;
        IShaderProgram* pTessControlShader;
        IShaderProgram* pTessEvalShader;
        IShaderProgram* pGeometryShader;
        IShaderProgram* pPixelShader;
    };

    struct VertexAttribute
    {
        std::string name;
        EFormat     format;
    };

    struct Rasterization
    {
        enum class ECullMode
        {
            FrontFace,
            BackFace,
            Discard
        };

        enum class EFillMode
        {
            Point,
            Triangle,
            Line
        };

        ECullMode cullMode = ECullMode::BackFace;
        EFillMode fillMode = EFillMode::Triangle;
    };

    struct DepthStencil
    {
        bool depthEnabled  = false;
        bool stencilEnable = false;
    }; // enable disable

    struct ColorBlend
    {
        // !TODO
    };

} // namespace PipelineStateDesc


struct PipelineLayoutDesc
{
    std::vector<ShaderResourceGroupLayout> shaderBindingGroupLayouts;
};

struct RenderTargetLayout
{
    std::vector<EFormat> colorFormats;
    EFormat              depthStencilFormat;
};

struct PipelineStateDescBase
{
    PipelineLayoutDesc                              pipelineLayoutDesc;
};

struct GraphicsPipelineStateDesc : PipelineStateDescBase
{
    RenderTargetLayout                              renderTargetLayout;
    PipelineStateDesc::GraphicsShaderStages         shaderStages;
    std::vector<PipelineStateDesc::VertexAttribute> vertexInputAttributes;
    PipelineStateDesc::Rasterization                rasterizationState;
    PipelineStateDesc::ColorBlend                   colorBlendState;
    PipelineStateDesc::DepthStencil                 depthStencil;
};

class IPipelineState
{
public:
    virtual ~IPipelineState() = default;
};

} // namespace RHI
