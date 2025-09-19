#pragma once

#include <RHI/RHI.hpp>

#include "Renderer/Common.hpp"
#include "Renderer/ImGuiPass.hpp"

#include "Shaders/GpuCommonStructs.h"
#include "Renderer-Shaders/GBufferPass.hpp"

namespace Engine
{
    class Scene;

    struct GBufferFill
    {
    private:
        RHI::PipelineLayout*                      m_pipelineLayout = nullptr;
        RHI::GraphicsPipeline*                    m_pipeline       = nullptr;
        RHI::BindGroup*                           m_bindGroup      = nullptr;
        RHI::Sampler*                             m_sampler        = nullptr;
        RHI::Image*                               m_texture        = nullptr;
        ConstantBuffer<GPU::SceneGlobalConstants> m_constantBuffer = {};
        ConstantBuffer<GPU::SceneView>            m_sceneView      = {};
        sig::GBufferInputs                        m_shaderParams   = {};

    public:
        RHI::RGImage* colorAttachment = nullptr;

        void init(RHI::Device* device);
        void shutdown(RHI::Device* device);
        void render(RHI::Device* device, RHI::RenderGraph* rg, const Scene& scene, RHI::ImageSize2D size);
    };

    class DeferredRenderer : public Singleton<DeferredRenderer>
    {
    public:
        ImGuiPass   m_imguiPass;
        GBufferFill m_gbufferPass;

        TL::Error init(RHI::Device* device);
        void      shutdown(RHI::Device* device);
        void      render(RHI::Device* device, RHI::RenderGraph* rg, const Scene* scene, RHI::RGImage* outputAttachment);
    };
} // namespace Engine