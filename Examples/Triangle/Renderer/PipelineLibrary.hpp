#pragma once

#include <RHI/RHI.hpp>

#include "Common.hpp"

namespace Engine
{
    namespace ShaderNames
    {
        inline constexpr const char* GBufferFill = "./Shaders/Basic";
        inline constexpr const char* Cull        = "./Shaders/Cull";
    }; // namespace ShaderNames

    /// A class which holds all the pipelines that are used by the renderer
    /// Also handles hot reloading of the pipelines when the shader files change
    class PipelineLibrary
    {
    public:
        ResultCode Init(RHI::Device* device);
        void       Shutdown();

        RHI::Handle<RHI::GraphicsPipeline> GetGraphicsPipeline(const char* name);
        RHI::Handle<RHI::ComputePipeline>  GetComputePipeline(const char* name);

        RHI::Handle<RHI::BindGroupLayout> GetBindGroupLayout(const char* name, uint32_t binding)
        {
            return m_computeBGL;
        }

        RHI::Handle<RHI::BindGroupLayout> GetBindGroupLayout(RHI::ShaderModule& shaderModule, uint32_t binding)
        {
            return m_computeBGL;
        }

        void ReloadInvalidatedShaders();

    private:
        RHI::Device*                      m_device;
        RHI::Handle<RHI::BindGroupLayout> m_gBufferBGL;
        RHI::Handle<RHI::BindGroupLayout> m_computeBGL;

        RHI::Handle<RHI::PipelineLayout> m_graphicsPipelineLayout;
        RHI::Handle<RHI::PipelineLayout> m_computePipelineLayout;

        TL::Map<TL::String, RHI::Handle<RHI::GraphicsPipeline>> m_graphicsPipelines;
        TL::Map<TL::String, RHI::Handle<RHI::ComputePipeline>>  m_computePipelines;
    };
} // namespace Engine