#include "ImGuiPass.hpp"

#include <TL/Allocator/MemPlumber.hpp>
#include <TL/Defer.hpp>
#include <TL/FileSystem/FileSystem.hpp>
#include <TL/Log.hpp>
#include <TL/Utils.hpp>

#include <algorithm>

#include "Examples-Base/Event.hpp"
#include "Examples-Base/Window.hpp"

using namespace Examples;

namespace Engine
{
    inline static RHI::ShaderModule* LoadShaderModule(RHI::Device* device, const char* path)
    {
        auto code   = TL::ReadBinaryFile(path);
        // NOTE: Code might not be correctly aligned here?
        auto module = device->CreateShaderModule({
            .name = path,
            .code = {(uint32_t*)code.ptr, code.size / 4},
        });
        TL::Release(code, 1);
        return module;
    }

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

    void ImGuiPass::ProcessEvent(Event& e)
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
                auto& event                = (WindowResizeEvent&)e;
                io.DisplaySize.x           = (float)event.GetSize().width;
                io.DisplaySize.y           = (float)event.GetSize().height;
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

    RHI::RGPass* ImGuiPass::AddPass(RHI::RenderGraph* rg, RHI::Handle<RHI::RGImage>& outAttachment, ImDrawData* drawData)
    {
        UpdateBuffers(drawData);

        return rg->AddPass({
            .name          = "ImGui",
            .type          = RHI::PassType::Graphics,
            .size          = rg->GetFrameSize(),
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                builder.AddColorAttachment({.color = outAttachment, .loadOp = RHI::LoadOperation::Discard});
            },
            .executeCallback = [this, drawData, &rg](RHI::CommandList& commandList)
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