#include "ImGuiPass.hpp"
#include <imgui_internal.h>

#include "../PipelineLibrary.hpp"

#include <TL/Allocator/MemPlumber.hpp>
#include <TL/Defer.hpp>
#include <TL/FileSystem/File.hpp>
#include <TL/Log.hpp>
#include <TL/Utils.hpp>

#include <algorithm>

namespace Engine
{
    using ImGuiMat4 = float[4][4];

    ResultCode ImGuiPass::Init(RHI::Device* device, RHI::Format colorAttachmentFormat, uint32_t maxViewportsCount)
    {
        ImGuiIO& io = ImGui::GetIO();

        TL_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");
        m_newframeHook.Type     = ImGuiContextHookType_NewFramePre;
        m_newframeHook.UserData = this;
        m_newframeHook.Callback = [](TL_MAYBE_UNUSED ImGuiContext* ctx, ImGuiContextHook* hook)
        {
            auto self                 = (ImGuiPass*)hook->UserData;
            self->m_indexBufferOffset = self->m_vertexBufferOffset = 0;
        };
        ImGui::AddContextHook(ImGui::GetCurrentContext(), &m_newframeHook);

        m_device = device;

        // create sampler state
        RHI::SamplerCreateInfo samplerCI{
            .name   = "ImGui-Sampler",
            .minLod = 0.0f,
            .maxLod = 1.0f,
        };
        m_sampler = m_device->CreateSampler(samplerCI);

        RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI{
            .name     = "ImGui-BindGroupLayout",
            .bindings = {
                         {RHI::BindingType::DynamicUniformBuffer, RHI::Access::Read, 1, RHI::ShaderStage::Vertex, sizeof(ImGuiMat4)},
                         {RHI::BindingType::Sampler, RHI::Access::Read, 1, RHI::ShaderStage::Pixel, 0},
                         {RHI::BindingType::SampledImage, RHI::Access::Read, 1, RHI::ShaderStage::Pixel, 0},
                         }
        };
        auto bindGroupLayout = m_device->CreateBindGroupLayout(bindGroupLayoutCI);

        m_maxViewportsCount = maxViewportsCount;
        RHI::BufferCreateInfo uniformBufferCI{
            .name       = "ImGui-UniformBuffer",
            .usageFlags = RHI::BufferUsage::Uniform,
            .byteSize   = sizeof(ImGuiMat4) * m_maxViewportsCount,
        };
        m_uniformBuffer = m_device->CreateBuffer(uniformBufferCI);

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

        m_bindGroup = m_device->CreateBindGroup({.layout = bindGroupLayout});
        RHI::BindGroupUpdateInfo bindings{
            .buffers  = {{0, 0, {{m_uniformBuffer}}}},
            .images   = {{2, 0, {m_image}}},
            .samplers = {{1, 0, {m_sampler}}},
        };
        m_device->UpdateBindGroup(m_bindGroup, bindings);

        RHI::PipelineLayoutCreateInfo pipelineLayoutCI{.layouts = {bindGroupLayout}};
        m_pipelineLayout = m_device->CreatePipelineLayout(pipelineLayoutCI);

        auto vertexShaderModule = PipelineLibrary::ptr->LoadShaderModule("Shaders/ImGui.vertex.spv");
        auto fragmentShader     = PipelineLibrary::ptr->LoadShaderModule("Shaders/ImGui.fragment.spv");

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
        return ResultCode::Success;
    }

    void ImGuiPass::Shutdown()
    {
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
    }

    RHI::RGPass* ImGuiPass::AddPass(RHI::RenderGraph* rg, RHI::RGImage*& outAttachment, ImDrawData* drawData, uint32_t viewportID)
    {
        TL_ASSERT(m_maxViewportsCount > viewportID);

        if (UpdateBuffers(drawData) == false)
            return nullptr;

        {
            float     L   = drawData->DisplayPos.x;
            float     R   = drawData->DisplayPos.x + drawData->DisplaySize.x;
            float     T   = drawData->DisplayPos.y;
            float     B   = drawData->DisplayPos.y + drawData->DisplaySize.y;
            ImGuiMat4 mvp = {
                {2.0f / (R - L),    0.0f,              0.0f, 0.0f},
                {0.0f,              2.0f / (T - B),    0.0f, 0.0f},
                {0.0f,              0.0f,              0.5f, 0.0f},
                {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
            };
            m_device->GetCurrentFrame()->BufferWrite(m_uniformBuffer, sizeof(ImGuiMat4) * viewportID, {mvp, sizeof(ImGuiMat4)});
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

                size_t& indexBufferOffset  = m_indexBufferOffset;
                size_t& vertexBufferOffset = m_vertexBufferOffset;
                for (const auto& drawList : drawData->CmdLists)
                {
                    // clang-format off
                    commandList.BindIndexBuffer({.buffer = m_indexBuffer, .offset = indexBufferOffset}, RHI::IndexType::uint16);
                    commandList.BindVertexBuffers(0, {{.buffer = m_vertexBuffer, .offset = vertexBufferOffset}});
                    // clang-format on

                    m_device->GetCurrentFrame()->BufferWrite(m_indexBuffer, indexBufferOffset, TL::Block{.ptr = drawList->IdxBuffer.Data, .size = drawList->IdxBuffer.Size * sizeof(ImDrawIdx)});
                    m_device->GetCurrentFrame()->BufferWrite(m_vertexBuffer, vertexBufferOffset, TL::Block{.ptr = drawList->VtxBuffer.Data, .size = drawList->VtxBuffer.Size * sizeof(ImDrawVert)});
                    indexBufferOffset += drawList->IdxBuffer.Size * sizeof(ImDrawIdx);
                    vertexBufferOffset += drawList->VtxBuffer.Size * sizeof(ImDrawVert);

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

                            RHI::BindGroupBindingInfo binding{m_bindGroup, viewportID * sizeof(ImGuiMat4)};
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

        if ((size_t)drawData->TotalVtxCount > m_vertexBufferSize)
        {
            m_vertexBufferSize = (size_t)drawData->TotalVtxCount + 5000;

            if (m_vertexBuffer != nullptr)
                m_device->DestroyBuffer(m_vertexBuffer);

            RHI::BufferCreateInfo createInfo{};
            createInfo.name       = "ImGui-VertexBuffer";
            createInfo.byteSize   = m_vertexBufferSize * sizeof(ImDrawVert);
            createInfo.usageFlags = RHI::BufferUsage::Vertex;
            m_vertexBuffer        = m_device->CreateBuffer(createInfo);
        }

        if ((size_t)drawData->TotalIdxCount > m_indexBufferSize)
        {
            m_indexBufferSize = (size_t)drawData->TotalIdxCount + 10000;

            if (m_indexBuffer != nullptr)
                m_device->DestroyBuffer(m_indexBuffer);

            RHI::BufferCreateInfo createInfo{};
            createInfo.name       = "ImGui-IndexBuffer";
            createInfo.byteSize   = m_indexBufferSize * sizeof(ImDrawIdx);
            createInfo.usageFlags = RHI::BufferUsage::Index;
            m_indexBuffer         = m_device->CreateBuffer(createInfo);
        }

        if (drawData->TotalIdxCount == 0 || drawData->TotalVtxCount == 0)
            return false;

        return true;
    }
} // namespace Engine