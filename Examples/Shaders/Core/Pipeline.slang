#pragma once

namespace Shader::Core
{
    enum class PipelineVertexInputRate
    {
        None,
        PerInstance,
        PerVertex,
    };

    enum class PipelineRasterizerStateCullMode
    {
        None,
        FrontFace,
        BackFace,
        Discard,
    };

    enum class PipelineRasterizerStateFillMode
    {
        Point,
        Triangle,
        Line
    };

    enum class PipelineTopologyMode
    {
        Points,
        Lines,
        Triangles,
    };

    enum class PipelineRasterizerStateFrontFace
    {
        Clockwise,
        CounterClockwise,
    };

    struct PipelineInputAssemblerState
    {
    };

    struct PipelineRasterizerStateDesc
    {
        PipelineRasterizerStateCullMode  cullMode;
        PipelineRasterizerStateFillMode  fillMode;
        PipelineRasterizerStateFrontFace frontFace;
        float                            lineWidth;
    };

    struct GraphicsPipelineCreateInfo
    {
        // Slang::String                      vertexShaderName;
        // Slang::String                      pixelShaderName;
        // PipelineInputAssemblerStateDesc inputAssemblerState;
        // PipelineRenderTargetLayout      renderTargetLayout;
        // PipelineColorBlendStateDesc     colorBlendState;
        PipelineTopologyMode            topologyMode;
        PipelineRasterizerStateDesc     rasterizationState;
        // PipelineDepthStencilStateDesc   depthStencilState;
    };
}