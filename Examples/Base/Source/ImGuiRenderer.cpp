#include "Examples-Base/ImGuiRenderer.hpp"
#include <RHI/Common/Hash.hpp>

inline static bool ImGui_ImplGlfw_ShouldChainCallback(GLFWwindow* window)
{
    auto bd = (ImGuiRenderer*)glfwGetWindowUserPointer(window);
    return bd->m_callbacksChainForAllWindows ? true : (window == bd->m_window);
}

inline static ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int key)
{
    switch (key)
    {
    case GLFW_KEY_TAB:           return ImGuiKey_Tab;
    case GLFW_KEY_LEFT:          return ImGuiKey_LeftArrow;
    case GLFW_KEY_RIGHT:         return ImGuiKey_RightArrow;
    case GLFW_KEY_UP:            return ImGuiKey_UpArrow;
    case GLFW_KEY_DOWN:          return ImGuiKey_DownArrow;
    case GLFW_KEY_PAGE_UP:       return ImGuiKey_PageUp;
    case GLFW_KEY_PAGE_DOWN:     return ImGuiKey_PageDown;
    case GLFW_KEY_HOME:          return ImGuiKey_Home;
    case GLFW_KEY_END:           return ImGuiKey_End;
    case GLFW_KEY_INSERT:        return ImGuiKey_Insert;
    case GLFW_KEY_DELETE:        return ImGuiKey_Delete;
    case GLFW_KEY_BACKSPACE:     return ImGuiKey_Backspace;
    case GLFW_KEY_SPACE:         return ImGuiKey_Space;
    case GLFW_KEY_ENTER:         return ImGuiKey_Enter;
    case GLFW_KEY_ESCAPE:        return ImGuiKey_Escape;
    case GLFW_KEY_APOSTROPHE:    return ImGuiKey_Apostrophe;
    case GLFW_KEY_COMMA:         return ImGuiKey_Comma;
    case GLFW_KEY_MINUS:         return ImGuiKey_Minus;
    case GLFW_KEY_PERIOD:        return ImGuiKey_Period;
    case GLFW_KEY_SLASH:         return ImGuiKey_Slash;
    case GLFW_KEY_SEMICOLON:     return ImGuiKey_Semicolon;
    case GLFW_KEY_EQUAL:         return ImGuiKey_Equal;
    case GLFW_KEY_LEFT_BRACKET:  return ImGuiKey_LeftBracket;
    case GLFW_KEY_BACKSLASH:     return ImGuiKey_Backslash;
    case GLFW_KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket;
    case GLFW_KEY_GRAVE_ACCENT:  return ImGuiKey_GraveAccent;
    case GLFW_KEY_CAPS_LOCK:     return ImGuiKey_CapsLock;
    case GLFW_KEY_SCROLL_LOCK:   return ImGuiKey_ScrollLock;
    case GLFW_KEY_NUM_LOCK:      return ImGuiKey_NumLock;
    case GLFW_KEY_PRINT_SCREEN:  return ImGuiKey_PrintScreen;
    case GLFW_KEY_PAUSE:         return ImGuiKey_Pause;
    case GLFW_KEY_KP_0:          return ImGuiKey_Keypad0;
    case GLFW_KEY_KP_1:          return ImGuiKey_Keypad1;
    case GLFW_KEY_KP_2:          return ImGuiKey_Keypad2;
    case GLFW_KEY_KP_3:          return ImGuiKey_Keypad3;
    case GLFW_KEY_KP_4:          return ImGuiKey_Keypad4;
    case GLFW_KEY_KP_5:          return ImGuiKey_Keypad5;
    case GLFW_KEY_KP_6:          return ImGuiKey_Keypad6;
    case GLFW_KEY_KP_7:          return ImGuiKey_Keypad7;
    case GLFW_KEY_KP_8:          return ImGuiKey_Keypad8;
    case GLFW_KEY_KP_9:          return ImGuiKey_Keypad9;
    case GLFW_KEY_KP_DECIMAL:    return ImGuiKey_KeypadDecimal;
    case GLFW_KEY_KP_DIVIDE:     return ImGuiKey_KeypadDivide;
    case GLFW_KEY_KP_MULTIPLY:   return ImGuiKey_KeypadMultiply;
    case GLFW_KEY_KP_SUBTRACT:   return ImGuiKey_KeypadSubtract;
    case GLFW_KEY_KP_ADD:        return ImGuiKey_KeypadAdd;
    case GLFW_KEY_KP_ENTER:      return ImGuiKey_KeypadEnter;
    case GLFW_KEY_KP_EQUAL:      return ImGuiKey_KeypadEqual;
    case GLFW_KEY_LEFT_SHIFT:    return ImGuiKey_LeftShift;
    case GLFW_KEY_LEFT_CONTROL:  return ImGuiKey_LeftCtrl;
    case GLFW_KEY_LEFT_ALT:      return ImGuiKey_LeftAlt;
    case GLFW_KEY_LEFT_SUPER:    return ImGuiKey_LeftSuper;
    case GLFW_KEY_RIGHT_SHIFT:   return ImGuiKey_RightShift;
    case GLFW_KEY_RIGHT_CONTROL: return ImGuiKey_RightCtrl;
    case GLFW_KEY_RIGHT_ALT:     return ImGuiKey_RightAlt;
    case GLFW_KEY_RIGHT_SUPER:   return ImGuiKey_RightSuper;
    case GLFW_KEY_MENU:          return ImGuiKey_Menu;
    case GLFW_KEY_0:             return ImGuiKey_0;
    case GLFW_KEY_1:             return ImGuiKey_1;
    case GLFW_KEY_2:             return ImGuiKey_2;
    case GLFW_KEY_3:             return ImGuiKey_3;
    case GLFW_KEY_4:             return ImGuiKey_4;
    case GLFW_KEY_5:             return ImGuiKey_5;
    case GLFW_KEY_6:             return ImGuiKey_6;
    case GLFW_KEY_7:             return ImGuiKey_7;
    case GLFW_KEY_8:             return ImGuiKey_8;
    case GLFW_KEY_9:             return ImGuiKey_9;
    case GLFW_KEY_A:             return ImGuiKey_A;
    case GLFW_KEY_B:             return ImGuiKey_B;
    case GLFW_KEY_C:             return ImGuiKey_C;
    case GLFW_KEY_D:             return ImGuiKey_D;
    case GLFW_KEY_E:             return ImGuiKey_E;
    case GLFW_KEY_F:             return ImGuiKey_F;
    case GLFW_KEY_G:             return ImGuiKey_G;
    case GLFW_KEY_H:             return ImGuiKey_H;
    case GLFW_KEY_I:             return ImGuiKey_I;
    case GLFW_KEY_J:             return ImGuiKey_J;
    case GLFW_KEY_K:             return ImGuiKey_K;
    case GLFW_KEY_L:             return ImGuiKey_L;
    case GLFW_KEY_M:             return ImGuiKey_M;
    case GLFW_KEY_N:             return ImGuiKey_N;
    case GLFW_KEY_O:             return ImGuiKey_O;
    case GLFW_KEY_P:             return ImGuiKey_P;
    case GLFW_KEY_Q:             return ImGuiKey_Q;
    case GLFW_KEY_R:             return ImGuiKey_R;
    case GLFW_KEY_S:             return ImGuiKey_S;
    case GLFW_KEY_T:             return ImGuiKey_T;
    case GLFW_KEY_U:             return ImGuiKey_U;
    case GLFW_KEY_V:             return ImGuiKey_V;
    case GLFW_KEY_W:             return ImGuiKey_W;
    case GLFW_KEY_X:             return ImGuiKey_X;
    case GLFW_KEY_Y:             return ImGuiKey_Y;
    case GLFW_KEY_Z:             return ImGuiKey_Z;
    case GLFW_KEY_F1:            return ImGuiKey_F1;
    case GLFW_KEY_F2:            return ImGuiKey_F2;
    case GLFW_KEY_F3:            return ImGuiKey_F3;
    case GLFW_KEY_F4:            return ImGuiKey_F4;
    case GLFW_KEY_F5:            return ImGuiKey_F5;
    case GLFW_KEY_F6:            return ImGuiKey_F6;
    case GLFW_KEY_F7:            return ImGuiKey_F7;
    case GLFW_KEY_F8:            return ImGuiKey_F8;
    case GLFW_KEY_F9:            return ImGuiKey_F9;
    case GLFW_KEY_F10:           return ImGuiKey_F10;
    case GLFW_KEY_F11:           return ImGuiKey_F11;
    case GLFW_KEY_F12:           return ImGuiKey_F12;
    case GLFW_KEY_F13:           return ImGuiKey_F13;
    case GLFW_KEY_F14:           return ImGuiKey_F14;
    case GLFW_KEY_F15:           return ImGuiKey_F15;
    case GLFW_KEY_F16:           return ImGuiKey_F16;
    case GLFW_KEY_F17:           return ImGuiKey_F17;
    case GLFW_KEY_F18:           return ImGuiKey_F18;
    case GLFW_KEY_F19:           return ImGuiKey_F19;
    case GLFW_KEY_F20:           return ImGuiKey_F20;
    case GLFW_KEY_F21:           return ImGuiKey_F21;
    case GLFW_KEY_F22:           return ImGuiKey_F22;
    case GLFW_KEY_F23:           return ImGuiKey_F23;
    case GLFW_KEY_F24:           return ImGuiKey_F24;
    default:                     return ImGuiKey_None;
    }
}

