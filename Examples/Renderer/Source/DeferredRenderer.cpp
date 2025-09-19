#include "DeferredRenderer.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/PipelineLibrary.hpp"
#include "Renderer/Scene.hpp"

#include "Renderer-Shaders/GBufferPass.hpp"

#include <TL/Defer.hpp>

namespace Engine
{
    // GBufferFill pass
    void GBufferFill::init(RHI::Device* device)
    {
        RHI::BindGroupLayout* bindGroupLayout = sig::GBufferInputs::createBindGroupLayout(device);
        TL_defer
        {
            device->DestroyBindGroupLayout(bindGroupLayout);
        };

        m_bindGroup      = device->CreateBindGroup({.name = "bindgrou", .layout = bindGroupLayout});
        m_pipelineLayout = device->CreatePipelineLayout({.name = "GBufferInputs", .layouts = bindGroupLayout});

        auto vertexModule = PipelineLibrary::LoadShaderModule("I:/repos/repos3/RHI/build/Examples/Renderer/Shaders/GBufferPass.spirv.VSMain");
        auto pixelModule  = PipelineLibrary::LoadShaderModule("I:/repos/repos3/RHI/build/Examples/Renderer/Shaders/GBufferPass.spirv.PSMain");

        TL_defer
        {
            device->DestroyShaderModule(vertexModule);
            device->DestroyShaderModule(pixelModule);
        };

        // clang-format off
        RHI::GraphicsPipelineCreateInfo gfxPipelineCI = {
            .name                 = "ImGui Pipeline",
            .vertexShaderName     = "VSMain",
            .vertexShaderModule   = vertexModule,
            .pixelShaderName      = "PSMain",
            .pixelShaderModule    = pixelModule,
            .layout               = m_pipelineLayout,
            .vertexBufferBindings = {
                {sizeof(glm::vec3), RHI::PipelineVertexInputRate::PerVertex,   {{0, RHI::Format::RGB32_FLOAT}} }, // position
                {sizeof(glm::vec3), RHI::PipelineVertexInputRate::PerVertex,   {{0, RHI::Format::RGB32_FLOAT}} }, // normal
                {sizeof(glm::vec2), RHI::PipelineVertexInputRate::PerVertex,   {{0, RHI::Format::RG32_FLOAT}}  }, // texcoord
                {sizeof(glm::vec3), RHI::PipelineVertexInputRate::PerInstance, {{0, RHI::Format::RGBA32_FLOAT}}}, // draw-id
            },
            .renderTargetLayout =
            {
                .colorAttachmentsFormats = { RHI::Format::RGBA8_UNORM },
                .depthAttachmentFormat = RHI::Format::D16,
            },
            .colorBlendState =
            {
                .blendStates =
                {
                    {
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
            },
        };
        m_pipeline = device->CreateGraphicsPipeline(gfxPipelineCI);
        // clang-format on

        m_sampler      = device->CreateSampler({.name = "gbuffer-sampler"});
        // m_texture      = device->CreateImage({});
        m_shaderParams = {};
    }

    void GBufferFill::shutdown(RHI::Device* device)
    {
        // freeConstantBuffer({}, m_sceneView);
        // freeConstantBuffer({}, m_constantBuffer);
        device->DestroyImage(m_texture);
        device->DestroySampler(m_sampler);
        device->DestroyBindGroup(m_bindGroup);
        device->DestroyGraphicsPipeline(m_pipeline);
        device->DestroyPipelineLayout(m_pipelineLayout);
    }

    void GBufferFill::render(RHI::Device* device, RHI::RenderGraph* rg, const Scene& scene, RHI::ImageSize2D frameSize)
    {
        m_shaderParams.view           = scene.m_sceneView;
        m_shaderParams.defaultSampler = m_sampler;
        // m_shaderParams.simpleTexture  = m_texture;
        m_shaderParams.updateBindGroup(device, m_bindGroup);

        rg->AddPass({
            .name          = "main",
            .type          = RHI::PassType::Graphics,
            .size          = frameSize,
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                this->colorAttachment = builder.CreateColorTarget("color", frameSize, RHI::Format::RGBA8_UNORM);
            },
            .executeCallback = [=, this](RHI::CommandList& cmd)
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

                // // Bind index buffer
                // // cmd.BindIndexBuffer(GpuSceneData::ptr->m_indexBuffer, RHI::IndexType::uint32);
                // cmd.BindVertexBuffers(
                //     0,
                //     {
                //         GpuSceneData::ptr->m_vertexBufferPositions,
                //         GpuSceneData::ptr->m_vertexBufferNormals,
                //         GpuSceneData::ptr->m_vertexBufferTexcoords,
                //         GpuSceneData::ptr->m_vertexBufferDrawIDs,
                //     });

                // RHI::BufferBindingInfo argCountBuffer{rg->GetBufferHandle(cullPass.m_drawIndirectArgs), 0};
                // RHI::BufferBindingInfo argParamsBuffer{rg->GetBufferHandle(cullPass.m_drawIndirectArgs), 64};
                // cmd.DrawIndexedIndirect(argParamsBuffer, argCountBuffer, 40, sizeof(RHI::DrawIndexedParameters));

                cmd.Draw({.vertexCount = 3, .instanceCount = 1});

                // cmd.DrawIndexed({
                //     .indexCount    = mesh.m_drawArgs.indexCount,
                //     .instanceCount = 1,
                //     .firstIndex    = mesh.m_drawArgs.firstIndex,
                //     .vertexOffset  = mesh.m_drawArgs.vertexOffset,
                //     .firstInstance = 0,
                // });
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

        return TL::NoError;
    }

    void DeferredRenderer::shutdown(RHI::Device* device)
    {
        m_imguiPass.shutdown();
        m_gbufferPass.shutdown(device);
    }

    void DeferredRenderer::render(RHI::Device* device, RHI::RenderGraph* rg, const Scene* scene, RHI::RGImage* outputAttachment)
    {
        m_gbufferPass.render(device, rg, *scene, rg->GetFrameSize());
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