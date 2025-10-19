#pragma once

#include <RHI/RHI.hpp>

#include "Renderer/Common.hpp"
#include "Renderer/ImGuiPass.hpp"
#include "Renderer/DrawList.hpp"
#include "Renderer/PipelineLibrary.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Scene.hpp"

#include "Renderer/Resources.hpp"

#include "Renderer-Shaders/Cull.hpp"
#include "Renderer-Shaders/GBufferPass.hpp"

#include "Shaders/GpuCommonStructs.h"

#include <TL/Defer.hpp>

namespace Engine
{
    static constexpr const char* kCullShaderPath     = "Renderer/Shaders/source/Cull.json";
    static constexpr const char* kGBufferShaderPath  = "Renderer/Shaders/source/GBufferPass.json";
    static constexpr const char* kLightingShaderPath = "Renderer/Shaders/source/Lighting.json";

    static void dispatchPP2D(RHI::CommandList& cmd, RHI::ImageSize2D workgroupSize, RHI::ImageSize2D imageSize)
    {
        const uint32_t sizeX = (imageSize.width + workgroupSize.width - 1) / workgroupSize.width;
        const uint32_t sizeY = (imageSize.height + workgroupSize.height - 1) / workgroupSize.height;
        cmd.Dispatch({sizeX, sizeY, 1});
    }

    static void dispatchPP3D(RHI::CommandList& cmd, RHI::ImageSize3D workgroupSize, RHI::ImageSize3D imageSize)
    {
        const uint32_t sizeX = (imageSize.width + workgroupSize.width - 1) / workgroupSize.width;
        const uint32_t sizeY = (imageSize.height + workgroupSize.height - 1) / workgroupSize.height;
        const uint32_t sizeZ = (imageSize.depth + workgroupSize.depth - 1) / workgroupSize.depth;
        cmd.Dispatch({sizeX, sizeY, sizeZ});
    }

    class MeshVisibilityPass
    {
    public:
        TL::Error init(RHI::Device* device)
        {
            m_device = device;

            m_shaderParams.init(device, 0);

            m_shader = PipelineLibrary::ptr->acquireComputePipeline<GPU::CullParams>(kCullShaderPath);

            auto& pool  = RenderContext::ptr->getConstantBuffersPool();
            m_constants = pool.allocate<GPU::CullParams::CB>();

            return TL::NoError;
        }

        void shutdown()
        {
            // m_device->DestroyComputePipeline(m_pipeline);
            // m_device->DestroyPipelineLayout(m_pipelineLayout);
            m_shaderParams.shutdown(m_device);
        }

        RHI::RGPass* addPass(RHI::RenderGraph* rg, TL::StringView name, const DrawList* drawList)
        {
            GPU::CullParams::CB cb{
                .drawCount = drawList->getCount(),
            };
            auto& pool = RenderContext::ptr->getConstantBuffersPool();
            pool.update(m_constants, cb);

            auto* pass = rg->addPass(name.data(), RHI::RGPassType::Compute, {});
            // Setup: translate builder calls to pass methods

            auto argsBufferSize = TL::AlignUp<uint32_t>(sizeof(uint32_t), m_device->GetLimits().minStorageBufferOffsetAlignment) + (sizeof(RHI::DrawIndexedParameters) * drawList->getCapacity());
            m_drawIndirectArgs  = pass->createBuffer({"mdi-args", drawList->getCapacity() * sizeof(RHI::DrawIndexedParameters) + argsBufferSize}, RHI::RGBufferUsage::UavCompute);

            // Execute
            rg->submitPass(pass, [this, drawList, rg](RHI::CommandList& cmd)
                {
                    m_shaderParams.cb                = this->m_constants;
                    m_shaderParams.drawRequests      = drawList->getDrawRequests();
                    m_shaderParams.staticMeshOffsets = RenderContext::ptr->getSBPoolRenderables();
                    m_shaderParams.drawCountOut      = getCountBuffer(rg);
                    m_shaderParams.drawIndirectArgs  = getArgBuffer(rg);
                    m_shaderParams.update(m_device);

                    cmd.BindComputePipeline(m_shader->getPipeline(), m_shaderParams.bind());

                    const uint32_t groupSize = 64; // must match [numthreads(x,y,z)] in shader
                    uint32_t       numGroups = (groupSize) / groupSize;
                    cmd.Dispatch({groupSize, 1, 1});
                });
            return pass;
        }

        RHI::BufferBindingInfo getCountBuffer(RHI::RenderGraph* rg)
        {
            return {
                .buffer = rg->GetBufferHandle(m_drawIndirectArgs),
                .offset = 0,
                .range  = sizeof(uint32_t),
            };
        }

        RHI::BufferBindingInfo getArgBuffer(RHI::RenderGraph* rg)
        {
            auto bindingInfo   = getCountBuffer(rg);
            bindingInfo.offset = m_device->GetLimits().minStorageBufferOffsetAlignment;
            bindingInfo.range  = RHI::RemainingSize;
            return bindingInfo;
        }

