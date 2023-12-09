#include "Examples-Base/ImGuiRenderer.hpp"


bool ImGuiRenderer::Init(RHI::Context* context)
{
    (void)context;

    // Setup graphics pipeline
    {
        auto vertexShaderModule = LoadVertexShaderModule();
        auto pixelShaderModule  = LoadPixelShaderModule();

        // clang-format off
        RHI::GraphicsPipelineCreateInfo createInfo{
            .vertexShaderName = "main",
            .vertexShaderModule = vertexShaderModule.get(),

            .pixelShaderName = "main",
            .pixelShaderModule = pixelShaderModule.get(),

            .layout = m_pipelineLayout,

            .inputAssemblerState =
            {
                .bindings{ { .binding = 0, .stride = 0, .stepRate = RHI::PipelineVertexInputRate::PerVertex, }  },
                .attributes
                {
                    { .location = 0, .binding = 0, .format = RHI::Format::RG32_FLOAT,  .offset = offsetof(ImDrawVert, pos), },
                    { .location = 0, .binding = 0, .format = RHI::Format::RG32_FLOAT,  .offset = offsetof(ImDrawVert, uv),  },
                    { .location = 0, .binding = 0, .format = RHI::Format::RGBA8_UNORM, .offset = offsetof(ImDrawVert, col), },
                }
            },

            .renderTargetLayout =
            {
                .colorAttachmentsFormats = { RHI::Format::RGBA8_UNORM },
                .depthAttachmentFormat = RHI::Format::D32,
            },

            .rasterizationState =
            {
                .cullMode = RHI::PipelineRasterizerStateCullMode::None,
                .fillMode = RHI::PipelineRasterizerStateFillMode::Triangle,
                .frontFace = RHI::PipelineRasterizerStateFrontFace::CounterClockwise,
                .lineWidth = 1.0f,
            },

            .multisampleState =
            {
                .sampleCount = RHI::SampleCount::Samples1
            },

            .colorBlendState =
            {
                .blendStates = 
                {
                    { 
                        .blendEnable  = true,
                        .colorBlendOp = RHI::BlendEquation::Add,
                        .srcColor     = RHI::BlendFactor::SrcAlpha,
                        .dstColor     = RHI::BlendFactor::OneMinusSrcAlpha,
                        .alphaBlendOp = RHI::BlendEquation::Add,
                        .srcAlpha     = RHI::BlendFactor::One,
                        .dstAlpha     = RHI::BlendFactor::OneMinusSrcAlpha,
                        .writeMask    = RHI::ColorWriteMask::All,
                    },
                },
            },
        };

        // clang-format on
        m_pipeline = context->CreateGraphicsPipeline(createInfo);
    }

    return true;
}

void ImGuiRenderer::Shutdown()
{
}

void ImGuiRenderer::NewFrame()
{
}

void ImGuiRenderer::RenderDrawData(ImDrawData* drawData, RHI::CommandList& commandList)
{
    (void)drawData;
    (void)commandList;

    // Upload vertex/index data into a single contiguous GPU buffer
    auto vtxDst = (ImDrawVert*)m_bufferPool->MapBuffer(m_vertexBuffer);
    auto idxDst = (ImDrawIdx*)m_bufferPool->MapBuffer(m_indexBuffer);
    for (int i = 0; i < drawData->CmdListsCount; i++)
    {
        auto imCmdList = drawData->CmdLists[i];
        memcpy(vtxDst, imCmdList->VtxBuffer.Data, imCmdList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idxDst, imCmdList->IdxBuffer.Data, imCmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtxDst += imCmdList->VtxBuffer.Size;
        idxDst += imCmdList->IdxBuffer.Size;
    }
    m_bufferPool->UnmapBuffer(m_vertexBuffer);
    m_bufferPool->UnmapBuffer(m_indexBuffer);

    // Setup orthographic projection matrix into our uniform buffer
    // Our visible imgui space lies from drawData->DisplayPos (top left)
    // to drawData->DisplayPos+data_data->DisplaySize (bottom right).
    // DisplayPos is (0,0) for single viewport apps.
    {
        auto mappedUniformBuffer = m_bufferPool->MapBuffer(m_uniformBuffer);
        float L = drawData->DisplayPos.x;
        float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
        float T = drawData->DisplayPos.y;
        float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
        float mvp[4][4] = {
            { 2.0f / (R - L), 0.0f, 0.0f, 0.0f },
            { 0.0f, 2.0f / (T - B), 0.0f, 0.0f },
            { 0.0f, 0.0f, 0.5f, 0.0f },
            { (R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f },
        };
        memcpy(mappedUniformBuffer, mvp, sizeof(mvp));
        m_bufferPool->UnmapBuffer(m_uniformBuffer);
    }

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    size_t globalIndexOffset = 0;
    size_t globalVertexOffset = 0;
    ImVec2 clip_off = drawData->DisplayPos;
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        auto imCmdList = drawData->CmdLists[n];
        for (int cmd_i = 0; cmd_i < imCmdList->CmdBuffer.Size; cmd_i++)
        {
            auto pCmd = &imCmdList->CmdBuffer[cmd_i];
            if (pCmd->UserCallback != nullptr)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                // if (pCmd->UserCallback == ImDrawCallback_ResetRenderState)
                // ImGui_ImplDX11_SetupRenderState(drawData, ctx);
                // else
                // pCmd->UserCallback(imCmdList, pCmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_min(pCmd->ClipRect.x - clip_off.x, pCmd->ClipRect.y - clip_off.y);
                ImVec2 clip_max(pCmd->ClipRect.z - clip_off.x, pCmd->ClipRect.w - clip_off.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // Apply scissor/clipping rectangle
                const RHI::Scissor scissor = { (int32_t)clip_min.x, (int32_t)clip_min.y, (uint32_t)clip_max.x, (uint32_t)clip_max.y };

                // Bind texture, Draw
                // ctx->PSSetShaderResources(0, 1, &texture_srv);

                commandList.Submit(RHI::CommandDraw{
                    .pipelineState = m_pipeline,
                    .bindGroups = m_bindGroup,
                    .vertexBuffers = m_vertexBuffer,
                    .indexBuffers = m_indexBuffer,
                    .parameters = {
                        .elementCount = pCmd->ElemCount,
                        .vertexOffset = pCmd->VtxOffset + globalVertexOffset,
                        .offset } });
            }
        }
        globalIndexOffset += imCmdList->IdxBuffer.Size;
        globalVertexOffset += imCmdList->VtxBuffer.Size;
    }
}

std::unique_ptr<RHI::ShaderModule> ImGuiRenderer::LoadVertexShaderModule()
{
    RHI::ShaderModuleCreateInfo createInfo{};
    createInfo.code = GlslShaderVertSpv;
    createInfo.size = sizeof(GlslShaderVertSpv);
    return m_context->CreateShaderModule(createInfo);
}
