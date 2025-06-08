#pragma once

#include <TL/Block.hpp>

#include <RHI/RHI.hpp>

namespace Engine
{
    enum class MeshAttributeType;

    class ShaderModule
    {
    };

    // Will generate C++ struct with the correct layout to match slang shader
    class ShaderStructGenerator
    {
    public:

        enum class StructUsageType
        {
            ConstantBuffer,
            StructuredBuffer,
            Push,
        };

        TL::String GenerateCStruct(uint32_t resourceID);

        // Hot-reloading:
        // updateing shader struct without updating C++ code can cause
        //      1. CPP code compilation fail: Skip hot-reloading shader module if CPP code is not updated and print error message to console
        //      2. Missing resource bindings: Skip hot-reloading shader module if CPP code is not updated and print error message to console
        // issue: this apprach will require adding all shaders to cmake build script, meaning adding new shaders at runtime will not work.
        // solution: add another runtime struct-like e.g. ConstantBufferLayout::SetConstant('paramID', paramValue), StructuredBufferLayout::SetConstant()
    };

    class ShaderParser
    {
    public:
        // Parses mesh attribute types from the shader
        bool ParseMeshAttributes(TL::Vector<MeshAttributeType>& meshAttributes);

        // Parses shader bindings for a specific bind group
        bool ParseShaderBindings(uint32_t bindGroup, TL::Vector<RHI::ShaderBinding>& shaderBindings);

        // Parses rasterizer state description from the shader
        bool ParseRasterizerState(RHI::PipelineRasterizerStateDesc& outRasterState);

        // Parses depth-stencil state description from the shader
        bool ParseDepthStencilState(RHI::PipelineDepthStencilStateDesc& outDepthStencilState);

        // Parses color and depth-stencil formats from the shader
        bool ParseRenderTargetFormats(TL::Vector<RHI::Format>& outColorFormats, RHI::Format& outDepthStencilFormat);

        // Parses color attachment blend state descriptions from the shader
        bool ParseColorAttachmentBlendStates(TL::Vector<RHI::ColorAttachmentBlendStateDesc>& outAttachmentBlendStates);

        // Parses blend constants from the shader
        bool ParseBlendConstants(float*& outBlendConstants);

        bool ParseShader(RHI::GraphicsPipelineCreateInfo& finalizedAllocator, TL::IAllocator* scopeAllocator = TL::GetCurrentAllocator());
    };

    class ShaderLoader
    {
    public:
        ShaderModule* LoadShader(TL::Block code);
        void          ReloadShader(ShaderModule* shaderModule, TL::Block code);
        void          UnloadShader(ShaderModule* shaderModule);
    };
} // namespace Engine