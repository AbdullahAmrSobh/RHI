#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

struct ShaderModuleDesc
{
    ShaderModuleDesc() = default;
	
    std::vector<std::byte> bytecode;
    std::string            entryPointName;
    EShaderStageFlagBits   stage;
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
    
    inline const std::string& GetEntryPointName() const
    {
        return m_entryPointName;
    }
    inline EShaderStageFlagBits GetStage() const
    {
        return m_stage;
    }

private:
    const std::string          m_entryPointName;
    const EShaderStageFlagBits m_stage;
};
using ShaderModulePtr = Unique<IShaderModule>;

struct GraphicsPipelineShaders
{
    GraphicsPipelineShaders() = default;

    IShaderModule* pVertexShader   = nullptr;
    IShaderModule* pPixelShader    = nullptr;
    IShaderModule* pDomainShader   = nullptr;
    IShaderModule* pHullShader     = nullptr;
    IShaderModule* pGeometryShader = nullptr;
};

struct ComputePipelineShader
{
    ComputePipelineShader() = default;

    IShaderModule* pComputeShader = nullptr;
};

} // namespace RHI
