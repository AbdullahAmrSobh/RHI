#include "Renderer/ImGuiPass.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/PipelineLibrary.hpp"
#include "Renderer/Scene.hpp"

#include <TL/Allocator/MemPlumber.hpp>
#include <TL/Defer.hpp>
#include <TL/FileSystem/File.hpp>
#include <TL/Log.hpp>
#include <TL/Utils.hpp>

#include <algorithm>

#include <Renderer-Shaders/ImGui.hpp>

namespace Engine
{
    ResultCode ImGuiPass::Init(RHI::Device* device, RHI::Format colorAttachmentFormat, uint32_t maxViewportsCount)
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
        m_image = RHI::CreateImageWithContent(*m_device, atlasTextureCI, TL::Block{pixels, size_t(width * height * 4)});
        m_projectionCB = createDynamicConstantBuffer<glm::mat4x4>(GpuSceneData::ptr->m_constantBuffersPool, m_maxViewportsCount);

        RHI::BindGroupLayout* bindGroupLayout = sig::ImGuiShaderParam::createBindGroupLayout(device);

        m_bindGroup                        = m_device->CreateBindGroup({.layout = bindGroupLayout});
        sig::ImGuiShaderParam shaderParams = {};
        shaderParams.cb                    = this->m_projectionCB;
        shaderParams.texture0              = m_image;
        shaderParams.sampler0              = m_sampler;
        shaderParams.updateBindGroup(m_device, m_bindGroup);

        RHI::PipelineLayoutCreateInfo pipelineLayoutCI{
            .layouts = {bindGroupLayout},
        };
        m_pipelineLayout = m_device->CreatePipelineLayout(pipelineLayoutCI);

        auto vertexShaderModule = PipelineLibrary::ptr->LoadShaderModule("I:/repos/repos3/RHI/build/Examples/Renderer/Shaders/ImGui.spirv.VSMain");
        auto fragmentShader     = PipelineLibrary::ptr->LoadShaderModule("I:/repos/repos3/RHI/build/Examples/Renderer/Shaders/ImGui.spirv.PSMain");

        RHI::ColorAttachmentBlendStateDesc attachmentBlendDesc{
            true,
            RHI::BlendEquation::Add,
            RHI::BlendFactor::SrcAlpha,
            RHI::BlendFactor::OneMinusSrcAlpha,
            RHI::BlendEquation::Add,
            RHI::BlendFactor::One,
            RHI::BlendFactor::OneMinusSrcAlpha,
            RHI::ColorWriteMask::All,
        };
        RHI::GraphicsPipelineCreateInfo pipelineCI{
            .name                 = "ImGui Pipeline",
            .vertexShaderName     = "VSMain",
            .vertexShaderModule   = vertexShaderModule,
            .pixelShaderName      = "PSMain",
            .pixelShaderModule    = fragmentShader,
            .layout               = m_pipelineLayout,
            .vertexBufferBindings = {{
                .stride     = sizeof(ImDrawVert),
                .stepRate   = RHI::PipelineVertexInputRate::PerVertex,
                .attributes = {
                    {.offset = offsetof(ImDrawVert, pos), .format = RHI::Format::RG32_FLOAT},
                    {.offset = offsetof(ImDrawVert, uv), .format = RHI::Format::RG32_FLOAT},
                    {.offset = offsetof(ImDrawVert, col), .format = RHI::Format::RGBA8_UNORM},
                },
            }},
            .renderTargetLayout   = {.colorAttachmentsFormats = RHI::Format::RGBA8_UNORM},
            .colorBlendState      = {.blendStates = {attachmentBlendDesc}},
            .rasterizationState   = {.cullMode = RHI::PipelineRasterizerStateCullMode::None},
        };
        m_pipeline = m_device->CreateGraphicsPipeline(pipelineCI);
        m_device->DestroyBindGroupLayout(bindGroupLayout);

        TL_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");
        m_newframeHook.Type     = ImGuiContextHookType_NewFramePre;
        m_newframeHook.UserData = this;
        m_newframeHook.Callback = [](TL_MAYBE_UNUSED ImGuiContext* ctx, ImGuiContextHook* hook)
        {
            auto self = (ImGuiPass*)hook->UserData;
            // self->m_indexBufferOffset = self->m_vertexBufferOffset = 0;
        };
        ImGui::AddContextHook(ImGui::GetCurrentContext(), &m_newframeHook);

