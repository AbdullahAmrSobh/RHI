#include "Examples-Base/ImGuiRenderer.hpp"
#include "Examples-Base/Event.hpp"

#include <RHI/Common/Hash.hpp>

namespace Examples
{
    inline static ImGuiKey ConvertToImguiKeycode(KeyCode key)
    {
        switch (key)
        {
        case KeyCode::Tab:          return ImGuiKey_Tab;
        case KeyCode::Left:         return ImGuiKey_LeftArrow;
        case KeyCode::Right:        return ImGuiKey_RightArrow;
        case KeyCode::Up:           return ImGuiKey_UpArrow;
        case KeyCode::Down:         return ImGuiKey_DownArrow;
        case KeyCode::PageUp:       return ImGuiKey_PageUp;
        case KeyCode::PageDown:     return ImGuiKey_PageDown;
        case KeyCode::Home:         return ImGuiKey_Home;
        case KeyCode::End:          return ImGuiKey_End;
        case KeyCode::Insert:       return ImGuiKey_Insert;
        case KeyCode::Delete:       return ImGuiKey_Delete;
        case KeyCode::Backspace:    return ImGuiKey_Backspace;
        case KeyCode::Space:        return ImGuiKey_Space;
        case KeyCode::Enter:        return ImGuiKey_Enter;
        case KeyCode::Escape:       return ImGuiKey_Escape;
        case KeyCode::Apostrophe:   return ImGuiKey_Apostrophe;
        case KeyCode::Comma:        return ImGuiKey_Comma;
        case KeyCode::Minus:        return ImGuiKey_Minus;
        case KeyCode::Period:       return ImGuiKey_Period;
        case KeyCode::Slash:        return ImGuiKey_Slash;
        case KeyCode::Semicolon:    return ImGuiKey_Semicolon;
        case KeyCode::Equal:        return ImGuiKey_Equal;
        case KeyCode::LeftBracket:  return ImGuiKey_LeftBracket;
        case KeyCode::Backslash:    return ImGuiKey_Backslash;
        case KeyCode::RightBracket: return ImGuiKey_RightBracket;
        case KeyCode::GraveAccent:  return ImGuiKey_GraveAccent;
        case KeyCode::CapsLock:     return ImGuiKey_CapsLock;
        case KeyCode::ScrollLock:   return ImGuiKey_ScrollLock;
        case KeyCode::NumLock:      return ImGuiKey_NumLock;
        case KeyCode::PrintScreen:  return ImGuiKey_PrintScreen;
        case KeyCode::Pause:        return ImGuiKey_Pause;
        case KeyCode::KP0:          return ImGuiKey_Keypad0;
        case KeyCode::KP1:          return ImGuiKey_Keypad1;
        case KeyCode::KP2:          return ImGuiKey_Keypad2;
        case KeyCode::KP3:          return ImGuiKey_Keypad3;
        case KeyCode::KP4:          return ImGuiKey_Keypad4;
        case KeyCode::KP5:          return ImGuiKey_Keypad5;
        case KeyCode::KP6:          return ImGuiKey_Keypad6;
        case KeyCode::KP7:          return ImGuiKey_Keypad7;
        case KeyCode::KP8:          return ImGuiKey_Keypad8;
        case KeyCode::KP9:          return ImGuiKey_Keypad9;
        case KeyCode::KPDecimal:    return ImGuiKey_KeypadDecimal;
        case KeyCode::KPDivide:     return ImGuiKey_KeypadDivide;
        case KeyCode::KPMultiply:   return ImGuiKey_KeypadMultiply;
        case KeyCode::KPSubtract:   return ImGuiKey_KeypadSubtract;
        case KeyCode::KPAdd:        return ImGuiKey_KeypadAdd;
        case KeyCode::KPEnter:      return ImGuiKey_KeypadEnter;
        case KeyCode::KPEqual:      return ImGuiKey_KeypadEqual;
        case KeyCode::LeftShift:    return ImGuiKey_LeftShift;
        case KeyCode::LeftControl:  return ImGuiKey_LeftCtrl;
        case KeyCode::LeftAlt:      return ImGuiKey_LeftAlt;
        case KeyCode::LeftSuper:    return ImGuiKey_LeftSuper;
        case KeyCode::RightShift:   return ImGuiKey_RightShift;
        case KeyCode::RightControl: return ImGuiKey_RightCtrl;
        case KeyCode::RightAlt:     return ImGuiKey_RightAlt;
        case KeyCode::RightSuper:   return ImGuiKey_RightSuper;
        case KeyCode::Menu:         return ImGuiKey_Menu;
        case KeyCode::D0:           return ImGuiKey_0;
        case KeyCode::D1:           return ImGuiKey_1;
        case KeyCode::D2:           return ImGuiKey_2;
        case KeyCode::D3:           return ImGuiKey_3;
        case KeyCode::D4:           return ImGuiKey_4;
        case KeyCode::D5:           return ImGuiKey_5;
        case KeyCode::D6:           return ImGuiKey_6;
        case KeyCode::D7:           return ImGuiKey_7;
        case KeyCode::D8:           return ImGuiKey_8;
        case KeyCode::D9:           return ImGuiKey_9;
        case KeyCode::A:            return ImGuiKey_A;
        case KeyCode::B:            return ImGuiKey_B;
        case KeyCode::C:            return ImGuiKey_C;
        case KeyCode::D:            return ImGuiKey_D;
        case KeyCode::E:            return ImGuiKey_E;
        case KeyCode::F:            return ImGuiKey_F;
        case KeyCode::G:            return ImGuiKey_G;
        case KeyCode::H:            return ImGuiKey_H;
        case KeyCode::I:            return ImGuiKey_I;
        case KeyCode::J:            return ImGuiKey_J;
        case KeyCode::K:            return ImGuiKey_K;
        case KeyCode::L:            return ImGuiKey_L;
        case KeyCode::M:            return ImGuiKey_M;
        case KeyCode::N:            return ImGuiKey_N;
        case KeyCode::O:            return ImGuiKey_O;
        case KeyCode::P:            return ImGuiKey_P;
        case KeyCode::Q:            return ImGuiKey_Q;
        case KeyCode::R:            return ImGuiKey_R;
        case KeyCode::S:            return ImGuiKey_S;
        case KeyCode::T:            return ImGuiKey_T;
        case KeyCode::U:            return ImGuiKey_U;
        case KeyCode::V:            return ImGuiKey_V;
        case KeyCode::W:            return ImGuiKey_W;
        case KeyCode::X:            return ImGuiKey_X;
        case KeyCode::Y:            return ImGuiKey_Y;
        case KeyCode::Z:            return ImGuiKey_Z;
        case KeyCode::F1:           return ImGuiKey_F1;
        case KeyCode::F2:           return ImGuiKey_F2;
        case KeyCode::F3:           return ImGuiKey_F3;
        case KeyCode::F4:           return ImGuiKey_F4;
        case KeyCode::F5:           return ImGuiKey_F5;
        case KeyCode::F6:           return ImGuiKey_F6;
        case KeyCode::F7:           return ImGuiKey_F7;
        case KeyCode::F8:           return ImGuiKey_F8;
        case KeyCode::F9:           return ImGuiKey_F9;
        case KeyCode::F10:          return ImGuiKey_F10;
        case KeyCode::F11:          return ImGuiKey_F11;
        case KeyCode::F12:          return ImGuiKey_F12;
        case KeyCode::F13:          return ImGuiKey_F13;
        case KeyCode::F14:          return ImGuiKey_F14;
        case KeyCode::F15:          return ImGuiKey_F15;
        case KeyCode::F16:          return ImGuiKey_F16;
        case KeyCode::F17:          return ImGuiKey_F17;
        case KeyCode::F18:          return ImGuiKey_F18;
        case KeyCode::F19:          return ImGuiKey_F19;
        case KeyCode::F20:          return ImGuiKey_F20;
        case KeyCode::F21:          return ImGuiKey_F21;
        case KeyCode::F22:          return ImGuiKey_F22;
        case KeyCode::F23:          return ImGuiKey_F23;
        case KeyCode::F24:          return ImGuiKey_F24;
        default:                    return ImGuiKey_None;
        }
    }

