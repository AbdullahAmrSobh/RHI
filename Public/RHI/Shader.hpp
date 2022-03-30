#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

struct ShaderModuleDesc
{
    ShaderModuleDesc(std::vector<std::byte> data, std::string_view entryPoint, EShaderStageFlagBits stage)
        : bytecode(std::move(data))
        , entryPointName(entryPoint)
        , stage(stage)
    {
    }

    std::vector<std::byte> bytecode;
    std::string            entryPointName;
    EShaderStageFlagBits           stage;
};

class IShaderModule
{
public:
    IShaderModule(std::string entryPointName, EShaderStageFlagBits stage)
        : m_entryPointName(std::move(entryPointName))
        , m_stage(stage)
    {
    }

    virtual ~IShaderModule() = default;

    inline const std::string& GetEntryPointName() const { return m_entryPointName; }
    inline EShaderStageFlagBits       GetStage() const { return m_stage; }

private:
    std::string  m_entryPointName;
    EShaderStageFlagBits m_stage;
};
using ShaderModulePtr = Unique<IShaderModule>;

struct GraphicsPipelineShaders
{
	GraphicsPipelineShaders(IShaderModule& vertexShader, IShaderModule& pixelShader)
		: pVertexShader(&vertexShader)
		, pPixelShader(&pixelShader)
	{}
    
    IShaderModule* pVertexShader   = nullptr;
    IShaderModule* pPixelShader    = nullptr;
    IShaderModule* pDomainShader   = nullptr;
    IShaderModule* pHullShader     = nullptr;
    IShaderModule* pGeometryShader = nullptr;
};

class ComputePipelineShader
{
    IShaderModule*                   pComputeShader = nullptr;
};

} // namespace RHI