        return ResultCode::Success;
    }

    void ImGuiPass::Shutdown()
    {
#if 0
        ImGui::RemoveContextHook(ImGui::GetCurrentContext(), m_newframeHook.HookId);
        m_device->DestroyGraphicsPipeline(m_pipeline);
        m_device->DestroyPipelineLayout(m_pipelineLayout);
        m_device->DestroyBindGroup(m_bindGroup);
        m_device->DestroySampler(m_sampler);
        m_device->DestroyImage(m_image);
        if (m_indexBuffer != nullptr)
            m_device->DestroyBuffer(m_indexBuffer);
        if (m_vertexBuffer != nullptr)
            m_device->DestroyBuffer(m_vertexBuffer);
        if (m_uniformBuffer != nullptr)
            m_device->DestroyBuffer(m_uniformBuffer);
#endif
    }

    RHI::RGPass* ImGuiPass::AddPass(RHI::RenderGraph* rg, RHI::RGImage*& outAttachment, ImDrawData* drawData, uint32_t viewportID)
    {
        TL_ASSERT(m_maxViewportsCount > viewportID);

        if (UpdateBuffers(drawData) == false)
            return nullptr;

        {
            float L = drawData->DisplayPos.x;
            float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
            float T = drawData->DisplayPos.y;
            float B = drawData->DisplayPos.y + drawData->DisplaySize.y;

            glm::mat4x4 mvp = {
                {2.0f / (R - L),    0.0f,              0.0f, 0.0f},
                {0.0f,              2.0f / (T - B),    0.0f, 0.0f},
                {0.0f,              0.0f,              0.5f, 0.0f},
                {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
            };
            bufferWrite(GpuSceneData::ptr->m_constantBuffersPool, m_projectionCB, viewportID, mvp);
        }

        auto [width, height] = drawData->OwnerViewport->Size;
        return rg->AddPass({
            .name          = "ImGui",
            .type          = RHI::PassType::Graphics,
            .size          = {(uint32_t)width, (uint32_t)height},
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                builder.AddColorAttachment({.color = outAttachment, .loadOp = RHI::LoadOperation::Discard});
                              },
            .executeCallback = [=, this](RHI::CommandList& commandList)
            {
                // Will project scissor/clipping rectangles into framebuffer space
                ImVec2 clipOff   = drawData->DisplayPos;       // (0,0) unless using multi-viewports
                ImVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

                int fb_width  = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
                int fb_height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
                if (fb_width <= 0 || fb_height <= 0)
                    return;

                uint32_t indexBufferOffset  = m_indexBuffer.getOffset();
                uint32_t vertexBufferOffset = m_vertexBuffer.getOffset();
                for (const auto& drawList : drawData->CmdLists)
                {

                    // TODO: use api, but need to accmulate elements count instead of offset in bytes ...

                    // clang-format off
                    commandList.BindIndexBuffer({.buffer = m_indexBuffer.getBuffer(), .offset = indexBufferOffset}, RHI::IndexType::uint16);
                    commandList.BindVertexBuffers(0, {{.buffer = m_vertexBuffer.getBuffer(), .offset = vertexBufferOffset}});
                    // clang-format on

                    // TODO: use gpuscenedata::update
                    m_device->GetCurrentFrame()->BufferWrite(m_indexBuffer.getBuffer(), indexBufferOffset, TL::Block{.ptr = drawList->IdxBuffer.Data, .size = drawList->IdxBuffer.Size * sizeof(ImDrawIdx)});
                    m_device->GetCurrentFrame()->BufferWrite(m_vertexBuffer.getBuffer(), vertexBufferOffset, TL::Block{.ptr = drawList->VtxBuffer.Data, .size = drawList->VtxBuffer.Size * sizeof(ImDrawVert)});
                    indexBufferOffset += drawList->IdxBuffer.Size * sizeof(ImDrawIdx);
                    vertexBufferOffset += drawList->VtxBuffer.Size * sizeof(ImDrawVert);

                    // bufferWrite(GpuSceneData::ptr->m_geometryBuffersPool, m_indexBuffer, indexBufferOffset, {drawList->IdxBuffer.Data, (size_t)drawList->IdxBuffer.size()});
                    // bufferWrite(GpuSceneData::ptr->m_geometryBuffersPool, m_vertexBuffer, vertexBufferOffset, {drawList->VtxBuffer.Data, (size_t)drawList->VtxBuffer.size()});
                    // indexBufferOffset += drawList->IdxBuffer.size();
                    // vertexBufferOffset += drawList->VtxBuffer.size();

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
                            commandList.BindGraphicsPipeline(m_pipeline, binding);

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

    bool ImGuiPass::UpdateBuffers(ImDrawData* drawData)
    {
        // Avoid rendering when minimized
        if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
            return false;

        auto& geometryBuffers  = GpuSceneData::ptr->m_geometryBuffersPool;
        auto  vertexBufferSize = geometryBuffers.getBufferElementsCount(m_vertexBuffer);
        if ((size_t)drawData->TotalVtxCount > vertexBufferSize)
        {
            vertexBufferSize = (size_t)drawData->TotalVtxCount + 5000;

            if (m_vertexBuffer.getBuffer())
                geometryBuffers.free(m_vertexBuffer);

            m_vertexBuffer = geometryBuffers.allocate<ImDrawVert>(vertexBufferSize);
        }

        auto indexBufferSize = geometryBuffers.getBufferElementsCount(m_indexBuffer);
        if ((size_t)drawData->TotalIdxCount > indexBufferSize)
        {
            indexBufferSize = (size_t)drawData->TotalIdxCount + 10000;

            if (m_indexBuffer.getBuffer())
                geometryBuffers.free(m_indexBuffer);

            m_indexBuffer = geometryBuffers.allocate<ImDrawIdx>(indexBufferSize);
        }

        if (drawData->TotalIdxCount == 0 || drawData->TotalVtxCount == 0)
            return false;

        return true;
    }
} // namespace Engine