static int ImGui_ImplGlfw_TranslateUntranslatedKey(int key, int scancode)
{
#if GLFW_HAS_GETKEYNAME && !defined(__EMSCRIPTEN__)
    // GLFW 3.1+ attempts to "untranslate" keys, which goes the opposite of what every other framework does, making using lettered shortcuts difficult.
    // (It had reasons to do so: namely GLFW is/was more likely to be used for WASD-type game controls rather than lettered shortcuts, but IHMO the 3.1 change could have been done differently)
    // See https://github.com/glfw/glfw/issues/1502 for details.
    // Adding a workaround to undo this (so our keys are translated->untranslated->translated, likely a lossy process).
    // This won't cover edge cases but this is at least going to cover common cases.
    if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_EQUAL)
        return key;
    GLFWerrorfun prev_error_callback = glfwSetErrorCallback(nullptr);
    const char* key_name = glfwGetKeyName(key, scancode);
    glfwSetErrorCallback(prev_error_callback);
    #if GLFW_HAS_GETERROR && !defined(__EMSCRIPTEN__) // Eat errors (see #5908)
    (void)glfwGetError(nullptr);
    #endif
    if (key_name && key_name[0] != 0 && key_name[1] == 0)
    {
        const char char_names[] = "`-=[]\\,;\'./";
        const int char_keys[] = { GLFW_KEY_GRAVE_ACCENT, GLFW_KEY_MINUS, GLFW_KEY_EQUAL, GLFW_KEY_LEFT_BRACKET, GLFW_KEY_RIGHT_BRACKET, GLFW_KEY_BACKSLASH, GLFW_KEY_COMMA, GLFW_KEY_SEMICOLON, GLFW_KEY_APOSTROPHE, GLFW_KEY_PERIOD, GLFW_KEY_SLASH, 0 };
        IM_ASSERT(IM_ARRAYSIZE(char_names) == IM_ARRAYSIZE(char_keys));
        if (key_name[0] >= '0' && key_name[0] <= '9')
        {
            key = GLFW_KEY_0 + (key_name[0] - '0');
        }
        else if (key_name[0] >= 'A' && key_name[0] <= 'Z')
        {
            key = GLFW_KEY_A + (key_name[0] - 'A');
        }
        else if (key_name[0] >= 'a' && key_name[0] <= 'z')
        {
            key = GLFW_KEY_A + (key_name[0] - 'a');
        }
        else if (const char* p = strchr(char_names, key_name[0]))
        {
            key = char_keys[p - char_names];
        }
    }
    // if (action == GLFW_PRESS) printf("key %d scancode %d name '%s'\n", key, scancode, key_name);
