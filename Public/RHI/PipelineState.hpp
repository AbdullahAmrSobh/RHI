#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/PipelineLayout.hpp"
#include "RHI/Shader.hpp"

#include "RHI/PipelineStateDesc.hpp"

namespace RHI
{

struct GraphicsPipelineStateDesc
{
    GraphicsPipelineStateDesc() = default;
    GraphicsPipelineStateDesc(
		const RenderTargetLayout& renderTargetLayout, const IPipelineLayout& pipelineShadersLayout, GraphicsPipelineShaders shaders)
        : renderTargetLayout(renderTargetLayout)
        , pPipelineLayout(&pipelineShadersLayout)
        , shaders(shaders)
    {
    }

    const IPipelineLayout*            pPipelineLayout;
    GraphicsPipelineShaders           shaders;
    PipelineInputAssemblyLayoutDesc   inputAssemblyLayout;
    PipelineRasterizationStateDesc    rasterizationState;
    PipelineStateMultisampleStateDesc multisampleState;
    PipelineDepthStencilStateDesc     depthStencilState;
    PipelineStateColorBlendStateDesc  colorBlendState;
    RenderTargetLayout                renderTargetLayout;
};

struct ComputePipelineStateDesc
{
    ComputePipelineStateDesc(const IPipelineLayout& layout, ComputePipelineShader shader)
        : shader(shader)
    {
    }
    
    const IPipelineLayout* pPipelineLayout;
    ComputePipelineShader  shader;
};

class IPipelineState
{
public:
    virtual ~IPipelineState() = default;
};
using PipelineStatePtr = Unique<IPipelineState>;

} // namespace RHI