    inline static ImGuiMouseButton ConvertToImguiMouseButton(MouseCode button)
    {
        switch (button)
        {
        case MouseCode::Button0: return ImGuiMouseButton_Left;
        case MouseCode::Button1: return ImGuiMouseButton_Right;
        case MouseCode::Button2: return ImGuiMouseButton_Middle;
        default:                 return {};
        }
    }

    void ImGuiRenderer::ProcessEvent(Event& e)
    {
        ImGuiIO& io = ImGui::GetIO();

        if ((e.GetCategoryFlags() & EventCategory::Mouse) && io.WantCaptureMouse)
        {
            e.Handled = true;
        }

        if ((e.GetCategoryFlags() & EventCategory::Keyboard) && io.WantCaptureKeyboard)
        {
            e.Handled = true;
        }

        switch (e.GetEventType())
        {
        case EventType::WindowResize:
            {
                auto& event = (WindowResizeEvent&)e;
                io.DisplaySize.x = (float)event.GetSize().width;
                io.DisplaySize.y = (float)event.GetSize().height;
                io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
                break;
            }
        case EventType::KeyPressed:
            {
                auto& event = (KeyPressedEvent&)e;
                io.AddKeyEvent(ConvertToImguiKeycode(event.GetKeyCode()), true);
                break;
            }
        case EventType::KeyReleased:
            {
                auto& event = (KeyReleasedEvent&)e;
                io.AddKeyEvent(ConvertToImguiKeycode(event.GetKeyCode()), false);
                break;
            }
        case EventType::KeyTyped:
            {
                auto& event = (KeyTypedEvent&)e;
                io.AddInputCharacter(ConvertToImguiKeycode(event.GetKeyCode()));
                break;
            }
        case EventType::MouseButtonPressed:
            {
                auto& event = (MouseButtonPressedEvent&)e;
                io.AddMouseButtonEvent(ConvertToImguiMouseButton(event.GetMouseButton()), true);
                break;
            }
        case EventType::MouseButtonReleased:
            {
                auto& event = (MouseButtonReleasedEvent&)e;
                io.AddMouseButtonEvent(ConvertToImguiMouseButton(event.GetMouseButton()), false);
                break;
            }
        case EventType::MouseMoved:
            {
                auto& event = (MouseMovedEvent&)e;
                io.AddMousePosEvent(event.GetX(), event.GetY());
                break;
            }
        case EventType::MouseScrolled:
            {
                auto& event = (MouseScrolledEvent&)e;
                io.AddMouseWheelEvent(event.GetXOffset(), event.GetYOffset());
                break;
            }
        default: break;
        }
    }

