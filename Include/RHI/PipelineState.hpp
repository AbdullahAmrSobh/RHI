#pragma once
#include <string>
#include <vector>
#include "RHI/Format.hpp"

namespace RHI
{
class IShaderProgram;

struct PipelineLayoutDesc
{
    std::vector<DescriptorSetLayout> descriptorSetLayouts;
};

class IPipelineLayout
{
public:
    virtual ~IPipelineLayout() = default;
};

struct RenderTargetLayout
{
    std::vector<EFormat> colorFormats;
    EFormat              depthStencilFormat;
};

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

struct GraphicsPipelineStateDesc
{
    RenderTargetLayout                              renderTargetLayout;
    PipelineStateDesc::GraphicsShaderStages         shaderStages;
    std::vector<PipelineStateDesc::VertexAttribute> attributes;
    PipelineStateDesc::Rasterization                rasterizationState;
    PipelineStateDesc::ColorBlend                   colorBlendState;
    PipelineStateDesc::DepthStencil                 DepthStencil;
};

class IPipelineState
{
public:
    virtual ~IPipelineState() = default;
};

} // namespace RHI
