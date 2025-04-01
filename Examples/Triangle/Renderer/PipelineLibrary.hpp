#pragma once

#include <RHI/RHI.hpp>

#include "Common.hpp"

namespace Engine
{
    inline constexpr const char* kGBufferFill = "./Shaders/Basic";

    /// A class which holds all the pipelines that are used by the renderer
    /// Also handles hot reloading of the pipelines when the shader files change
    class PipelineLibrary
    {
    public:
        ResultCode Init(RHI::Device* device);
        void       Shutdown();

        RHI::Handle<RHI::GraphicsPipeline> GetGraphicsPipeline(const char* name);
        RHI::Handle<RHI::ComputePipeline>  GetComputePipeline(const char* name);

        RHI::Handle<RHI::BindGroupLayout> GetBindGroupLayout(const char* name, uint32_t binding);
        RHI::Handle<RHI::BindGroupLayout> GetBindGroupLayout(RHI::ShaderModule& shaderModule, uint32_t binding);

        void ReloadInvalidatedShaders();

    private:
        RHI::Device*                      m_device;
        RHI::Handle<RHI::BindGroupLayout> m_gBufferBGL;

        RHI::Handle<RHI::PipelineLayout> m_graphicsPipelineLayout;
        RHI::Handle<RHI::PipelineLayout> m_computePipelineLayout;

        TL::Map<TL::String, RHI::Handle<RHI::GraphicsPipeline>> m_graphicsPipelines;
        TL::Map<TL::String, RHI::Handle<RHI::ComputePipeline>>  m_computePipelines;
    };
} // namespace Engine