    void ImGuiRenderer::Init(const ImGuiRenderer::CreateInfo& createInfo)
    {
        m_imguiContext = ImGui::CreateContext();
        ImGui::SetCurrentContext(m_imguiContext);

        ImGuiIO& io = ImGui::GetIO();
        IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

        m_context = createInfo.context;

        // create sampler state
        {
            RHI::SamplerCreateInfo samplerCI{};
            samplerCI.name = "ImGui-Sampler";
            samplerCI.filterMin = RHI::SamplerFilter::Linear;
            samplerCI.filterMag = RHI::SamplerFilter::Linear;
            samplerCI.filterMip = RHI::SamplerFilter::Linear;
            samplerCI.compare = RHI::SamplerCompareOperation::Always;
            samplerCI.mipLodBias = 0.0f;
            samplerCI.addressU = RHI::SamplerAddressMode::Repeat;
            samplerCI.addressV = RHI::SamplerAddressMode::Repeat;
            samplerCI.addressW = RHI::SamplerAddressMode::Repeat;
            samplerCI.minLod = 0.0f;
            samplerCI.maxLod = 1.0f;
            m_sampler = m_context->CreateSampler(samplerCI);
        }

        {
            RHI::BindGroupLayoutCreateInfo bindGroupLayoutCreateInfo{};
            bindGroupLayoutCreateInfo.bindings[0] = RHI::ShaderBinding{ .type = RHI::BindingType::UniformBuffer, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Vertex };
            bindGroupLayoutCreateInfo.bindings[1] = RHI::ShaderBinding{ .type = RHI::BindingType::Sampler, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel };
            bindGroupLayoutCreateInfo.bindings[2] = RHI::ShaderBinding{ .type = RHI::BindingType::SampledImage, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel };
            m_bindGroupLayout = m_context->CreateBindGroupLayout(bindGroupLayoutCreateInfo);

            {
                RHI::BufferCreateInfo bufferCreateInfo{};
                bufferCreateInfo.name = "ImGui-UniformBuffer";
                bufferCreateInfo.byteSize = sizeof(float) * 4 * 4;
                bufferCreateInfo.usageFlags = RHI::BufferUsage::Uniform;
                m_uniformBuffer = m_context->CreateBuffer(bufferCreateInfo).GetValue();
            }
        }

        // create font texture atlas
        {
            unsigned char* pixels;
            int width, height;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

            [[maybe_unused]] RHI::ImageCreateInfo imageInfo{};
            imageInfo.size.width = uint32_t(width);
            imageInfo.size.height = uint32_t(height);
            imageInfo.size.depth = 1;
            imageInfo.type = RHI::ImageType::Image2D;
            imageInfo.format = RHI::Format::RGBA8_UNORM;
            imageInfo.usageFlags = RHI::ImageUsage::ShaderResource;
            imageInfo.usageFlags |= RHI::ImageUsage::CopyDst;
            imageInfo.sampleCount = RHI::SampleCount::Samples1;
            imageInfo.arrayCount = 1;
            imageInfo.mipLevels = 1;

            // m_image = RHI::CreateImageWithData(*m_context, imageInfo, TL::Span<const uint8_t>{ pixels, size_t(width * height * 4) }).GetValue();
            RHI::ImageViewCreateInfo viewInfo{};
            viewInfo.image = m_image;
            viewInfo.viewType = RHI::ImageViewType::View2D;
            viewInfo.subresource.imageAspects = RHI::ImageAspect::Color;
            viewInfo.subresource.arrayCount = 1;
            viewInfo.subresource.mipLevelCount = 1;
            m_imageView = m_context->CreateImageView(viewInfo);
        }

        {
            m_bindGroup = m_context->CreateBindGroup(m_bindGroupLayout);
            TL::Span<const RHI::BindGroupUpdateInfo> bindings{
                RHI::BindGroupUpdateInfo(0, 0, m_uniformBuffer),
                RHI::BindGroupUpdateInfo(1, 0, m_sampler),
                RHI::BindGroupUpdateInfo(2, 0, m_imageView)
            };
            m_context->UpdateBindGroup(m_bindGroup, bindings);
        }

        {
            RHI::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{ m_bindGroupLayout };
            m_pipelineLayout = m_context->CreatePipelineLayout(pipelineLayoutCreateInfo);

            auto shaderModule = m_context->CreateShaderModule({ createInfo.shaderBlob.data(), createInfo.shaderBlob.size() });
            auto defaultBlendState = RHI::ColorAttachmentBlendStateDesc{
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
                // clang-format off
            .name = "ImGui Pipeline",
            .vertexShaderName = "VSMain",
            .vertexShaderModule = shaderModule.get(),
            .pixelShaderName = "PSMain",
            .pixelShaderModule = shaderModule.get(),
            .layout = m_pipelineLayout,
            .inputAssemblerState =
                {
                    .bindings
                    {
                        { .binding = 0, .stride = sizeof(ImDrawVert), .stepRate = RHI::PipelineVertexInputRate::PerVertex, }
                    },
                    .attributes
                    {
                        { .location = 0, .binding = 0, .format = RHI::Format::RG32_FLOAT, .offset = offsetof(ImDrawVert, pos), },
                        { .location = 1, .binding = 0, .format = RHI::Format::RG32_FLOAT, .offset = offsetof(ImDrawVert, uv), },
                        { .location = 2, .binding = 0, .format = RHI::Format::RGBA8_UNORM, .offset = offsetof(ImDrawVert, col), }
                    }
                },
            .renderTargetLayout =
                {
                    .colorAttachmentsFormats = { createInfo.renderTargetFormat },
                    .depthAttachmentFormat = RHI::Format::Unknown,
                    .stencilAttachmentFormat = RHI::Format::Unknown,
                },
            .colorBlendState =
                {
                    .blendStates = { defaultBlendState, defaultBlendState },
                    .blendConstants = {}
                },
            .topologyMode = RHI::PipelineTopologyMode::Triangles,
            .rasterizationState =
                {
                    .cullMode = RHI::PipelineRasterizerStateCullMode::None,
                    .fillMode = RHI::PipelineRasterizerStateFillMode::Triangle,
                    .frontFace = RHI::PipelineRasterizerStateFrontFace::CounterClockwise,
                    .lineWidth = 1.0,
                },
            .multisampleState =
                {
                    .sampleCount = RHI::SampleCount::Samples1,
                    .sampleShading = false,
                },
            .depthStencilState =
                {
                    .depthTestEnable = false,
                    .depthWriteEnable = true,
                    .compareOperator = RHI::CompareOperator::Always,
                    .stencilTestEnable = false,
                },
                // clang-format on
            };
            m_pipeline = m_context->CreateGraphicsPipeline(pipelineCI);
        }
    }

