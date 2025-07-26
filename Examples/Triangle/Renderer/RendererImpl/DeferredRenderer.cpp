#include "DeferredRenderer.hpp"

#include "../Geometry.hpp"
#include "../Scene.hpp"
#include "../PipelineLibrary.hpp"
#include "../Renderer.hpp"

namespace Engine
{
    ResultCode CullPass::Init(RHI::Device* device)
    {
        m_bindGroup = device->CreateBindGroup({.name = "Cull-BindGroup", .layout = PipelineLibrary::ptr->GetBindGroupLayout()});
        return ResultCode::Success;
    }

    void CullPass::Shutdown(RHI::Device* device)
    {
    }

    void CullPass::AddPass(RHI::Device* device, RHI::RenderGraph* rg, const Scene* scene)
    {
        this->m_drawIndirectArgs = rg->CreateBuffer("draw-indexed-indirect", kCapacity * sizeof(RHI::DrawIndexedParameters));

        rg->AddPass({
            .name          = "Cull",
            .type          = RHI::PassType::Compute,
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                this->m_drawIndirectArgs = builder.WriteBuffer(this->m_drawIndirectArgs, RHI::BufferUsage::Storage, RHI::PipelineStage::ComputeShader);
            },
            .executeCallback = [=](RHI::CommandList& cmd)
            {
                auto& meshDrawData = GeometryBufferPool::ptr->m_drawParams;

                RHI::BindGroupBuffersUpdateInfo updateInfo[] = {
                    {Bindings::DrawRequests,        0, scene->m_drawRequests.GetBindingInfo()                                   },
                    {Bindings::IndexedMeshes,       0, meshDrawData.GetBindingInfo()                                            },
                    {Bindings::DrawParametersCount, 0, RHI::BufferBindingInfo{rg->GetBufferHandle(this->m_drawIndirectArgs), 0} },
                    {Bindings::OutDrawParameters,   0, RHI::BufferBindingInfo{rg->GetBufferHandle(this->m_drawIndirectArgs), 64}},
                };
                device->UpdateBindGroup(m_bindGroup, {.buffers = updateInfo});

                auto pipeline = PipelineLibrary::ptr->GetComputePipeline(ShaderNames::Cull);

                cmd.BindComputePipeline(pipeline, {{m_bindGroup}});
                cmd.Dispatch({scene->m_drawRequests.GetCount(), 1, 1});
            },
        });
    }

    ResultCode GBufferPass::Init(RHI::Device* device)
    {
        m_bindGroup = device->CreateBindGroup({ "GBuffer-BindGroup", PipelineLibrary::ptr->GetBindGroupLayout()});
        return ResultCode::Success;
    }

    void GBufferPass::Shutdown(RHI::Device* device)
    {
        device->DestroyBindGroup(m_bindGroup);
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
                    {Bindings::SceneView, 0, scene->m_primaryView->m_sceneViewUB.GetBinding()}
                };
                Renderer::ptr->GetDevice()->UpdateBindGroup(m_bindGroup, {.buffers = updateInfo});
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

    ResultCode LightingPass::Init(RHI::Device* device)
    {
        m_bindGroup = device->CreateBindGroup({.name = "Lighting-BindGroup", .layout = PipelineLibrary::ptr->GetBindGroupLayout()});
        return ResultCode::Success;
    }

    void LightingPass::Shutdown(RHI::Device* device)
    {
        device->DestroyBindGroup(m_bindGroup);
    }

    void LightingPass::AddPass(RHI::RenderGraph* rg, const GBufferPass& gbuffer, const class Scene* scene)
    {
        rg->AddPass({
            .name          = "Lighting-Pass",
            .type          = RHI::PassType::Compute,
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                builder.ReadImage(gbuffer.m_attachments[0], RHI::ImageUsage::ShaderResource, RHI::PipelineStage::ComputeShader);
                builder.ReadImage(gbuffer.m_attachments[1], RHI::ImageUsage::ShaderResource, RHI::PipelineStage::ComputeShader);
                builder.ReadImage(gbuffer.m_attachments[2], RHI::ImageUsage::ShaderResource, RHI::PipelineStage::ComputeShader);
                builder.ReadImage(gbuffer.m_attachments[3], RHI::ImageUsage::ShaderResource, RHI::PipelineStage::ComputeShader);

                auto size    = rg->GetFrameSize();
                m_attachment = rg->CreateImage("lighting", RHI::ImageType::Image2D, {size.width, size.height}, RHI::Format::RGBA8_UNORM);
                m_attachment = builder.WriteImage(m_attachment, RHI::ImageUsage::StorageResource, RHI::PipelineStage::ComputeShader);
            },
            .executeCallback = [this, rg, gbuffer, scene](RHI::CommandList& cmd)
            {
                // update bind groups
                RHI::BindGroupImagesUpdateInfo updateInfo[] = {
                    {Bindings::gBuffer_wsPosition, 0, rg->GetImageHandle(gbuffer.m_attachments[0])},
                    {Bindings::gBuffer_normal,     0, rg->GetImageHandle(gbuffer.m_attachments[1])},
                    {Bindings::gBuffer_material,   0, rg->GetImageHandle(gbuffer.m_attachments[2])},
                    {Bindings::gBuffer_depth,      0, rg->GetImageHandle(gbuffer.m_attachments[3])},
                    {Bindings::compose_output,     0, rg->GetImageHandle(m_attachment)            },
                };
                Renderer::ptr->GetDevice()->UpdateBindGroup(m_bindGroup, {.images = updateInfo});

                // bind pipeline and bind group states
                auto pipeline = PipelineLibrary::ptr->GetComputePipeline(ShaderNames::Lighting);
                cmd.BindComputePipeline(pipeline, {{m_bindGroup}});

                // dispatch
                static constexpr RHI::ImageSize3D workgroupSize{32, 32, 1};

                auto frameSize = rg->GetFrameSize();

                RHI::DispatchParameters params{
                    .countX = (uint32_t)std::ceil((float)frameSize.width / (float)workgroupSize.width),
                    .countY = (uint32_t)std::ceil((float)frameSize.height / (float)workgroupSize.height),
                    .countZ = 1,
                };
                cmd.Dispatch(params);
            },
        });
    }

    ResultCode ComposePass::Init(RHI::Device* device)
    {
        m_bindGroup = device->CreateBindGroup({.name = "Lighting-BindGroup", .layout = PipelineLibrary::ptr->GetBindGroupLayout()});
        return ResultCode::Success;
    }

    void ComposePass::Shutdown(RHI::Device* device)
    {
        device->DestroyBindGroup(m_bindGroup);
    }

    void ComposePass::AddPass(RHI::RenderGraph* rg, RHI::RGImage* input, RHI::RGImage*& output)
    {
        rg->AddPass({
            .name          = "Compose-Pass",
            .type          = RHI::PassType::Graphics,
            .size          = rg->GetFrameSize(),
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                builder.ReadImage(input, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::ComputeShader);

                output = builder.AddColorAttachment({.color = output, .loadOp = RHI::LoadOperation::Discard});
            },
            .executeCallback = [this, rg, input, output](RHI::CommandList& cmd)
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

                // update bind groups
                RHI::BindGroupImagesUpdateInfo updateInfo[] = {
                    {Bindings::lighting_input, 0, rg->GetImageHandle(input)},
                };
                Renderer::ptr->GetDevice()->UpdateBindGroup(m_bindGroup, {.images = updateInfo});

                // bind pipeline and bind group states
                auto pipeline = PipelineLibrary::ptr->GetGraphicsPipeline(ShaderNames::Compose);
                cmd.BindGraphicsPipeline(pipeline, {{m_bindGroup}});

                cmd.Draw({6, 1});
            },
        });
    }

    ///

    ResultCode DeferredRenderer::Init(RHI::Device* device)
    {
#define TRY(expr)                                          \
    {                                                      \
        auto result = (expr);                              \
        if (result != ResultCode::Success)                 \
        {                                                  \
            TL_LOG_ERROR("DeferredRenderer::Init failed"); \
            this->Shutdown(device);                        \
            return result;                                 \
        }                                                  \
    }
        TRY(m_cullPass.Init(device));
        TRY(m_gbufferPass.Init(device));
        TRY(m_lightingPass.Init(device));
        TRY(m_composePass.Init(device));
        TRY(m_imguiPass.Init(device, RHI::Format::RGBA8_UNORM));

#undef TRY
        return ResultCode::Success;
    }

    void DeferredRenderer::Shutdown(RHI::Device* device)
    {
        m_imguiPass.Shutdown();
        m_composePass.Shutdown(device);
        m_lightingPass.Shutdown(device);
        m_gbufferPass.Shutdown(device);
        m_cullPass.Shutdown(device);
    }

    void DeferredRenderer::Render(RHI::Device* device, RHI::RenderGraph* rg, const Scene* scene, RHI::RGImage* outputAttachment)
    {
        // TODO: Hot reloading this section
        m_cullPass.AddPass(device, rg, scene);
        m_gbufferPass.AddPass(rg, m_cullPass, scene);
        m_lightingPass.AddPass(rg, m_gbufferPass, scene);
        // Needs to be graphics because swapchain
        m_composePass.AddPass(rg, m_lightingPass.m_attachment, outputAttachment);
        if (m_imguiPass.Enabled())
        {
            m_imguiPass.AddPass(rg, outputAttachment, ImGui::GetDrawData());
            if (const auto& io = ImGui::GetIO(); io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::RenderPlatformWindowsDefault(nullptr, &m_imguiPass);
            }
        }
    }

} // namespace Engine