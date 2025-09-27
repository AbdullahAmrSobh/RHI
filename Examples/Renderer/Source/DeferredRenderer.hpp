#pragma once

#include <RHI/RHI.hpp>

#include "Renderer/Common.hpp"
#include "Renderer/ImGuiPass.hpp"
#include "Renderer/MeshDrawProcessor.hpp"

#include "Renderer-Shaders/GBufferPass.hpp"
#include "Renderer-Shaders/Compose.hpp"

#include "Shaders/GpuCommonStructs.h"

namespace Engine
{
    class Scene;

    struct GBufferFill
    {
    private:
        RHI::BindGroupLayout*             m_bindGroupLayout = nullptr;
        RHI::PipelineLayout*              m_pipelineLayout  = nullptr;
        RHI::GraphicsPipeline*            m_pipeline        = nullptr;
        RHI::BindGroup*                   m_bindGroup       = nullptr;
        RHI::Sampler*                     m_sampler         = nullptr;
        RHI::Image*                       m_texture         = nullptr;
        Buffer<GPU::SceneGlobalConstants> m_constantBuffer  = {};
        Buffer<GPU::SceneView>            m_sceneView       = {};
        GPU::GBufferInputs                m_shaderParams    = {};

    public:
        RHI::RGImage* colorAttachment = nullptr;

        void init(RHI::Device* device);
        void shutdown(RHI::Device* device);
        void render(RHI::Device* device, RHI::RenderGraph* rg, const Scene& scene, MeshVisibilityPass& visIn);
    };

    class DeferredRenderer : public Singleton<DeferredRenderer>
    {
    public:
        ImGuiPass m_imguiPass;

        MeshVisibilityPass m_vizabilityPass;
        GBufferFill        m_gbufferPass;

        TL::Error init(RHI::Device* device);
        void      shutdown(RHI::Device* device);
        void      render(RHI::Device* device, RHI::RenderGraph* rg, const Scene* scene, RHI::RGImage* outputAttachment);
    };
} // namespace Engine