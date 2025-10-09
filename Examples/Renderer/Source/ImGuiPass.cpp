#include "Renderer/ImGuiPass.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/PipelineLibrary.hpp"
#include "Renderer/Scene.hpp"

#include <TL/Allocator/MemPlumber.hpp>
#include <TL/Defer.hpp>
#include <TL/FileSystem/File.hpp>
#include <TL/Log.hpp>
#include <TL/Utils.hpp>
#include <TL/Literals.hpp>

#include <algorithm>

#include <Renderer-Shaders/ImGui.hpp>

namespace Engine
{
    TL::Error ImGuiPass::init(RHI::Device* device, RHI::Format colorAttachmentFormat, uint32_t maxViewportsCount)
    {
        m_device            = device;
        m_maxViewportsCount = maxViewportsCount;
        m_indexBuffer       = {};
        m_vertexBuffer      = {};

        ImGuiIO& io = ImGui::GetIO();

        // create sampler state
        RHI::SamplerCreateInfo samplerCI{
            .name   = "ImGui-Sampler",
            .minLod = 0.0f,
            .maxLod = 1.0f,
        };
        m_sampler = m_device->CreateSampler(samplerCI);

        // create ImGui font atlas texture
        unsigned char* pixels;
        int            width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        RHI::ImageCreateInfo atlasTextureCI{
            .name       = "ImGui-Atlas",
            .usageFlags = RHI::ImageUsage::CopyDst | RHI::ImageUsage::ShaderResource,
            .type       = RHI::ImageType::Image2D,
            .size       = {uint32_t(width), uint32_t(height)},
            .format     = RHI::Format::RGBA8_UNORM,
        };
        m_image        = RHI::CreateImageWithContent(*m_device, atlasTextureCI, TL::Block{pixels, size_t(width * height * 4)});
        m_projectionCB = RenderContext::ptr->getConstantBuffersPool().allocate<glm::mat4x4>(m_maxViewportsCount);

        m_shader = PipelineLibrary::ptr->acquireGraphicsPipeline<GPU::ImGuiShaderParam>(
            "I:/repos/repos3/RHI/Examples/Renderer/Shaders/source/ImGui.json",
            {
                {
                    .stride     = sizeof(ImDrawVert),
                    .stepRate   = RHI::PipelineVertexInputRate::PerVertex,
                    .attributes = {
                        {.offset = offsetof(ImDrawVert, pos), .format = RHI::Format::RG32_FLOAT},
                        {.offset = offsetof(ImDrawVert, uv), .format = RHI::Format::RG32_FLOAT},
                        {.offset = offsetof(ImDrawVert, col), .format = RHI::Format::RGBA8_UNORM},
                    },
                },
            },
            {RHI::Format::RGBA8_UNORM});

        m_bindGroup = m_device->CreateBindGroup({.layout = m_shader->getBindGroupLayout(0)});

        GPU::ImGuiShaderParam shaderParams = {};
        shaderParams.projection            = this->m_projectionCB;
        shaderParams.texture0              = m_image;
        shaderParams.sampler0              = m_sampler;
        shaderParams.updateBindGroup(m_device, m_bindGroup);


        TL_ASSERT(io.BackendRendererUserData == nullptr, "Already initialized a renderer backend!");
        m_newframeHook.Type     = ImGuiContextHookType_NewFramePre;
        m_newframeHook.UserData = this;
        m_newframeHook.Callback = [](TL_MAYBE_UNUSED ImGuiContext* ctx, ImGuiContextHook* hook)
        {
            auto self = (ImGuiPass*)hook->UserData;
            // self->m_indexBufferOffset = self->m_vertexBufferOffset = 0;
        };
        ImGui::AddContextHook(ImGui::GetCurrentContext(), &m_newframeHook);

        return TL::NoError;
    }

    void ImGuiPass::shutdown()
    {
        // ImGui::RemoveContextHook(ImGui::GetCurrentContext(), m_newframeHook.HookId);
        // RenderContext::ptr->m_geometryBuffersPool.free(m_vertexBuffer);
        // RenderContext::ptr->m_geometryBuffersPool.free(m_indexBuffer);
        // freeDynamicConstantBuffer(RenderContext::ptr->m_constantBuffersPool, m_projectionCB);
        m_device->DestroyImage(m_image);
        m_device->DestroySampler(m_sampler);
        m_device->DestroyBindGroup(m_bindGroup);
        // m_device->DestroyGraphicsPipeline(m_pipeline);
        // m_device->DestroyPipelineLayout(m_pipelineLayout);
    }

