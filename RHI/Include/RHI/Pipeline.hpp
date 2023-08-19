#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "RHI/Export.hpp"
#include "RHI/Format.hpp"
#include "RHI/Object.hpp"
#include "RHI/Result.hpp"

namespace RHI
{

struct ShaderResourceGroupLayout;

class Context;
class ShaderFunction;
class PipelineStateCache;

enum class ShaderStages
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

enum class SamplerFilter
{
    Linear,
    Nearest
};

enum class SamplerAddressMode
{
    Repeat,
    Clamp
};

enum class SamplerCompareOp
{
    None,
    Never,
    Equal,
    NotEqual,
    Always,
    Less,
    LessEq,
    Greater,
    GreaterEq,
};

struct SamplerCreateInfo
{
    SamplerFilter      filter;
    SamplerCompareOp   compare;
    float              mipLodBias;
    SamplerAddressMode addressU;
    SamplerAddressMode addressV;
    SamplerAddressMode addressW;
    float              minLod;
    float              maxLod;
    float              maxAnisotropy;
};

struct GraphicsPipelineShaders
{
    const ShaderFunction* vertex;
    const ShaderFunction* tessControl;
    const ShaderFunction* tessEval;
    const ShaderFunction* pixel;
};

struct RayTracingPipelineShaders
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
    PipelineStateCache*                  pipelineStateCache;
    std::span<ShaderResourceGroupLayout> shadersLayout;
    GraphicsPipelineShaders              shaders;
    PipelineVertexInputLayout            vertexInputLayout;
    PipelineRasterizationState           rasterizationState;
    PipelineDepthStencilState            depthStencilState;
    PipelineMultisampleState             multisampleState;
    RenderTargetLayout                   renderTargetLayout;
};

struct ComputePipelineCreateInfo
{
    PipelineStateCache*                  pipelineStateCache;
    std::span<ShaderResourceGroupLayout> shadersLayout;
    const ShaderFunction*                shader;
};

struct RayTracingPipelineCreateInfo
{
    PipelineStateCache*                  pipelineStateCache;
    std::span<ShaderResourceGroupLayout> shadersLayout;
    RayTracingPipelineShaders            shaders;
    uint32_t                             maxRecursionDepth;
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
    const ShaderStage           m_shaderType;
    const std::string           m_name;
    const std::vector<uint32_t> m_code;
};

class RHI_EXPORT Sampler : public Object
{
public:
    using Object::Object;
    virtual ~Sampler() = default;
};

class RHI_EXPORT PipelineStateCache : public Object
{
public:
    using Object::Object;
    virtual ~PipelineStateCache() = default;
};

class RHI_EXPORT PipelineState : public Object
{
public:
    using Object::Object;
    virtual ~PipelineState() = default;
};

}  // namespace RHI