    void ImGuiRenderer::Shutdown()
    {
        m_context->DestroyPipelineLayout(m_pipelineLayout);
        m_context->DestroyGraphicsPipeline(m_pipeline);
        m_context->DestroyBindGroupLayout(m_bindGroupLayout);
        m_context->DestroyBindGroup(m_bindGroup);
        m_context->DestroyImageView(m_imageView);
        m_context->DestroySampler(m_sampler);
        m_context->DestroyImage(m_image);
        if (m_indexBuffer != RHI::NullHandle)
            m_context->DestroyBuffer(m_indexBuffer);
        if (m_vertexBuffer != RHI::NullHandle)
            m_context->DestroyBuffer(m_vertexBuffer);
        if (m_uniformBuffer != RHI::NullHandle)
            m_context->DestroyBuffer(m_uniformBuffer);
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

        // Will project scissor/clipping rectangles into framebuffer space
        ImVec2 clipOff = drawData->DisplayPos;         // (0,0) unless using multi-viewports
        ImVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

        uint32_t fb_width = 1600, fb_height = 900;

        for (int n = 0; n < drawData->CmdListsCount; n++)
        {
            const ImDrawList* cmdList = drawData->CmdLists[n];
            for (int i = 0; i < cmdList->CmdBuffer.Size; i++)
            {
                const ImDrawCmd* pcmd = &cmdList->CmdBuffer[i];

                if (pcmd->UserCallback)
                {
                    pcmd->UserCallback(cmdList, pcmd);
                }
                else
                {
                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec2 clip_min((pcmd->ClipRect.x - clipOff.x) * clipScale.x, (pcmd->ClipRect.y - clipOff.y) * clipScale.y);
                    ImVec2 clip_max((pcmd->ClipRect.z - clipOff.x) * clipScale.x, (pcmd->ClipRect.w - clipOff.y) * clipScale.y);

                    // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
                    if (clip_min.x < 0.0f)
                    {
                        clip_min.x = 0.0f;
                    }
                    if (clip_min.y < 0.0f)
                    {
                        clip_min.y = 0.0f;
                    }
                    if (clip_max.x > fb_width)
                    {
                        clip_max.x = (float)fb_width;
                    }
                    if (clip_max.y > fb_height)
                    {
                        clip_max.y = (float)fb_height;
                    }
                    if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                        continue;

                    // Apply scissor/clipping rectangle
                    RHI::Scissor scissor{};
                    scissor.offsetX = (int32_t)(clip_min.x);
                    scissor.offsetY = (int32_t)(clip_min.y);
                    scissor.width = (uint32_t)(clip_max.x - clip_min.x);
                    scissor.height = (uint32_t)(clip_max.y - clip_min.y);
                    commandList.SetSicssor(scissor);

                    // Bind texture, Draw
                    RHI::DrawInfo drawInfo{};
                    drawInfo.bindGroups = { m_bindGroup };
                    drawInfo.pipelineState = m_pipeline;
                    drawInfo.indexBuffer = m_indexBuffer;
                    drawInfo.vertexBuffers = { m_vertexBuffer };
                    drawInfo.parameters.elementsCount = pcmd->ElemCount;
                    drawInfo.parameters.vertexOffset = (int32_t)pcmd->VtxOffset + globalVtxOffset;
                    drawInfo.parameters.firstElement = pcmd->IdxOffset + globalIdxOffset;
                    commandList.Draw(drawInfo);
                }
            }

            globalIdxOffset += cmdList->IdxBuffer.Size;
            globalVtxOffset += cmdList->VtxBuffer.Size;
        }
    }

