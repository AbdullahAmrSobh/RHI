#include "DeferredRenderer.hpp"

#include "Renderer/PipelineLibrary.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Scene.hpp"

#include "Renderer-Shaders/GBufferPass.hpp"

#include <TL/Defer.hpp>

namespace Engine
{
    // GBufferFill pass
    void GBufferFill::init(RHI::Device* device)
    {
        m_bindGroupLayout = sig::GBufferInputs::createBindGroupLayout(device);
        m_pipelineLayout  = device->CreatePipelineLayout({.name = "GBufferInputs", .layouts = m_bindGroupLayout});
        m_bindGroup       = device->CreateBindGroup({.name = "bindgrou", .layout = m_bindGroupLayout});

        // m_sampler      = device->CreateSampler({.name = "gbuffer-sampler"});
        // m_texture      = device->CreateImage({});
        m_shaderParams = {};

        // create PSOs

        PipelineLibrary::ptr->acquireGraphicsPipeline(
            "I:/repos/repos3/RHI/build/Examples/Renderer/Shaders/GBufferPass.spirv.VSMain",
            "I:/repos/repos3/RHI/build/Examples/Renderer/Shaders/GBufferPass.spirv.PSMain",
            [this](RHI::Device* device, RHI::ShaderModule* vs, RHI::ShaderModule* ps)
            {
                if (m_pipeline)
                {
                    device->DestroyGraphicsPipeline(m_pipeline);
                }

                RHI::GraphicsPipelineCreateInfo gfxPipelineCI =
                    {
                        .name               = "ImGui Pipeline",
                        .vertexShaderName   = "VSMain",
                        .vertexShaderModule = vs,
                        .pixelShaderName    = "PSMain",
                        .pixelShaderModule  = ps,
                        .layout             = m_pipelineLayout,
                        .vertexBufferBindings =
                            {
                                {sizeof(glm::vec3), RHI::PipelineVertexInputRate::PerVertex, {{0, RHI::Format::RGB32_FLOAT}}},
                                {sizeof(glm::vec3), RHI::PipelineVertexInputRate::PerVertex, {{0, RHI::Format::RGB32_FLOAT}}},
                                {sizeof(glm::vec2), RHI::PipelineVertexInputRate::PerVertex, {{0, RHI::Format::RG32_FLOAT}}},
                                // {sizeof(glm::ivec4), RHI::PipelineVertexInputRate::PerInstance, {{0, RHI::Format::RGBA32_UINT}}},
                            },
                        .renderTargetLayout =
                            {
                                .colorAttachmentsFormats = {RHI::Format::RGBA8_UNORM},
                                .depthAttachmentFormat   = RHI::Format::D16,
                            },
                        .colorBlendState =
                            {
                                .blendStates = RHI::ColorAttachmentBlendStateDesc{
                                    true,
                                    RHI::BlendEquation::Add,
                                    RHI::BlendFactor::SrcAlpha,
                                    RHI::BlendFactor::OneMinusSrcAlpha,
                                    RHI::BlendEquation::Add,
                                    RHI::BlendFactor::One,
                                    RHI::BlendFactor::OneMinusSrcAlpha,
                                    RHI::ColorWriteMask::All,
                                },
                            },
                    };
                m_pipeline = device->CreateGraphicsPipeline(gfxPipelineCI);
            });
    }

    void GBufferFill::shutdown(RHI::Device* device)
    {
        // freeConstantBuffer({}, m_sceneView);
        // freeConstantBuffer({}, m_constantBuffer);
        // device->DestroyImage(m_texture);
        // device->DestroySampler(m_sampler);
        device->DestroyGraphicsPipeline(m_pipeline);
        device->DestroyPipelineLayout(m_pipelineLayout);
        device->DestroyBindGroup(m_bindGroup);
        device->DestroyBindGroupLayout(m_bindGroupLayout);
    }

    void GBufferFill::render(RHI::Device* device, RHI::RenderGraph* rg, const Scene& scene, MeshVisibilityPass& visIn)
    {
        m_shaderParams.view           = scene.m_sceneView;
        m_shaderParams.defaultSampler = m_sampler;
        // m_shaderParams.simpleTexture  = m_texture;
        m_shaderParams.updateBindGroup(device, m_bindGroup);

        RHI::ImageSize2D frameSize = scene.m_imageSize;
        rg->AddPass({
            .name          = "main",
            .type          = RHI::PassType::Graphics,
            .size          = frameSize,
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                this->colorAttachment = builder.CreateColorTarget("color", frameSize, RHI::Format::RGBA8_UNORM);

                visIn.setup(builder);
            },
            .executeCallback = [=, this, &scene, &visIn](RHI::CommandList& cmd)
            {
                cmd.SetViewport(RHI::Viewport{
                    .width    = (float)frameSize.width,
                    .height   = (float)frameSize.height,
                    .maxDepth = 1.0,
                });
                cmd.SetScissor(RHI::Scissor{
                    .offsetX = 0,
                    .offsetY = 0,
                    .width   = frameSize.width,
                    .height  = frameSize.height,
                });

                cmd.BindGraphicsPipeline(
                    m_pipeline,
                    RHI::BindGroupBindingInfo{
                        .bindGroup      = m_bindGroup,
                        .dynamicOffsets = {},
                    });

                visIn.draw(rg, cmd);
            },
        });
    }

    // end

    TL::Error DeferredRenderer::init(RHI::Device* device)
    {
        if (auto err = m_imguiPass.init(device, RHI ::Format ::BGRA8_UNORM); err.IsError())
        {
            return err;
        }

        m_gbufferPass.init(device);
        m_vizabilityPass.init(device);

        return TL::NoError;
    }

    void DeferredRenderer::shutdown(RHI::Device* device)
    {
        m_imguiPass.shutdown();
        m_gbufferPass.shutdown(device);
        m_vizabilityPass.shutdown();
    }

    void DeferredRenderer::render(RHI::Device* device, RHI::RenderGraph* rg, const Scene* scene, RHI::RGImage* outputAttachment)
    {
        MeshVisibilityPassParams params{
            .name     = "Cull",
            .capacity = 4,
            .drawList = &scene->m_drawList
        };
        m_vizabilityPass.addPass(rg, params);
        m_gbufferPass.render(device, rg, *scene, m_vizabilityPass);

        rg->AddPass({
            .name          = "copy-to-output",
            .type          = RHI::PassType::Transfer,
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                builder.ReadImage(m_gbufferPass.colorAttachment, RHI::ImageUsage::CopySrc, RHI::PipelineStage::Copy);
                outputAttachment = builder.WriteImage(outputAttachment, RHI::ImageUsage::CopyDst, RHI::PipelineStage::Copy);
            },
            .executeCallback = [=, this](RHI::CommandList& cmd)
            {
                cmd.CopyImage({
                    .srcImage = rg->GetImageHandle(m_gbufferPass.colorAttachment),
                    .srcSize  = m_gbufferPass.colorAttachment->m_frameResource->size,
                    .dstImage = rg->GetImageHandle(outputAttachment),
                });
            },
        });

        if (m_imguiPass.enabled())
        {
            m_imguiPass.addPass(rg, outputAttachment, ImGui::GetDrawData());
            if (const auto& io = ImGui::GetIO(); io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::RenderPlatformWindowsDefault(nullptr, &m_imguiPass);
            }
        }
    }
} // namespace Engine