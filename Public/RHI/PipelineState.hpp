#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

class IPipelineLayout;


// Todo move to Definitions.hpp
enum class EShaderStage
{
    Vertex,
    Hull,
    Domain,
    Geometry,
    Pixel,
    Compute,
    MaxEnum,
};

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

struct ShaderBytecodeDesc
{
	ShaderBytecodeDesc(std::vector<std::byte> data, std::string_view entryPoint, EShaderStage stage)
		: bytecode(std::move(data))
		, entryPointName(entryPoint)
		, stage(stage)
	{}
	
    std::vector<std::byte> bytecode;
    std::string          entryPointName;
    EShaderStage         stage;
};

class IShaderBytecode
{
public:
    IShaderBytecode(std::string entryPointName, EShaderStage stage)
        : m_entryPointName(std::move(entryPointName))
        , m_stage(stage)
    {
    }

    virtual ~IShaderBytecode() = default;

    inline const std::string& GetEntryPointName() const { return m_entryPointName; }
    inline EShaderStage       GetStage() const { return m_stage; }

private:
    std::string  m_entryPointName;
    EShaderStage m_stage;
};
using ShaderBytecodePtr = Unique<IShaderBytecode>;

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

struct PipelineRenderTargetLayoutDesc
{
    uint32_t     renderTargetColorAttachmentCount;
    EPixelFormat formats[8];
    EPixelFormat depthFormat;
};

struct GraphicsPipelineStateDesc
{
    IShaderBytecode* pVertexShader   = nullptr;
    IShaderBytecode* pPixelShader    = nullptr;
    IShaderBytecode* pDomainShader   = nullptr;
    IShaderBytecode* pHullShader     = nullptr;
    IShaderBytecode* pGeometryShader = nullptr;

    std::vector<EVertexAttributeFormat> vertexAttributes;
    PipelineRasterizationStateDesc      rasterizationState;
    PipelineStateMultisampleStateDesc   multisampleState;
    PipelineDepthStencilStateDesc       depthStencilState;
    PipelineStateColorBlendStateDesc    colorBlendState;
    PipelineRenderTargetLayoutDesc      renderTargetLayout;

    IPipelineLayout* pPipelineLayout;
};

struct ComputePipelineStateDesc
{
    IShaderBytecode* pGeometryShader = nullptr;
    IPipelineLayout* pPipelineLayout;
};

class IPipelineState
{
public:
    virtual ~IPipelineState() = default;
};
using PipelineStatePtr = Unique<IPipelineState>;

} // namespace RHI
