#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "RHI/Export.hpp"
#include "RHI/Format.hpp"
#include "RHI/Result.hpp"

namespace RHI
{

class Context;

class ShaderFunction;
struct ShaderResourceGroupLayout;

enum class ShaderType
{
    None                   = 0 << 1,
    Vertex                 = 1 << 2,
    TessellationControl    = 1 << 3,
    TessellationEvaluation = 1 << 4,
    Pixel                  = 1 << 5,
    RayGen                 = 1 << 6,
    RayIntersection        = 1 << 7,
    AnyHit                 = 1 << 8,
    ClosetHit              = 1 << 9,
    MeshAmplification      = 1 << 10,
    MeshTask               = 1 << 11,
    Mesh                   = 1 << 12,
    Compute                = 1 << 13,
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
    const GraphicsPipelineShaderTypes      shaders;
    std::vector<ShaderResourceGroupLayout> shadersLayout;
    PipelineVertexInputLayout              vertexInputLayout;
    PipelineRasterizationState             rasterizationState;
    PipelineDepthStencilState              depthStencilState;
    PipelineMultisampleState               multisampleState;
    RenderTargetLayout                     renderTargetLayout;
};

struct ComputePipelineCreateInfo
{
    std::vector<ShaderResourceGroupLayout> shaderLayout;
    const ShaderFunction*                  computeShader;
};

struct RayTracingPipelineCreateInfo
{
    RayTracingPipelineShaderTypes          shaders;
    std::vector<ShaderResourceGroupLayout> shadersLayout;
    uint32_t                               maxRayRecursionDepth;
};

size_t HashCombineShaderResourceGroups(const std::span<const ShaderResourceGroupLayout> layouts)
{
    return 0;
}

class RHI_EXPORT ShaderFunction
{
public:
    ShaderFunction(ShaderType functionType, std::vector<uint32_t> code);
    virtual ~ShaderFunction() = default;

    const ShaderType            m_shaderType;
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

    virtual ResultCode Init(const GraphicsPipelineCreateInfo& createInfo)   = 0;
    virtual ResultCode Init(const ComputePipelineCreateInfo& createInfo)    = 0;
    virtual ResultCode Init(const RayTracingPipelineCreateInfo& createInfo) = 0;

protected:
    Context* m_context;
};

}  // namespace RHI
