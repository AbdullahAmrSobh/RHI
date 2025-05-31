#pragma once

#include <RHI/RHI.hpp>

#include "Common.hpp"

#include <slang/slang.h>
#include <slang/slang-com-helper.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang-gfx.h>

namespace Engine
{
    constexpr uint32_t BINDING_SCENEVIEW           = 0;
    constexpr uint32_t BINDING_DRAWREQUESTS        = 1;
    constexpr uint32_t BINDING_INDEXEDMESHES       = 2;
    constexpr uint32_t BINDING_TRANSFORMS          = 3;
    constexpr uint32_t BINDING_MATERIALS           = 4;
    constexpr uint32_t BINDING_DRAWPARAMETERSCOUNT = 5;
    constexpr uint32_t BINDING_OUTDRAWPARAMETERS   = 6;
    constexpr uint32_t BINDING_BINDLESSTEXTURES    = 7;

    namespace ShaderNames
    {
        inline constexpr const char* GBufferFill = "./Shaders/GBufferPass";
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

        RHI::Handle<RHI::BindGroupLayout> GetBindGroupLayout() const
        {
            return m_bindGroupLayout;
        }

        inline static PipelineLibrary* ptr = nullptr;

    private:
        RHI::Device*                      m_device;
        RHI::Handle<RHI::BindGroupLayout> m_bindGroupLayout;

        RHI::Handle<RHI::PipelineLayout> m_graphicsPipelineLayout;
        RHI::Handle<RHI::PipelineLayout> m_computePipelineLayout;

        TL::Map<TL::String, RHI::Handle<RHI::GraphicsPipeline>> m_graphicsPipelines;
        TL::Map<TL::String, RHI::Handle<RHI::ComputePipeline>>  m_computePipelines;
    };
} // namespace Engine