#else
    IM_UNUSED(scancode);
#endif
    return key;
}

// X11 does not include current pressed/released modifier key in 'mods' flags submitted by GLFW
// See https://github.com/ocornut/imgui/issues/6034 and https://github.com/glfw/glfw/issues/1630
inline static void ImGui_ImplGlfw_UpdateKeyModifiers(GLFWwindow* window)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl, (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS));
    io.AddKeyEvent(ImGuiMod_Shift, (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS));
    io.AddKeyEvent(ImGuiMod_Alt, (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS));
    io.AddKeyEvent(ImGuiMod_Super, (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS));
}

IMGUI_IMPL_API void ImGui_ImplGlfw_WindowFocusCallback(GLFWwindow* window, int focused)
{
    auto bd = (ImGuiRenderer*)glfwGetWindowUserPointer(window);
    if (bd->m_prevUserCallbackWindowFocus != nullptr && ImGui_ImplGlfw_ShouldChainCallback(window))
        bd->m_prevUserCallbackWindowFocus(window, focused);

    ImGuiIO& io = ImGui::GetIO();
    io.AddFocusEvent(focused != 0);
}

// Workaround: X11 seems to send spurious Leave/Enter events which would make us lose our position,
// so we back it up and restore on Leave/Enter (see https://github.com/ocornut/imgui/issues/4984)
IMGUI_IMPL_API void ImGui_ImplGlfw_CursorEnterCallback(GLFWwindow* window, int entered)
{
    auto bd = (ImGuiRenderer*)glfwGetWindowUserPointer(window);
    if (bd->m_prevUserCallbackCursorEnter != nullptr && ImGui_ImplGlfw_ShouldChainCallback(window))
        bd->m_prevUserCallbackCursorEnter(window, entered);

    // ImGuiIO& io = ImGui::GetIO();
    // if (entered)
    // {
    //     bd->m_mouseWindow = window;
    //     io.AddMousePosEvent(bd->m_lastValidMousePos.x, bd->m_lastValidMousePos.y);
    // }
    // else if (!entered && bd->m_mouseWindow == window)
    // {
    //     bd->m_lastValidMousePos = io.MousePos;
    //     bd->m_mouseWindow = nullptr;
    //     io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    // }
}

