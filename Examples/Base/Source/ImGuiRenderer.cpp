#include "Examples-Base/ImGuiRenderer.hpp"

void ImGuiRenderer::Init(RHI::Context* context, RHI::FrameScheduler* scheduler, RHI::CommandListAllocator* commandListAllocator, RHI::BindGroupAllocator* bindGroupAllocator, RHI::ImagePool& imagePool, RHI::BufferPool& bufferPool, const std::vector<uint32_t>& shaderModuleBlob)
{
    m_imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_imguiContext);

    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    m_context = context;
    m_imagePool = &imagePool;
    m_bufferPool = &bufferPool;

    // create sampler state
    {
        RHI::SamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.filterMag = RHI::SamplerFilter::Linear;
        samplerCreateInfo.filterMin = RHI::SamplerFilter::Linear;
        samplerCreateInfo.filterMip = RHI::SamplerFilter::Linear;
        samplerCreateInfo.addressU = RHI::SamplerAddressMode::Repeat;
        samplerCreateInfo.addressV = RHI::SamplerAddressMode::Repeat;
        samplerCreateInfo.addressW = RHI::SamplerAddressMode::Repeat;
        m_sampler = m_context->CreateSampler(samplerCreateInfo);
    }

    {
        RHI::BindGroupLayoutCreateInfo createInfo{};
        createInfo.bindings = {
            RHI::ShaderBinding{ .type = RHI::ShaderBindingType::Buffer, .access = RHI::ShaderBindingAccess::OnlyRead, .arrayCount = 1, .stages = RHI::ShaderStage::Vertex },
            RHI::ShaderBinding{ .type = RHI::ShaderBindingType::Sampler, .access = RHI::ShaderBindingAccess::OnlyRead, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel },
            RHI::ShaderBinding{ .type = RHI::ShaderBindingType::Image, .access = RHI::ShaderBindingAccess::OnlyRead, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel },
        };

        m_bindGroupLayout = m_context->CreateBindGroupLayout(createInfo);

        {
            RHI::BufferCreateInfo bufferCreateInfo{};
            bufferCreateInfo.byteSize = sizeof(float) * 4 * 4;
            bufferCreateInfo.usageFlags = RHI::BufferUsage::Uniform;
            m_uniformBuffer = m_bufferPool->Allocate(bufferCreateInfo).GetValue();
        }
    }

    // create font texture atlas
    {
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        RHI::ImageCreateInfo imageInfo{};
        imageInfo.size.width = width;
        imageInfo.size.height = height;
        imageInfo.size.depth = 1;
        imageInfo.type = RHI::ImageType::Image2D;
        imageInfo.format = RHI::Format::RGBA8_UNORM;
        imageInfo.usageFlags = RHI::ImageUsage::ShaderResource | RHI::ImageUsage::CopyDst;
        imageInfo.arrayCount = 1;
        m_image = m_imagePool->Allocate(imageInfo).GetValue();

        RHI::ImageViewCreateInfo useInfo{};
        useInfo.subresource.imageAspects = RHI::ImageAspect::Color;
        m_imageView = m_context->CreateImageView(m_image, useInfo);

        RHI::BufferCreateInfo stagingBufferCreateInfo{};
        stagingBufferCreateInfo.usageFlags = RHI::BufferUsage::CopySrc;
        stagingBufferCreateInfo.byteSize = 64 * RHI::AllocationSizeConstants::MB;
        auto stagingBuffer = m_bufferPool->Allocate(stagingBufferCreateInfo).GetValue();

        auto stagingBufferPtr = m_bufferPool->MapBuffer(stagingBuffer);
        memcpy(stagingBufferPtr, pixels, width * height * 4);
        m_bufferPool->UnmapBuffer(stagingBuffer);

        RHI::CopyBufferToImageDescriptor copyInfo{};
        copyInfo.srcBuffer = stagingBuffer;
        copyInfo.srcOffset = 0;
        copyInfo.srcSize.width = width;
        copyInfo.srcSize.height = height;
        copyInfo.srcSize.depth = 1;
        copyInfo.dstImage = m_image;
        copyInfo.dstSubresource.imageAspects = RHI::ImageAspect::Color;
        RHI::CommandList* commandList = commandListAllocator->Allocate();
        commandList->Begin();
        commandList->Submit(copyInfo);
        commandList->End();

        auto fence = m_context->CreateFence();
        scheduler->ExecuteCommandList(commandList, *fence);
        fence->Wait();

        delete pixels;
        m_bufferPool->FreeBuffer(stagingBuffer);
    }

    {
        m_bindGroup = bindGroupAllocator->AllocateBindGroups(m_bindGroupLayout).front();
        RHI::BindGroupData data{};
        data.BindBuffers(0, m_uniformBuffer);
        data.BindSamplers(1, m_sampler);
        data.BindImages(2, m_imageView);
        bindGroupAllocator->Update(m_bindGroup, data);
    }

    {
        RHI::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{ m_bindGroupLayout };
        m_pipelineLayout = m_context->CreatePipelineLayout(pipelineLayoutCreateInfo);

        RHI::ShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.code = (void*) shaderModuleBlob.data();
        shaderModuleCreateInfo.size = shaderModuleBlob.size();
        auto shaderModule = m_context->CreateShaderModule(shaderModuleCreateInfo);

        RHI::GraphicsPipelineCreateInfo createInfo{};
        createInfo.pixelShaderName = "PSMain";
        createInfo.vertexShaderName = "VSMain";
        createInfo.pixelShaderModule = shaderModule.get();
        createInfo.vertexShaderModule = shaderModule.get();
        createInfo.layout = m_pipelineLayout;
        createInfo.inputAssemblerState = {
            .bindings{ {
                .binding = 0,
                .stride = sizeof(ImDrawVert),
                .stepRate = RHI::PipelineVertexInputRate::PerVertex,
            } },
            .attributes{
                {
                    .location = 0,
                    .binding = 0,
                    .format = RHI::Format::RG32_FLOAT,
                    .offset = offsetof(ImDrawVert, pos),
                },
                {
                    .location = 1,
                    .binding = 0,
                    .format = RHI::Format::RG32_FLOAT,
                    .offset = offsetof(ImDrawVert, uv),
                },
                {
                    .location = 2,
                    .binding = 0,
                    .format = RHI::Format::RGBA8_UNORM,
                    .offset = offsetof(ImDrawVert, col),
                },
            }
        };
        createInfo.renderTargetLayout.colorAttachmentsFormats = RHI::Format::BGRA8_UNORM;
        createInfo.renderTargetLayout.depthAttachmentFormat = RHI::Format::D32;
        createInfo.topologyMode = RHI::PipelineTopologyMode::Triangles;
        createInfo.depthStencilState.depthTestEnable = false;
        createInfo.depthStencilState.depthWriteEnable = true;
        m_pipeline = m_context->CreateGraphicsPipeline(createInfo);
    }
}