    RHI::RGPass* ImGuiPass::addPass(RHI::RenderGraph* rg, RHI::RGImage*& outAttachment, ImDrawData* drawData, uint32_t viewportID)
    {
        TL_ASSERT(m_maxViewportsCount > viewportID);

        if (updateBuffers(drawData) == false)
            return nullptr;

        {
            float       L = drawData->DisplayPos.x;
            float       R = drawData->DisplayPos.x + drawData->DisplaySize.x;
            float       T = drawData->DisplayPos.y;
            float       B = drawData->DisplayPos.y + drawData->DisplaySize.y;
            glm::mat4x4 mvp{
                {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
                {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
                {0.0f, 0.0f, 0.5f, 0.0f},
                {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
            };
            RenderContext::ptr->getConstantBuffersPool().update(m_projectionCB, viewportID, TL::Span<const glm::mat4x4>{mvp});
        }

        auto [width, height] = drawData->OwnerViewport->Size;
        return rg->AddPass({
            .name          = "ImGui",
            .type          = RHI::PassType::Graphics,
            .size          = {(uint32_t)width, (uint32_t)height},
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                builder.AddColorAttachment({.color = outAttachment, .loadOp = RHI::LoadOperation::Load});
            },
            .executeCallback = [=, this](RHI::CommandList& commandList)
            {
                auto& pool = RenderContext::ptr->getUnifiedGeometryBuffersPool();

                // Will project scissor/clipping rectangles into framebuffer space
                ImVec2 clipOff   = drawData->DisplayPos;       // (0,0) unless using multi-viewports
                ImVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

                int fb_width  = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
                int fb_height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
                if (fb_width <= 0 || fb_height <= 0)
                    return;

                uint32_t indexBufferOffset  = 0;
                uint32_t vertexBufferOffset = 0;
                for (const auto& drawList : drawData->CmdLists)
                {
                    commandList.BindIndexBuffer(m_indexBuffer.getBinding(indexBufferOffset), RHI::IndexType::uint16);
                    commandList.BindVertexBuffers(0, m_vertexBuffer.getBinding(vertexBufferOffset));

                    pool.update(m_indexBuffer, indexBufferOffset, {drawList->IdxBuffer.Data, (size_t)drawList->IdxBuffer.Size});
                    pool.update(m_vertexBuffer, vertexBufferOffset, {drawList->VtxBuffer.Data, (size_t)drawList->VtxBuffer.Size});
                    indexBufferOffset += drawList->IdxBuffer.Size;
                    vertexBufferOffset += drawList->VtxBuffer.Size;

                    for (const auto& drawCmd : drawList->CmdBuffer)
                    {
                        if (drawCmd.UserCallback)
                        {
                            drawCmd.UserCallback(drawList, &drawCmd);
                        }
                        else
                        {
                            // Project scissor/clipping rectangles into framebuffer space
                            ImVec2 clip_min((drawCmd.ClipRect.x - clipOff.x) * clipScale.x, (drawCmd.ClipRect.y - clipOff.y) * clipScale.y);
                            ImVec2 clip_max((drawCmd.ClipRect.z - clipOff.x) * clipScale.x, (drawCmd.ClipRect.w - clipOff.y) * clipScale.y);

                            // Clamp to viewport as vkCmdSetScissor won't accept values that are off bounds
                            clip_min.x = std::max(clip_min.x, 0.0f);
                            clip_min.y = std::max(clip_min.y, 0.0f);
                            clip_max.x = std::max(clip_max.x, (float)fb_width);
                            clip_max.y = std::max(clip_max.y, (float)fb_height);
                            if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                                continue;

                            RHI::Viewport viewport{
                                .width    = (float)width,
                                .height   = (float)height,
                                .maxDepth = 1.0,
                            };
                            commandList.SetViewport(viewport);
                            // Apply scissor/clipping rectangle
                            RHI::Scissor scissor{
                                .offsetX = (int32_t)(clip_min.x),
                                .offsetY = (int32_t)(clip_min.y),
                                .width   = (uint32_t)(clip_max.x - clip_min.x),
                                .height  = (uint32_t)(clip_max.y - clip_min.y),
                            };
                            commandList.SetScissor(scissor);

                            // TODO: dynamic offset are not supported yet
                            RHI::BindGroupBindingInfo binding{m_bindGroup, {}};
                            // RHI::BindGroupBindingInfo binding{m_bindGroup, (uint32_t)m_projectionCB.getOffset(viewportID)};
                            commandList.BindGraphicsPipeline(m_shader->getPipeline(), binding);

                            commandList.DrawIndexed({
                                .indexCount    = drawCmd.ElemCount,
                                .instanceCount = 1,
                                .firstIndex    = drawCmd.IdxOffset,
                                .vertexOffset  = int32_t(drawCmd.VtxOffset),
                                .firstInstance = 0,

                            });
                        }
                    }
                }
            },
        });
    }

    bool ImGuiPass::updateBuffers(ImDrawData* drawData)
    {
        // Avoid rendering when minimized
        if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
            return false;

        auto& pool = RenderContext::ptr->getUnifiedGeometryBuffersPool();

        auto vertexBufferSize = m_vertexBuffer.getElementsCount();
        if ((size_t)drawData->TotalVtxCount > vertexBufferSize)
        {
            vertexBufferSize = (size_t)drawData->TotalVtxCount + 5000;

            if (m_vertexBuffer.getBuffer())
                pool.free<ImDrawVert>(m_vertexBuffer);

            m_vertexBuffer = pool.allocate<ImDrawVert>(vertexBufferSize);
        }

        auto indexBufferSize = m_indexBuffer.getElementsCount();
        if ((size_t)drawData->TotalIdxCount > indexBufferSize)
        {
            indexBufferSize = (size_t)drawData->TotalIdxCount + 10000;

            if (m_indexBuffer.getBuffer())
                pool.free<ImDrawIdx>(m_indexBuffer);

            m_indexBuffer = pool.allocate<ImDrawIdx>(indexBufferSize);
        }

        if (drawData->TotalIdxCount == 0 || drawData->TotalVtxCount == 0)
            return false;

        return true;
    }
} // namespace Engine