IMGUI_IMPL_API void ImGui_ImplGlfw_CursorPosCallback(GLFWwindow* window, double x, double y)
{
    auto bd = (ImGuiRenderer*)glfwGetWindowUserPointer(window);
    if (bd->m_prevUserCallbackCursorPos != nullptr && ImGui_ImplGlfw_ShouldChainCallback(window))
        bd->m_prevUserCallbackCursorPos(window, x, y);

    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent((float)x, (float)y);
    bd->m_lastValidMousePos = ImVec2((float)x, (float)y);
}

IMGUI_IMPL_API void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto bd = (ImGuiRenderer*)glfwGetWindowUserPointer(window);
    if (bd->m_prevUserCallbackScroll != nullptr && ImGui_ImplGlfw_ShouldChainCallback(window))
        bd->m_prevUserCallbackScroll(window, xoffset, yoffset);

#ifdef __EMSCRIPTEN__
    // Ignore GLFW events: will be processed in ImGui_ImplEmscripten_WheelCallback().
    return;
#endif

    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseWheelEvent((float)xoffset, (float)yoffset);
}

IMGUI_IMPL_API void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
    auto bd = (ImGuiRenderer*)glfwGetWindowUserPointer(window);
    if (bd->m_prevUserCallbackKey != nullptr && ImGui_ImplGlfw_ShouldChainCallback(window))
        bd->m_prevUserCallbackKey(window, keycode, scancode, action, mods);

    if (action != GLFW_PRESS && action != GLFW_RELEASE)
        return;

    ImGui_ImplGlfw_UpdateKeyModifiers(window);

    keycode = ImGui_ImplGlfw_TranslateUntranslatedKey(keycode, scancode);

    ImGuiIO& io = ImGui::GetIO();
    ImGuiKey imgui_key = ImGui_ImplGlfw_KeyToImGuiKey(keycode);
    io.AddKeyEvent(imgui_key, (action == GLFW_PRESS));
    io.SetKeyEventNativeData(imgui_key, keycode, scancode); // To support legacy indexing (<1.87 user code)
}

