#include "ImGuiPass.hpp"

#include "../PipelineLibrary.hpp"

#include <TL/Allocator/MemPlumber.hpp>
#include <TL/Defer.hpp>
#include <TL/FileSystem/File.hpp>
#include <TL/Log.hpp>
#include <TL/Utils.hpp>

#include <algorithm>

namespace Engine
{

    ResultCode ImGuiPass::Init(RHI::Device* device, RHI::Format colorAttachmentFormat)
    {
        m_imguiContext = ImGui::CreateContext();
        ImGui::SetCurrentContext(m_imguiContext);

        ImGuiIO& io = ImGui::GetIO();
        TL_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

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
                         {RHI::BindingType::UniformBuffer, RHI::Access::Read, 1, RHI::ShaderStage::Vertex, sizeof(float) * 4 * 4},
                         {RHI::BindingType::Sampler, RHI::Access::Read, 1, RHI::ShaderStage::Pixel},
                         {RHI::BindingType::SampledImage, RHI::Access::Read, 1, RHI::ShaderStage::Pixel},
                         }
        };
        auto bindGroupLayout = m_device->CreateBindGroupLayout(bindGroupLayoutCI);

        RHI::BufferCreateInfo uniformBufferCI{};
        uniformBufferCI.name       = "ImGui-UniformBuffer";
        uniformBufferCI.byteSize   = sizeof(float) * 4 * 4;
        uniformBufferCI.usageFlags = RHI::BufferUsage::Uniform;
        m_uniformBuffer            = m_device->CreateBuffer(uniformBufferCI);

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

        auto vertexShaderModule = LoadShaderModule(m_device, "Shaders/ImGui.vertex.spv");
        auto fragmentShader     = LoadShaderModule(m_device, "Shaders/ImGui.fragment.spv");

        RHI::ColorAttachmentBlendStateDesc attachmentBlendDesc =
            {
                true,
                RHI::BlendEquation::Add,
                RHI::BlendFactor::SrcAlpha,
                RHI::BlendFactor::OneMinusSrcAlpha,
                RHI::BlendEquation::Add,
                RHI::BlendFactor::One,
                RHI::BlendFactor::OneMinusSrcAlpha,
                RHI::ColorWriteMask::All,
            };
        RHI::GraphicsPipelineCreateInfo pipelineCI =
            {
                .name               = "ImGui Pipeline",
                .vertexShaderName   = "VSMain",
                .vertexShaderModule = vertexShaderModule,
                .pixelShaderName    = "PSMain",
                .pixelShaderModule  = fragmentShader,
                .layout             = m_pipelineLayout,
                .vertexBufferBindings =
                    {
                                           {
                            .stride   = sizeof(ImDrawVert),
                            .stepRate = RHI::PipelineVertexInputRate::PerVertex,
                            .attributes =
                                {
                                    {.offset = offsetof(ImDrawVert, pos), .format = RHI::Format::RG32_FLOAT},
                                    {.offset = offsetof(ImDrawVert, uv), .format = RHI::Format::RG32_FLOAT},
                                    {.offset = offsetof(ImDrawVert, col), .format = RHI::Format::RGBA8_UNORM},
                                },
                        },
                                           },
                .renderTargetLayout =
                    {
                                           .colorAttachmentsFormats = RHI::Format::RGBA8_UNORM,
                                           },
                .colorBlendState =
                    {
                                           .blendStates    = {attachmentBlendDesc},
                                           .blendConstants = {},
                                           },
                .rasterizationState =
                    {
                                           .cullMode  = RHI::PipelineRasterizerStateCullMode::None,
                                           .fillMode  = RHI::PipelineRasterizerStateFillMode::Triangle,
                                           .frontFace = RHI::PipelineRasterizerStateFrontFace::CounterClockwise,
                                           .lineWidth = 1.0f,
                                           },
        };
        m_pipeline = m_device->CreateGraphicsPipeline(pipelineCI);
        m_device->DestroyBindGroupLayout(bindGroupLayout);
        return ResultCode::Success;
    }

    void ImGuiPass::Shutdown()
    {
        m_device->DestroyGraphicsPipeline(m_pipeline);
        m_device->DestroyPipelineLayout(m_pipelineLayout);
        m_device->DestroyBindGroup(m_bindGroup);
        m_device->DestroySampler(m_sampler);
        m_device->DestroyImage(m_image);
        if (m_indexBuffer != RHI::NullHandle)
            m_device->DestroyBuffer(m_indexBuffer);
        if (m_vertexBuffer != RHI::NullHandle)
            m_device->DestroyBuffer(m_vertexBuffer);
        if (m_uniformBuffer != RHI::NullHandle)
            m_device->DestroyBuffer(m_uniformBuffer);
    }

    RHI::RGPass* ImGuiPass::AddPass(RHI::RenderGraph* rg, RHI::RGImage*& outAttachment, ImDrawData* drawData)
    {
        UpdateBuffers(drawData);

        return rg->AddPass({
            .name          = "ImGui",
            .type          = RHI::PassType::Graphics,
            .size          = rg->GetFrameSize(),
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                builder.AddColorAttachment({.color = outAttachment, .loadOp = RHI::LoadOperation::Load});
            },
            .executeCallback = [=](RHI::CommandList& commandList)
            {
                // Render command lists
                int globalIdxOffset = 0;
                int globalVtxOffset = 0;

                // Will project scissor/clipping rectangles into framebuffer space
                ImVec2 clipOff   = drawData->DisplayPos;       // (0,0) unless using multi-viewports
                ImVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
                for (int n = 0; n < drawData->CmdListsCount; n++)
                {
                    const ImDrawList* drawList = drawData->CmdLists[n];
                    for (int i = 0; i < drawList->CmdBuffer.Size; i++)
                    {
                        const ImDrawCmd* drawCmd = &drawList->CmdBuffer[i];

                        if (drawCmd->UserCallback)
                        {
                            drawCmd->UserCallback(drawList, drawCmd);
                        }
                        else
                        {
                            // Project scissor/clipping rectangles into framebuffer space
                            ImVec2 clip_min((drawCmd->ClipRect.x - clipOff.x) * clipScale.x, (drawCmd->ClipRect.y - clipOff.y) * clipScale.y);
                            ImVec2 clip_max((drawCmd->ClipRect.z - clipOff.x) * clipScale.x, (drawCmd->ClipRect.w - clipOff.y) * clipScale.y);

                            // Clamp to viewport as commandList.SetSicssor() won't accept values that are off bounds
                            clip_min.x = std::clamp(clip_min.x, 0.0f, drawData->DisplaySize.x);
                            clip_min.y = std::clamp(clip_min.y, 0.0f, drawData->DisplaySize.y);
                            clip_max.x = std::clamp(clip_max.x, 0.0f, drawData->DisplaySize.x);
                            clip_max.y = std::clamp(clip_max.y, 0.0f, drawData->DisplaySize.y);
                            if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                                continue;

                            commandList.SetViewport({
                                .width    = (float)rg->GetFrameSize().width,
                                .height   = (float)rg->GetFrameSize().height,
                                .maxDepth = 1.0,
                            });
                            // Apply scissor/clipping rectangle
                            RHI::Scissor scissor{
                                .offsetX = (int32_t)(clip_min.x),
                                .offsetY = (int32_t)(clip_min.y),
                                .width   = (uint32_t)(clip_max.x - clip_min.x),
                                .height  = (uint32_t)(clip_max.y - clip_min.y),
                            };
                            commandList.SetScissor(scissor);

                            commandList.BindGraphicsPipeline(m_pipeline, {{.bindGroup = m_bindGroup}});
                            commandList.BindIndexBuffer({.buffer = m_indexBuffer}, RHI::IndexType::uint16);
                            commandList.BindVertexBuffers(0, {{.buffer = m_vertexBuffer}});
                            commandList.DrawIndexed({
                                .indexCount    = drawCmd->ElemCount,
                                .instanceCount = 1,
                                .firstIndex    = drawCmd->IdxOffset + globalIdxOffset,
                                .vertexOffset  = int32_t(drawCmd->VtxOffset + globalVtxOffset),
                                .firstInstance = 0,

                            });
                        }
                    }

                    globalIdxOffset += drawList->IdxBuffer.Size;
                    globalVtxOffset += drawList->VtxBuffer.Size;
                }
            },
        });
    }

    void ImGuiPass::UpdateBuffers(ImDrawData* drawData)
    {
        // Avoid rendering when minimized
        if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
            return;

        if ((size_t)drawData->TotalVtxCount > m_vertexBufferSize)
        {
            m_vertexBufferSize = (size_t)drawData->TotalVtxCount + 5000;

            if (m_vertexBuffer != RHI::NullHandle)
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

            if (m_indexBuffer != RHI::NullHandle)
                m_device->DestroyBuffer(m_indexBuffer);

            RHI::BufferCreateInfo createInfo{};
            createInfo.name       = "ImGui-IndexBuffer";
            createInfo.byteSize   = m_indexBufferSize * sizeof(ImDrawIdx);
            createInfo.usageFlags = RHI::BufferUsage::Index;
            m_indexBuffer         = m_device->CreateBuffer(createInfo);
        }

        if (drawData->TotalIdxCount == 0 || drawData->TotalVtxCount == 0)
            return;

        size_t indexBufferOffset  = 0;
        size_t vertexBufferOffset = 0;
        for (int n = 0; n < drawData->CmdListsCount; n++)
        {
            const ImDrawList* cmdList = drawData->CmdLists[n];
            m_device->BufferWrite(m_indexBuffer, indexBufferOffset, TL::Block{.ptr = cmdList->IdxBuffer.Data, .size = cmdList->IdxBuffer.Size * sizeof(ImDrawIdx)});
            m_device->BufferWrite(m_vertexBuffer, vertexBufferOffset, TL::Block{.ptr = cmdList->VtxBuffer.Data, .size = cmdList->VtxBuffer.Size * sizeof(ImDrawVert)});
            indexBufferOffset += cmdList->IdxBuffer.Size;
            vertexBufferOffset += cmdList->VtxBuffer.Size;
        }
        // m_device->UnmapBuffer(m_vertexBuffer);
        // m_device->UnmapBuffer(m_indexBuffer);

        {
            float L = drawData->DisplayPos.x;
            float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
            float T = drawData->DisplayPos.y;
            float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
            // clang-format off
            float mvp[4][4] =
            {
                { 2.0f / (R - L), 0.0f,                 0.0f, 0.0f },
                { 0.0f,           2.0f / (T - B),       0.0f, 0.0f },
                { 0.0f,           0.0f,                 0.5f, 0.0f },
                { (R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f },
            };
            // clang-format on
            // auto  uniformBufferPtr = m_device->MapBuffer(m_uniformBuffer);
            m_device->BufferWrite(m_uniformBuffer, 0, {mvp, sizeof(mvp)});
            // m_device->UnmapBuffer(m_uniformBuffer);
        }
    }
} // namespace Engine