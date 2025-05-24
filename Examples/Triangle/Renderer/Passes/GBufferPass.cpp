#include "GBufferPass.hpp"

#include "../Scene.hpp"

namespace Engine
{
    void Shader::Init(RHI::Device& device, const char* path)
    {
    }

    void GBufferPass::AddPass(RHI::RenderGraph* rg, const SceneView* view)
    {
        auto frameSize = rg.GetFrameSize();
        rg.AddPass({
            .name          = "GBuffer",
            .type          = RHI::PassType::Graphics,
            .size          = frameSize,
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                auto position = rg->CreateRenderTarget("gbuffer-position", frameSize, FormatPosition);
                auto normal   = rg->CreateRenderTarget("gbuffer-normal", frameSize, FormatNormal);
                auto material = rg->CreateRenderTarget("gbuffer-material", frameSize, FormatMaterial);
                auto depth    = rg->CreateRenderTarget("gbuffer-depth", frameSize, FormatDepth);

                m_position = builder.AddColorAttachment({.color = position, .clearValue = ClearPosition});
                m_normal   = builder.AddColorAttachment({.color = normal, .clearValue = ClearNormal});
                m_material = builder.AddColorAttachment({.color = material, .clearValue = ClearMaterial});
                m_depth    = builder.SetDepthStencil({.depthStencil = depth, .clearValue = ClearDepth});
            },
            .executeCallback = [this](RHI::CommandList& cmd)
            {
                cmd.BindGraphicsPipeline(m_shader->m_pipeline, {{m_shader->m_bindGroup[0]}});
            },
        });
    }
} // namespace Engine