IMGUI_IMPL_API void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    auto bd = (ImGuiRenderer*)glfwGetWindowUserPointer(window);
    if (bd->m_prevUserCallbackMousebutton != nullptr && ImGui_ImplGlfw_ShouldChainCallback(window))
        bd->m_prevUserCallbackMousebutton(window, button, action, mods);

    ImGui_ImplGlfw_UpdateKeyModifiers(window);

    ImGuiIO& io = ImGui::GetIO();
    if (button >= 0 && button < ImGuiMouseButton_COUNT)
        io.AddMouseButtonEvent(button, action == GLFW_PRESS);
}

IMGUI_IMPL_API void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c)
{
    auto bd = (ImGuiRenderer*)glfwGetWindowUserPointer(window);
    if (bd->m_prevUserCallbackChar != nullptr && ImGui_ImplGlfw_ShouldChainCallback(window))
        bd->m_prevUserCallbackChar(window, c);

    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(c);
}

IMGUI_IMPL_API void ImGui_ImplGlfw_MonitorCallback(GLFWmonitor*, int)
{
    // Unused in 'master' branch but 'docking' branch will use this, so we declare it ahead of it so if you have to install callbacks you can install this one too.
}

void ImGuiRenderer::Init(ImGuiRendererCreateInfo createInfo)
{
    m_imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_imguiContext);

    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    m_context = createInfo.context;

    // create sampler state
    m_sampler = m_context->CreateSampler(RHI::SamplerCreateInfo{ RHI::SamplerFilter::Linear, RHI::SamplerAddressMode::Repeat });

    {
        RHI::BindGroupLayoutCreateInfo bindGroupLayoutCreateInfo{};
        bindGroupLayoutCreateInfo.bindings[0] = RHI::ShaderBinding{ .type = RHI::ShaderBindingType::UniformBuffer, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Vertex };
        bindGroupLayoutCreateInfo.bindings[1] = RHI::ShaderBinding{ .type = RHI::ShaderBindingType::Sampler, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel };
        bindGroupLayoutCreateInfo.bindings[2] = RHI::ShaderBinding{ .type = RHI::ShaderBindingType::SampledImage, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel };
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

        RHI::ImageCreateInfo imageInfo{};
        imageInfo.size.width = uint32_t(width);
        imageInfo.size.height = uint32_t(height);
        imageInfo.size.depth = 1;
        imageInfo.type = RHI::ImageType::Image2D;
        imageInfo.format = RHI::Format::RGBA8_UNORM;
        imageInfo.usageFlags = RHI::ImageUsage::ShaderResource;
        imageInfo.usageFlags |= RHI::ImageUsage::CopyDst;
        imageInfo.arrayCount = 1;
        m_image = RHI::CreateImageWithData(*m_context, imageInfo, RHI::TL::Span<const uint8_t>{ pixels, size_t(width * height * 4) }).GetValue();
        RHI::ImageViewCreateInfo viewInfo{};
        viewInfo.image = m_image;
        viewInfo.subresource.imageAspects = RHI::ImageAspect::Color;
        m_imageView = m_context->CreateImageView(viewInfo);
    }

    {
        m_bindGroup = m_context->CreateBindGroup(m_bindGroupLayout);
        RHI::BindGroupData data{};
        data.BindBuffers(0, m_uniformBuffer);
        data.BindSamplers(1, m_sampler);
        data.BindImages(2, m_imageView);
        m_context->UpdateBindGroup(m_bindGroup, data);
    }

    {
        RHI::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{ m_bindGroupLayout };
        m_pipelineLayout = m_context->CreatePipelineLayout(pipelineLayoutCreateInfo);

        auto shaderModule = m_context->CreateShaderModule(createInfo.shaderBlob);

        RHI::GraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.name = "ImGui-Pipeline";
        pipelineCreateInfo.pixelShaderName = "PSMain";
        pipelineCreateInfo.vertexShaderName = "VSMain";
        pipelineCreateInfo.pixelShaderModule = shaderModule.get();
        pipelineCreateInfo.vertexShaderModule = shaderModule.get();
        pipelineCreateInfo.layout = m_pipelineLayout;
        // clang-format off
        pipelineCreateInfo.inputAssemblerState = {
            .bindings{ { .binding = 0, .stride = sizeof(ImDrawVert), .stepRate = RHI::PipelineVertexInputRate::PerVertex, } },
            .attributes{ { .location = 0, .binding = 0, .format = RHI::Format::RG32_FLOAT, .offset = offsetof(ImDrawVert, pos), },
                         { .location = 1, .binding = 0, .format = RHI::Format::RG32_FLOAT, .offset = offsetof(ImDrawVert, uv), },
                         { .location = 2, .binding = 0, .format = RHI::Format::RGBA8_UNORM, .offset = offsetof(ImDrawVert, col), }} };
        // clang-format on
        pipelineCreateInfo.renderTargetLayout.colorAttachmentsFormats[0] = RHI::Format::BGRA8_UNORM;
        pipelineCreateInfo.renderTargetLayout.depthAttachmentFormat = RHI::Format::D32;
        pipelineCreateInfo.topologyMode = RHI::PipelineTopologyMode::Triangles;
        pipelineCreateInfo.depthStencilState.depthTestEnable = false;
        pipelineCreateInfo.depthStencilState.depthWriteEnable = true;
        pipelineCreateInfo.rasterizationState.cullMode = RHI::PipelineRasterizerStateCullMode::None;
        pipelineCreateInfo.colorBlendState.blendStates[0] = {
            true,
            RHI::BlendEquation::Add,
            RHI::BlendFactor::SrcAlpha,
            RHI::BlendFactor::OneMinusSrcAlpha,
            RHI::BlendEquation::Add,
            RHI::BlendFactor::One,
            RHI::BlendFactor::OneMinusSrcAlpha,
            RHI::ColorWriteMask::All,
        };
        m_pipeline = m_context->CreateGraphicsPipeline(pipelineCreateInfo);
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
                if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
                if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
                if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
                if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
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
                drawInfo.bindGroups = m_bindGroup;
                drawInfo.pipelineState = m_pipeline;
                drawInfo.indexBuffers = m_indexBuffer;
                drawInfo.vertexBuffers = m_vertexBuffer;
                drawInfo.parameters.elementCount = pcmd->ElemCount;
                drawInfo.parameters.vertexOffset = pcmd->VtxOffset + globalVtxOffset;
                drawInfo.parameters.firstElement = pcmd->IdxOffset + globalIdxOffset;
                commandList.Draw(drawInfo);
            }
        }

        globalIdxOffset += cmdList->IdxBuffer.Size;
        globalVtxOffset += cmdList->VtxBuffer.Size;
    }
}

void ImGuiRenderer::InstallGlfwCallbacks(GLFWwindow* window)
{
    RHI_ASSERT(m_installedCallbacks == false && "Callbacks already installed!");

    m_prevUserCallbackWindowFocus = glfwSetWindowFocusCallback(window, ImGui_ImplGlfw_WindowFocusCallback);
    m_prevUserCallbackCursorEnter = glfwSetCursorEnterCallback(window, ImGui_ImplGlfw_CursorEnterCallback);
    m_prevUserCallbackCursorPos = glfwSetCursorPosCallback(window, ImGui_ImplGlfw_CursorPosCallback);
    m_prevUserCallbackMousebutton = glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
    m_prevUserCallbackScroll = glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
    m_prevUserCallbackKey = glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
    m_prevUserCallbackChar = glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
    m_prevUserCallbackMonitor = glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);
    m_installedCallbacks = true;
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
        float mvp[4][4] =
        {
            { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
        };
        auto uniformBufferPtr = m_context->MapBuffer(m_uniformBuffer);
        memcpy(uniformBufferPtr, mvp, sizeof(mvp));
        m_context->UnmapBuffer(m_uniformBuffer);
    }
}