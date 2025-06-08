#include "CullPass.hpp"
#include "GBufferPass.hpp"

#include "../Geometry.hpp"
#include "../Scene.hpp"
#include "../PipelineLibrary.hpp"
#include "../Renderer.hpp"

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

    void GBufferPass::AddPass(RHI::RenderGraph* rg, const CullPass& cullPass, const Scene* scene)
    {
        auto frameSize = rg->GetFrameSize();
        rg->AddPass({
            .name          = "GBuffer",
            .type          = RHI::PassType::Graphics,
            .size          = frameSize,
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                auto position = rg->CreateRenderTarget("gbuffer-position", frameSize, GBufferPass::Formats[0]);
                auto normal   = rg->CreateRenderTarget("gbuffer-normal", frameSize, GBufferPass::Formats[1]);
                auto material = rg->CreateRenderTarget("gbuffer-material", frameSize, GBufferPass::Formats[2]);
                auto depth    = rg->CreateRenderTarget("gbuffer-depth", frameSize, GBufferPass::DepthFormat);

                m_attachments[0] = builder.AddColorAttachment({.color = position});
                m_attachments[1] = builder.AddColorAttachment({.color = normal});
                m_attachments[2] = builder.AddColorAttachment({.color = material});
                m_attachments[3] = builder.SetDepthStencil({.depthStencil = depth, .clearValue = {.depthValue = 1.0}});

                builder.ReadBuffer(cullPass.m_drawIndirectArgs, RHI::BufferUsage::Indirect, RHI::PipelineStage::DrawIndirect);
            },
            .executeCallback = [this, rg, cullPass, scene](RHI::CommandList& cmd)
            {
                cmd.SetViewport(RHI::Viewport{
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

                auto pipeline = PipelineLibrary::ptr->GetGraphicsPipeline(ShaderNames::GBufferFill);

                RHI::BindGroupBuffersUpdateInfo updateInfo[] = {
                    {BINDING_SCENEVIEW, 0, scene->m_primaryView->m_sceneViewUB.GetBinding()}
                };
                Renderer::ptr->m_device->UpdateBindGroup(m_bindGroup, {.buffers = updateInfo});
                cmd.BindGraphicsPipeline(pipeline, {{m_bindGroup}});

                // Bind index buffer
                cmd.BindIndexBuffer(GeometryBufferPool::ptr->GetAttribute(MeshAttributeType::Index), RHI::IndexType::uint32);
                cmd.BindVertexBuffers(
                    0,
                    {
                        GeometryBufferPool::ptr->GetAttribute(MeshAttributeType::Position),
                        GeometryBufferPool::ptr->GetAttribute(MeshAttributeType::Normal),
                        GeometryBufferPool::ptr->GetAttribute(MeshAttributeType::TexCoord),
                        GeometryBufferPool::ptr->GetAttribute(MeshAttributeType::TexCoord),
                    });

                RHI::BufferBindingInfo argCountBuffer{rg->GetBufferHandle(cullPass.m_drawIndirectArgs), 0};
                RHI::BufferBindingInfo argParamsBuffer{rg->GetBufferHandle(cullPass.m_drawIndirectArgs), 64};
                cmd.DrawIndexedIndirect(argParamsBuffer, argCountBuffer, 40, sizeof(RHI::DrawIndexedParameters));
            },
        });
    }
} // namespace Engine