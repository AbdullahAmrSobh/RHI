#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "RHI/Export.hpp"
#include "RHI/Format.hpp"
#include "RHI/Result.hpp"

// #include "RHI/ShaderResourceGroup.hpp"

namespace RHI
{

struct ShaderResourceGroupLayout;

class Context;
class ShaderFunction;

enum class ShaderStage
{
    None                   = 0 << 1,
    Vertex                 = 1 << 2,
    TessellationControl    = 1 << 3,
    TessellationEvaluation = 1 << 4,
    Pixel                  = 1 << 5,
    Compute                = 1 << 6,
};

enum class PolygonMode
{
    Fill,
    Line,
    Point
};

enum class CullMode
{
    None,
    Back,
    Front,
    Both
};

enum class FrontFace
{
    CounterClockwise,
    Clockwise
};

struct GraphicsPipelineShaderTypes
{
    const ShaderFunction* vertex;
    const ShaderFunction* tessControl;
    const ShaderFunction* tessEval;
    const ShaderFunction* pixel;
};

struct RayTracingPipelineShaderTypes
{
    const ShaderFunction* rayGen;
    const ShaderFunction* rayIntersection;
    const ShaderFunction* anyHit;
    const ShaderFunction* closetHit;
};

struct PipelineVertexInputLayout
{
    std::vector<Format> vertexAttributes;
    std::vector<Format> instanceAttributes;
};

struct PipelineRasterizationState
{
    bool        depthClampEnable;
    bool        rasterizerDiscardEnable;
    PolygonMode polygonMode;
    float       lineWidth;
    CullMode    cullMode;
    FrontFace   frontFace;
};

struct PipelineDepthStencilState
{
    bool  depthEnable;
    bool  stencilEnable;
    float minDepthBound;
    float maxDepthBound;
};

struct PipelineMultisampleState
{
    uint32_t sampleCount;
};

struct RenderTargetLayout
{
    std::vector<RHI::Format> colorAttachments;
    RHI::Format              depthStencilAttachment;
};

struct GraphicsPipelineCreateInfo
{
    GraphicsPipelineShaderTypes            shaders;
    std::vector<ShaderResourceGroupLayout> shadersLayout;
    PipelineVertexInputLayout              vertexInputLayout;
    PipelineRasterizationState             rasterizationState;
    PipelineDepthStencilState              depthStencilState;
    PipelineMultisampleState               multisampleState;
    RenderTargetLayout                     renderTargetLayout;
};

class RHI_EXPORT ShaderFunction
{
public:
    ShaderFunction(ShaderStage functionType, std::string name, std::vector<uint32_t> code)
        : m_shaderType(functionType)
        , m_name(std::move(name))
        , m_code(std::move(code))
    {
    }

    RHI_SUPPRESS_C4251
    const ShaderStage            m_shaderType;
    const std::string           m_name;
    const std::vector<uint32_t> m_code;
};

class RHI_EXPORT PipelineState
{
public:
    PipelineState(Context& context)
        : m_context(&context)
    {
    }

    virtual ~PipelineState() = default;

    virtual ResultCode Init(const GraphicsPipelineCreateInfo& createInfo) = 0;

protected:
    Context* m_context;
};

}  // namespace RHI