    void ImGuiRenderer::UpdateBuffers(ImDrawData* drawData)
    {
        // Avoid rendering when minimized
        if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
            return;

        if ((size_t)drawData->TotalVtxCount > m_vertexBufferSize)
        {
            m_vertexBufferSize = (size_t)drawData->TotalVtxCount + 5000;

            if (m_vertexBuffer != RHI::NullHandle)
                m_context->DestroyBuffer(m_vertexBuffer);

            RHI::BufferCreateInfo createInfo{};
            createInfo.name = "ImGui-VertexBuffer";
            createInfo.byteSize = m_vertexBufferSize * sizeof(ImDrawVert);
            createInfo.usageFlags = RHI::BufferUsage::Vertex;
            m_vertexBuffer = m_context->CreateBuffer(createInfo).GetValue();
        }

        if ((size_t)drawData->TotalIdxCount > m_indexBufferSize)
        {
            m_indexBufferSize = (size_t)drawData->TotalIdxCount + 10000;

            if (m_indexBuffer != RHI::NullHandle)
                m_context->DestroyBuffer(m_indexBuffer);

            RHI::BufferCreateInfo createInfo{};
            createInfo.name = "ImGui-IndexBuffer";
            createInfo.byteSize = m_indexBufferSize * sizeof(ImDrawIdx);
            createInfo.usageFlags = RHI::BufferUsage::Index;
            m_indexBuffer = m_context->CreateBuffer(createInfo).GetValue();
        }

        if (drawData->TotalIdxCount == 0 || drawData->TotalVtxCount == 0)
            return;

        auto indexBufferPtr = (ImDrawIdx*)m_context->MapBuffer(m_indexBuffer);
        auto vertexBufferPtr = (ImDrawVert*)m_context->MapBuffer(m_vertexBuffer);
        for (int n = 0; n < drawData->CmdListsCount; n++)
        {
            const ImDrawList* cmdList = drawData->CmdLists[n];
            memcpy(indexBufferPtr, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
            memcpy(vertexBufferPtr, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
            indexBufferPtr += cmdList->IdxBuffer.Size;
            vertexBufferPtr += cmdList->VtxBuffer.Size;
        }
        m_context->UnmapBuffer(m_vertexBuffer);
        m_context->UnmapBuffer(m_indexBuffer);

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
            auto uniformBufferPtr = m_context->MapBuffer(m_uniformBuffer);
            memcpy(uniformBufferPtr, mvp, sizeof(mvp));
            m_context->UnmapBuffer(m_uniformBuffer);
        }
    }
} // namespace Examples