        void setup(RHI::RGPass* pass)
        {
            pass->readBuffer(m_drawIndirectArgs, RHI::RGBufferUsage::IndirectDraw);
        }

        void draw(RHI::RenderGraph* rg, RHI::CommandList& cmd, uint32_t maxDrawCount)
        {
            cmd.BindIndexBuffer(RenderContext::ptr->getIndexPool(), RHI::IndexType::uint32);
            cmd.BindVertexBuffers(
                0,
                {
                    RenderContext::ptr->getVertexPoolPositions(),
                    RenderContext::ptr->getVertexPoolNormals(),
                    RenderContext::ptr->getVertexPoolUVs(),
                    // {rg->GetBufferHandle(m_instanceBuffer), 0},
                });
            cmd.DrawIndexedIndirect(
                getArgBuffer(rg),
                getCountBuffer(rg),
                maxDrawCount,
                sizeof(RHI::DrawIndexedParameters));
        }

    private:
        RHI::Device* m_device;

        RHI::RGBuffer* m_drawIndirectArgs = nullptr;

        TL::Ptr<ComputeShader>           m_shader;
        Buffer<GPU::CullParams::CB>      m_constants;
        ShaderBindGroup<GPU::CullParams> m_shaderParams;
    };

    struct GBufferFill
    {
    private:
        TL::Ptr<GraphicsShader> m_shader = nullptr;
        Buffer<GPU::SceneGlobalConstants> m_constantBuffer = {};
        Buffer<GPU::SceneView>            m_sceneView      = {};
        ShaderBindGroup<GPU::GBufferInputs> m_shaderParams;

    public:
        RHI::RGImage* colorAttachment      = nullptr;
        RHI::RGImage* wsPositionAttachment = nullptr;
        RHI::RGImage* normalAttachment     = nullptr;
        RHI::RGImage* depthAttachment      = nullptr;

        void init()
        {
            m_shader = PipelineLibrary::ptr->acquireGraphicsPipeline<GPU::GBufferInputs>(
                kGBufferShaderPath,
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


            m_constantBuffer = RenderContext::ptr->allocateConstantBuffer<GPU::SceneGlobalConstants>();

            m_shaderParams.init(RenderContext::ptr->m_device, 0);
        }

        void shutdown()
        {
            // Intentionally left commented as in original implementation.
            // freeConstantBuffer({}, m_sceneView);
            // freeConstantBuffer({}, m_constantBuffer);
            // device->DestroyImage(m_texture);
            // device->DestroySampler(m_sampler);
            // m_shaderParams.shutdown(device);
        }

        void render(RHI::RenderGraph* rg, const Scene& scene, MeshVisibilityPass& visIn)
        {
            auto* renderCtx = RenderContext::ptr;

            m_shaderParams.view = scene.m_sceneView;
            m_shaderParams.update(renderCtx->m_device);

            RHI::ImageSize2D frameSize = scene.m_imageSize;
            auto pass = rg->addPass("GBufferFill", RHI::RGPassType::Graphics, frameSize);
            this->colorAttachment      = pass->createRenderTarget({"color", RHI::Format::RGBA8_UNORM});
            visIn.setup(pass);
            rg->submitPass(pass, [this, &scene, &visIn, rg, frameSize](RHI::CommandList& cmd)
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
                });
        }
    };

    class DeferredRenderer : public Singleton<DeferredRenderer>
    {
    public:
        ImGuiPass m_imguiPass;

        MeshVisibilityPass m_vizabilityPass;
        GBufferFill        m_gbufferPass;

        TL::Error init()
        {
            auto* renderCtx = RenderContext::ptr;
            auto* device    = renderCtx->m_device;

            if (auto err = m_imguiPass.init(device, RHI::Format::BGRA8_UNORM); err.IsError())
            {
                return err;
            }

            m_gbufferPass.init();
            m_vizabilityPass.init(device);

            return TL::NoError;
        }

        void shutdown()
        {
            m_imguiPass.shutdown();
            m_gbufferPass.shutdown();
            m_vizabilityPass.shutdown();
        }

        void render(RHI::RenderGraph* rg, const Scene* scene, RHI::RGImage* outputAttachment)
        {
            m_vizabilityPass.addPass(rg, "cull", &scene->m_drawList);
            m_gbufferPass.render(rg, *scene, m_vizabilityPass);

            auto* pass = rg->addPass("copy-to-output", RHI::RGPassType::Transfer, {});
            pass->readImage(m_gbufferPass.colorAttachment, RHI::RGImageUsage::CopySource);
            outputAttachment = pass->writeImage(outputAttachment, RHI::RGImageUsage::CopyDestination);
            rg->submitPass(pass, [this, rg, outputAttachment](RHI::CommandList& cmd)
                {
                    cmd.CopyImage({
                        .srcImage = rg->GetImageHandle(m_gbufferPass.colorAttachment),
                        .srcSize  = m_gbufferPass.colorAttachment->m_frameResource->size,
                        .dstImage = rg->GetImageHandle(outputAttachment),
                    });
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
    };

} // namespace Engine