void ImGuiRenderer::Shutdown()
{
    m_bindGroupAllocator->Free(m_bindGroup);
    m_imagePool->FreeImage(m_image);
    m_context->DestroyImageView(m_imageView);
    m_context->DestroySampler(m_sampler);
    m_bufferPool->FreeBuffer(m_indexBuffer);
    m_bufferPool->FreeBuffer(m_vertexBuffer);
    m_bufferPool->FreeBuffer(m_uniformBuffer);
}

void ImGuiRenderer::NewFrame()
{
}

void ImGuiRenderer::RenderDrawData(ImDrawData* drawData, RHI::CommandList& commandList)
{
    UpdateBuffers(drawData);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int globalIdxOffset = 0;
    int globalVtxOffset = 0;
    ImVec2 clip_off = drawData->DisplayPos;
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmdList = drawData->CmdLists[n];
        for (int i = 0; i < cmdList->CmdBuffer.Size; i++)
        {
            const ImDrawCmd* pcmd = &cmdList->CmdBuffer[i];

            // Project scissor/clipping rectangles into framebuffer space
            ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
            ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
            if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                continue;

            // RHI::Viewport viewport{};
            // viewport.width = drawData->DisplaySize.x;
            // viewport.height = drawData->DisplaySize.y;
            // viewport.minDepth = 0.0f;
            // viewport.maxDepth = 1.0f;
            // viewport.offsetX = viewport.offsetY = 0;
            // RHI::Scissor scissor{};
            // scissor.offsetX = int32_t(clip_min.x);
            // scissor.offsetY = int32_t(clip_min.y);
            // scissor.width = uint32_t(clip_max.x);
            // scissor.height = uint32_t(clip_max.y);

            // commandList.SetSicssor(scissor);
            // commandList.SetViewport(viewport);

            // Bind texture, Draw
            RHI::CommandDraw draw{};
            draw.bindGroups = m_bindGroup;
            draw.pipelineState = m_pipeline;
            draw.indexBuffers = m_indexBuffer;
            draw.vertexBuffers = m_vertexBuffer;
            draw.parameters.elementCount = pcmd->ElemCount;
            draw.parameters.vertexOffset = pcmd->VtxOffset + globalVtxOffset;
            draw.parameters.firstElement = pcmd->IdxOffset + globalIdxOffset;
            commandList.Submit(draw);
        }

        globalIdxOffset += cmdList->IdxBuffer.Size;
        globalVtxOffset += cmdList->VtxBuffer.Size;
    }
}

void ImGuiRenderer::UpdateBuffers(ImDrawData* drawData)
{
    if (!m_vertexBuffer || m_bufferPool->GetSize(m_vertexBuffer) < uint32_t(drawData->TotalVtxCount))
    {
        if (m_vertexBuffer)
            m_bufferPool->FreeBuffer(m_vertexBuffer);

        RHI::BufferCreateInfo createInfo{};
        createInfo.byteSize = drawData->TotalVtxCount + 5000;
        createInfo.usageFlags = RHI::BufferUsage::Vertex;
        m_vertexBuffer = m_bufferPool->Allocate(createInfo).GetValue();
    }

    if (!m_indexBuffer || m_bufferPool->GetSize(m_indexBuffer) < uint32_t(drawData->TotalIdxCount))
    {
        if (m_indexBuffer)
            m_bufferPool->FreeBuffer(m_indexBuffer);

        RHI::BufferCreateInfo createInfo{};
        createInfo.byteSize = drawData->TotalIdxCount + 5000;
        createInfo.usageFlags = RHI::BufferUsage::Index;
        m_indexBuffer = m_bufferPool->Allocate(createInfo).GetValue();
    }

    auto indexBufferPtr = (ImDrawVert*)m_bufferPool->MapBuffer(m_indexBuffer);
    auto vertexBufferPtr = (ImDrawIdx*)m_bufferPool->MapBuffer(m_vertexBuffer);
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmdList = drawData->CmdLists[n];
        memcpy(indexBufferPtr, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
        memcpy(vertexBufferPtr, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
        indexBufferPtr += cmdList->IdxBuffer.Size;
        vertexBufferPtr += cmdList->VtxBuffer.Size;
    }
    m_bufferPool->UnmapBuffer(m_vertexBuffer);
    m_bufferPool->UnmapBuffer(m_indexBuffer);

    {
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
        auto uniformBufferPtr = m_bufferPool->MapBuffer(m_uniformBuffer);
        memcpy(uniformBufferPtr, mvp, sizeof(mvp));
        m_bufferPool->UnmapBuffer(m_uniformBuffer);
    }
}