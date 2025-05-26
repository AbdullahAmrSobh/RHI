#include "CullPass.hpp"
#include "GBufferPass.hpp"

#include "../PipelineLibrary.hpp"

namespace Engine
{
    ResultCode GBufferPass::Init(RHI::Device* device)
    {
        m_pipeline = PipelineLibrary::ptr->GetGraphicsPipeline(ShaderNames::GBufferFill);

        auto bindGroupLayout = PipelineLibrary::ptr->GetBindGroupLayout();
        m_bindGroup          = device->CreateBindGroup({.name = "GBuffer-BindGroup", .layout = bindGroupLayout});

        return ResultCode::Success;
    }

    void GBufferPass::Shutdown()
    {
    }

    void GBufferPass::AddPass(RHI::RenderGraph* rg, const CullPass& cullPass, TL::Function<void(RHI::CommandList&)> cb)
    {
        auto frameSize = rg->GetFrameSize();
        rg->AddPass({
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

                builder.ReadBuffer(cullPass.m_drawIndirectArgs, RHI::BufferUsage::Indirect, RHI::PipelineStage::DrawIndirect);
            },
            .executeCallback = [this, rg, cb](RHI::CommandList& cmd)
            {
                cmd.SetViewport({
                    .width    = (float)rg->GetFrameSize().width,
                    .height   = (float)rg->GetFrameSize().height,
                    .maxDepth = 1.0,
                });
                // Apply scissor/clipping rectangle
                RHI::Scissor scissor{
                    .offsetX = 0,
                    .offsetY = 0,
                    .width   = rg->GetFrameSize().width,
                    .height  = rg->GetFrameSize().height,
                };
                cmd.SetScissor(scissor);
                cb(cmd);
            },
        });
    }
} // namespace Engine