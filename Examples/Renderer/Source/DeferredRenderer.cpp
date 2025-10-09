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
        // m_bindGroupLayout = GPU::GBufferInputs::createBindGroupLayout(device);

        auto layout = ShaderBindGroup<GPU::GBufferInputs>::getLayout()->get();

        m_shader = PipelineLibrary::ptr->acquireGraphicsPipeline<GPU::GBufferInputs>(
            "I:/repos/repos3/RHI/Examples/Renderer/Shaders/source/GBufferPass.json",
            {
                {sizeof(glm::vec3), RHI::PipelineVertexInputRate::PerVertex, {{0, RHI::Format::RGB32_FLOAT}}},
                {sizeof(glm::vec3), RHI::PipelineVertexInputRate::PerVertex, {{0, RHI::Format::RGB32_FLOAT}}},
                {sizeof(glm::vec2), RHI::PipelineVertexInputRate::PerVertex, {{0, RHI::Format::RG32_FLOAT}}},
                // {sizeof(glm::ivec4), RHI::PipelineVertexInputRate::PerInstance, {{0, RHI::Format::RGBA32_UINT}}},
            },
            {
                .colorAttachmentsFormats = {RHI::Format::RGBA8_UNORM},
                .depthAttachmentFormat   = RHI::Format::D16,
            });

        m_shaderParams.init(device, 0);
    }

    void GBufferFill::shutdown(RHI::Device* device)
    {
        // freeConstantBuffer({}, m_sceneView);
        // freeConstantBuffer({}, m_constantBuffer);
        // device->DestroyImage(m_texture);
        // device->DestroySampler(m_sampler);
        m_shaderParams.shutdown(device);
    }

    void GBufferFill::render(RHI::Device* device, RHI::RenderGraph* rg, const Scene& scene, MeshVisibilityPass& visIn)
    {
        m_shaderParams.view           = scene.m_sceneView;
        m_shaderParams.defaultSampler = m_sampler;
        // m_shaderParams.simpleTexture  = m_texture;
        m_shaderParams.update(device);

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

                cmd.BindGraphicsPipeline(m_shader->getPipeline(), m_shaderParams.bind());

                visIn.draw(rg, cmd, scene.m_drawList.getCount());
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
            .drawList = &scene->m_drawList};
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