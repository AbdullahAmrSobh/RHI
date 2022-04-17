#pragma once
#include "RHI/Definitions.hpp"

namespace RHI {


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

struct VertexAttribute
{
    VertexAttribute() = default;

    std::string            name;
    EVertexAttributeFormat format;
};

struct PipelineInputAssemblyLayoutDesc
{
    PipelineInputAssemblyLayoutDesc() = default;

    std::vector<VertexAttribute> vertexAttributes;
};

struct PipelineRasterizationStateDesc
{
    PipelineRasterizationStateDesc() = default;

    ERasterizationCullMode cullMode = ERasterizationCullMode::BackFace;
    ERasterizationFillMode fillMode = ERasterizationFillMode::Triangle;
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
    PipelineStateColorBlendStateDesc() = default;
};

struct RenderTargetLayout
{
    RenderTargetLayout() = default;
    RenderTargetLayout(const std::vector<EPixelFormat>& colorFormats, EPixelFormat depthStencilFormat = EPixelFormat::Undefined)
        : colorFormats(colorFormats)
        , depthStencilFormat(depthStencilFormat)
    {
    }
    
    std::vector<EPixelFormat> colorFormats;
    EPixelFormat              depthStencilFormat;
};


} // namespace RHI
