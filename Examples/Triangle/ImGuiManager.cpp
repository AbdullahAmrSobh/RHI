#include "ImGuiManager.hpp"

#include "Examples-Base/Window.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/RendererImpl/ImGuiPass.hpp"

#include <TL/UniquePtr.hpp>

namespace Engine
{
    static uint32_t window_count = 0;

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

    inline static bool WindowEventsHandler(const WindowEvent& e)
    {
        auto& io = ImGui::GetIO();
        switch (e.type)
        {
        case WindowEventType::Resized:
            io.DisplaySize.x           = (float)e.size.width;
            io.DisplaySize.y           = (float)e.size.height;
            io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
            break;
        case WindowEventType::Focused:   io.AddFocusEvent(true); break;
        case WindowEventType::Unfocused: io.AddFocusEvent(false); break;
        case WindowEventType::Closed:    break;
        case WindowEventType::Moved:
        case WindowEventType::CursorMoved:
        {
            float x = e.cursorPosition.x;
            float y = e.cursorPosition.y;
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                x += e.window->GetPosition().x;
                y += e.window->GetPosition().y;
            }
            io.AddMousePosEvent(x, y);
            return io.WantCaptureMouse;
        }
        case WindowEventType::MouseScrolled:
            io.AddMouseWheelEvent(e.scrolled.x, e.scrolled.y);
            return io.WantCaptureMouse;
        case WindowEventType::MouseInput:
            switch (e.mouseInput.state)
            {
            case KeyState::Press:
            case KeyState::Repeat:  io.AddMouseButtonEvent(ConvertToImguiMouseButton(e.mouseInput.code), true); break;
            case KeyState::Release: io.AddMouseButtonEvent(ConvertToImguiMouseButton(e.mouseInput.code), false); break;
            default:                break;
            }
            return io.WantCaptureMouse;
        case WindowEventType::KeyInput:
            switch (e.keyInput.state)
            {
            case KeyState::Press:
            case KeyState::Repeat:  io.AddKeyEvent(ConvertToImguiKeycode(e.keyInput.code), true); break;
            case KeyState::Release: io.AddKeyEvent(ConvertToImguiKeycode(e.keyInput.code), false); break;
            default:                break;
            }
            return io.WantCaptureKeyboard;
        case WindowEventType::KeyTyped:
            io.AddInputCharacter(e.keyCode);
            return io.WantCaptureKeyboard;
        }
        return false; // event not captured
    }

    struct ImGuiPlatformWindowData
    {
        ImGuiPlatformWindowData() { window_count ++; };
        ~ImGuiPlatformWindowData() { window_count --; };

        uint32_t id = window_count + 1;
        Window* window = nullptr;
    };

    struct ImGuiPlatformRenderData
    {
        RHI::Swapchain* swapchain;
    };

    inline static ImGuiPlatformWindowData* GetViewportWindowData(ImGuiViewport* vp)
    {
        TL_ASSERT(vp->PlatformUserData != nullptr);
        if (vp->PlatformUserData)
        {
            return (ImGuiPlatformWindowData*)vp->PlatformUserData;
        }
        return nullptr;
    }

    inline static ImGuiPlatformRenderData* GetViewportRenderData(ImGuiViewport* vp)
    {
        TL_ASSERT(vp->PlatformUserData != nullptr);
        if (vp->PlatformUserData)
        {
            return (ImGuiPlatformRenderData*)vp->PlatformUserData;
        }
        return nullptr;
    }

    inline static void ImGuiPlatformIO_CreateWindow(ImGuiViewport* vp)
    {
        TL_LOG_INFO("ImGuiPlatformIO_CreateWindow: ");

        auto position = vp->Pos;
        auto size     = vp->Size;

        auto windowData    = TL::Construct<ImGuiPlatformWindowData>();
        windowData->window = WindowManager::CreateWindow("", WindowFlags::NoDecorations, {(uint32_t)size.x, (uint32_t)size.y});
        windowData->window->SetPosition({position.x, position.y});
        windowData->window->SetSize({(uint32_t)size.x, (uint32_t)size.y});
        windowData->window->Subscribe([](const WindowEvent& e) -> bool
            {
                return WindowEventsHandler(e);
            });

        vp->PlatformUserData      = windowData;
    }

    inline static void ImGuiPlatformIO_DestroyWindow(ImGuiViewport* vp)
    {
        auto windowData = GetViewportWindowData(vp);
        if (windowData && windowData->window)
        {
            WindowManager::DestroyWindow(windowData->window);
            TL::Destruct(windowData);
            vp->PlatformUserData = nullptr;
        }
    }

    inline static void ImGuiPlatformIO_ShowWindow(ImGuiViewport* vp)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        windowData->window->Show();
    }

    inline static void ImGuiPlatformIO_SetWindowPos(ImGuiViewport* vp, ImVec2 pos)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        windowData->window->SetPosition(WindowPosition{pos.x, pos.y});
    }

    inline static ImVec2 ImGuiPlatformIO_GetWindowPos(ImGuiViewport* vp)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        auto [x, y]     = windowData->window->GetPosition();
        return ImVec2(x, y);
    }

    inline static void ImGuiPlatformIO_SetWindowSize(ImGuiViewport* vp, ImVec2 size)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        windowData->window->SetSize({(uint32_t)size.x, (uint32_t)size.y});
    }

    inline static ImVec2 ImGuiPlatformIO_GetWindowSize(ImGuiViewport* vp)
    {
        auto windowData      = GetViewportWindowData(vp);
        auto renderData      = GetViewportRenderData(vp);
        auto [width, height] = windowData->window->GetSize();
        return ImVec2(width, height);
    }

    inline static void ImGuiPlatformIO_SetWindowFocus(ImGuiViewport* vp)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        windowData->window->SetFocus();
    }

    inline static bool ImGuiPlatformIO_GetWindowFocus(ImGuiViewport* vp)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        return windowData->window->IsFocused();
    }

    inline static bool ImGuiPlatformIO_GetWindowMinimized(ImGuiViewport* vp)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        return windowData->window->GetAttribute(WindowAttribute::Maximized); // == false // @FIXME!
    }

    inline static void ImGuiPlatformIO_SetWindowTitle(ImGuiViewport* vp, const char* str)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        windowData->window->SetTitle(str);
    }

    inline static void ImGuiPlatformIO_SetWindowAlpha(ImGuiViewport* vp, float alpha)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        windowData->window->SetOpacity(alpha);
    }

    inline static void ImGuiPlatformIO_UpdateWindow(ImGuiViewport* vp)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        windowData->window->Poll();
    }

    inline static void ImGuiPlatformIO_RenderWindow(ImGuiViewport* vp, void* render_arg)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
    }

    inline static void ImGuiPlatformIO_SwapBuffers(ImGuiViewport* vp, void* render_arg)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
    }

    inline static float ImGuiPlatformIO_GetWindowDpiScale(ImGuiViewport* vp)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        return 1.0f;
    }

    inline static void ImGuiRenderer_CreateWindow(ImGuiViewport* vp)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        auto device     = Renderer::ptr->GetDevice();

        RHI::SwapchainCreateInfo swapchainCI{};
        swapchainCI.win32Window.hwnd = windowData->window->GetNativeHandle(),
        renderData->swapchain        = device->CreateSwapchain(swapchainCI);

        auto [width, height] = windowData->window->GetSize();
        RHI::SwapchainConfigureInfo config{
            .size        = {height},
            .imageCount  = 1,
            .imageUsage  = RHI::ImageUsage::Color,
            .format      = RHI::Format::RGBA8_UNORM,
            .presentMode = RHI::SwapchainPresentMode::Fifo,
            .alphaMode   = RHI::SwapchainAlphaMode::None};
        auto result = renderData->swapchain->Configure(config);
        TL_ASSERT(RHI::IsSuccess(result));
    }

    inline static void ImGuiRenderer_DestroyWindow(ImGuiViewport* vp)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        auto device     = Renderer::ptr->GetDevice();
        device->DestroySwapchain(renderData->swapchain);
    }

    inline static void ImGuiRenderer_SetWindowSize(ImGuiViewport* vp, ImVec2 size)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);
        auto result     = renderData->swapchain->Resize({(uint32_t)size.x, (uint32_t)size.y});
        TL_ASSERT(RHI::IsSuccess(result));
    }

    inline static void ImGuiRenderer_RenderWindow(ImGuiViewport* vp, void* render_arg)
    {
        auto       windowData = GetViewportWindowData(vp);
        auto       renderData = GetViewportRenderData(vp);
        auto       rg         = Renderer::ptr->GetRenderGraph();
        ImGuiPass* pass       = (ImGuiPass*)render_arg;

        auto swapchainBackbuffer = rg->ImportSwapchain("imgui-vp", *renderData->swapchain, RHI::Format::RGBA8_UNORM);
        pass->AddPass(rg, swapchainBackbuffer, vp->DrawData);
    }

    inline static void ImGuiRenderer_SwapBuffers(ImGuiViewport* vp, void* render_arg)
    {
        auto windowData = GetViewportWindowData(vp);
        auto renderData = GetViewportRenderData(vp);

        auto present = renderData->swapchain->Present();
        if (present == RHI::ResultCode::ErrorOutdated || present == RHI::ResultCode::ErrorOutdated)
        {
            TL_UNREACHABLE();
        }
        else if (TL::IsError(present))
        {
            TL_UNREACHABLE();
        }
    }

    void ImGuiManager::Init(Window* primaryWindow)
    {
        auto device     = Renderer::ptr->GetDevice();
        m_primaryWindow = primaryWindow;
        m_imguiContext  = ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
        io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports | ImGuiBackendFlags_RendererHasViewports;

        auto [width, height] = m_primaryWindow->GetSize();
        io.DisplaySize.x     = float(width);
        io.DisplaySize.y     = float(height);

        auto* windowData   = TL::Construct<ImGuiPlatformWindowData>();
        windowData->window = primaryWindow;

        ImGuiViewport* mainViewport = ImGui::GetMainViewport();
        mainViewport->Flags |= ImGuiViewportFlags_IsPlatformWindow | ImGuiViewportFlags_OwnedByApp;
        mainViewport->PlatformUserData      = windowData;
        // mainViewport->PlatformHandle        = primaryWindow;
        mainViewport->PlatformWindowCreated = true;

        primaryWindow->Subscribe([](const WindowEvent& e) -> bool
            {
                return WindowEventsHandler(e);
            });

        // Update ImGui platform IOs
        auto& platformIO                       = ImGui::GetPlatformIO();
        platformIO.Platform_CreateWindow       = ImGuiPlatformIO_CreateWindow;
        platformIO.Platform_DestroyWindow      = ImGuiPlatformIO_DestroyWindow;
        platformIO.Platform_ShowWindow         = ImGuiPlatformIO_ShowWindow;
        platformIO.Platform_SetWindowPos       = ImGuiPlatformIO_SetWindowPos;
        platformIO.Platform_GetWindowPos       = ImGuiPlatformIO_GetWindowPos;
        platformIO.Platform_SetWindowSize      = ImGuiPlatformIO_SetWindowSize;
        platformIO.Platform_GetWindowSize      = ImGuiPlatformIO_GetWindowSize;
        platformIO.Platform_SetWindowFocus     = ImGuiPlatformIO_SetWindowFocus;
        platformIO.Platform_GetWindowFocus     = ImGuiPlatformIO_GetWindowFocus;
        platformIO.Platform_GetWindowMinimized = ImGuiPlatformIO_GetWindowMinimized;
        platformIO.Platform_SetWindowTitle     = ImGuiPlatformIO_SetWindowTitle;
        platformIO.Platform_SetWindowAlpha     = ImGuiPlatformIO_SetWindowAlpha;
        platformIO.Platform_UpdateWindow       = ImGuiPlatformIO_UpdateWindow;
        platformIO.Platform_RenderWindow       = ImGuiPlatformIO_RenderWindow;
        platformIO.Platform_SwapBuffers        = ImGuiPlatformIO_SwapBuffers;
        platformIO.Platform_GetWindowDpiScale  = ImGuiPlatformIO_GetWindowDpiScale;
        platformIO.Renderer_CreateWindow       = ImGuiRenderer_CreateWindow;
        platformIO.Renderer_DestroyWindow      = ImGuiRenderer_DestroyWindow;
        platformIO.Renderer_SetWindowSize      = ImGuiRenderer_SetWindowSize;
        platformIO.Renderer_RenderWindow       = ImGuiRenderer_RenderWindow;
        platformIO.Renderer_SwapBuffers        = ImGuiRenderer_SwapBuffers;
        for (const auto& m : WindowManager::GetMonitors())
        {
            ImGuiPlatformMonitor monitor{};
            monitor.MainPos  = {m.GetPosition().x, m.GetPosition().y};
            monitor.MainSize = {(float)m.GetPhysicalSize().width, (float)m.GetPhysicalSize().height};
            // TODO: Implement later (if needed)
            monitor.WorkPos  = monitor.MainPos;
            monitor.WorkSize = monitor.MainSize;
            monitor.DpiScale = 1.0;
            platformIO.Monitors.push_back(monitor);
        }
    }

    void ImGuiManager::Shutdown()
    {
        ImGui::DestroyContext(m_imguiContext);
    }

} // namespace Engine