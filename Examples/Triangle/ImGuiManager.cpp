#include "ImGuiManager.hpp"

#include "Examples-Base/Window.hpp"

#include <TL/UniquePtr.hpp>

using namespace Engine;

namespace Engine
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

    inline static Window* GetWindowFromViewport(ImGuiViewport* vp)
    {
        if (vp->PlatformUserData)
        {
            return static_cast<Window*>(vp->PlatformUserData);
        }
        return nullptr;
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
        case WindowEventType::Moved:     break;
        case WindowEventType::CursorMoved:
            io.AddMousePosEvent(e.cursorPosition.x, e.cursorPosition.y);
            return io.WantCaptureMouse;
        case WindowEventType::MouseScrolled:
            io.AddMouseWheelEvent(e.cursorPosition.x, e.cursorPosition.y);
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

    // . . U . .  // Create a new platform window for the given viewport
    inline static void ImGuiPlatformIO_CreateWindow(ImGuiViewport* vp)
    {
        auto position = vp->Pos;
        auto size     = vp->Size;

        TL::Flags<WindowFlags> attributes;
        if (vp->Flags & ImGuiViewportFlags_NoDecoration)
            attributes |= WindowFlags::NoDecorations;
        auto window = WindowManager::CreateWindow({}, attributes, {(uint32_t)size.x, (uint32_t)size.y});
        window->SetPosition({position.x, position.y});

        // if (!(vp->Flags & ImGuiViewportFlags_NoInputs))
        {
            window->Subscribe([](const WindowEvent& e) -> bool
                {
                    return WindowEventsHandler(e);
                });
        }
    }

    // N . U . D  //
    inline static void ImGuiPlatformIO_DestroyWindow(ImGuiViewport* vp)
    {
        auto window = GetWindowFromViewport(vp);
    }

    // . . U . .  // Newly created windows are initially hidden so SetWindowPos/Size/Title can be called on them before showing the window
    inline static void ImGuiPlatformIO_ShowWindow(ImGuiViewport* vp)
    {
        auto window = GetWindowFromViewport(vp);
        window->Show();
    }

    // . . U . .  // Set platform window position (given the upper-left corner of client area)
    inline static void ImGuiPlatformIO_SetWindowPos(ImGuiViewport* vp, ImVec2 pos)
    {
        auto window = GetWindowFromViewport(vp);
        window->SetPosition(WindowPosition{pos.x, pos.y});
    }

    // N . . . .  //
    inline static ImVec2 ImGuiPlatformIO_GetWindowPos(ImGuiViewport* vp)
    {
        auto window = GetWindowFromViewport(vp);
        return ImVec2(window->GetPosition().x, window->GetPosition().y);
    }

    // . . U . .  // Set platform window client area size (ignoring OS decorations such as OS title bar etc.)
    inline static void ImGuiPlatformIO_SetWindowSize(ImGuiViewport* vp, ImVec2 size)
    {
        auto window = GetWindowFromViewport(vp);
        window->SetSize({(uint32_t)size.x, (uint32_t)size.y});
    }

    // N . . . .  // Get platform window client area size
    inline static ImVec2 ImGuiPlatformIO_GetWindowSize(ImGuiViewport* vp)
    {
        auto window = GetWindowFromViewport(vp);
        return ImVec2(window->GetSize().width, window->GetSize().height);
    }

    // N . . . .  // Move window to front and set input focus
    inline static void ImGuiPlatformIO_SetWindowFocus(ImGuiViewport* vp)
    {
        auto window = GetWindowFromViewport(vp);
        window->SetFocus();
    }

    // . . U . .  //
    inline static bool ImGuiPlatformIO_GetWindowFocus(ImGuiViewport* vp)
    {
        auto window = GetWindowFromViewport(vp);
        return window->IsFocused();
    }

    // N . . . .  // Get platform window minimized state. When minimized, we generally won't attempt to get/set size and contents will be culled more easily
    inline static bool ImGuiPlatformIO_GetWindowMinimized(ImGuiViewport* vp)
    {
        auto window = GetWindowFromViewport(vp);
        return !window->GetAttribute(WindowAttribute::Maximized);
    }

    // . . U . .  // Set platform window title (given an UTF-8 string)
    inline static void ImGuiPlatformIO_SetWindowTitle(ImGuiViewport* vp, const char* str)
    {
        auto window = GetWindowFromViewport(vp);
        window->SetTitle(str);
    }

    // (Optional) Setup global transparency (not per-pixel transparency)
    inline static void ImGuiPlatformIO_SetWindowAlpha(ImGuiViewport* vp, float alpha)
    {
        auto window = GetWindowFromViewport(vp);
        window->SetOpacity(alpha);
    }

    // . . U . .  // (Optional) Called by UpdatePlatformWindows(). Optional hook to allow the platform backend from doing general book-keeping every frame.
    inline static void ImGuiPlatformIO_UpdateWindow(ImGuiViewport* vp)
    {
        auto window = GetWindowFromViewport(vp);
        window->Poll();
    }

    // . . . R .  // (Optional) Main rendering (platform side! This is often unused, or just setting a "current" context for OpenGL bindings). 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    inline static void ImGuiPlatformIO_RenderWindow(ImGuiViewport* vp, void* render_arg)
    {
        auto window = GetWindowFromViewport(vp);
    }

    // . . . R .  // (Optional) Call Present/SwapBuffers (platform side! This is often unused!). 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    inline static void ImGuiPlatformIO_SwapBuffers(ImGuiViewport* vp, void* render_arg)
    {
        auto window = GetWindowFromViewport(vp);
    }

    // N . . . .  // (Optional) [BETA] FIXME-DPI: DPI handling: Return DPI scale for this viewport. 1.0f = 96 DPI.
    inline static float ImGuiPlatformIO_GetWindowDpiScale(ImGuiViewport* vp)
    {
        auto window = GetWindowFromViewport(vp);
        return 1.0f;
    }

    // . F . . .  // (Optional) [BETA] FIXME-DPI: DPI handling: Called during Begin() every time the viewport we are outputting into changes, so backend has a chance to swap fonts to adjust style.
    // inline static void ImGuiPlatformIO_OnChangedViewport(ImGuiViewport* vp)
    // {
    //     auto window = GetWindowFromViewport(vp);
    // }

    // inline static ImVec4 ImGuiPlatformIO_GetWindowWorkAreaInsets(ImGuiViewport* vp)
    // {
    //     auto window = GetWindowFromViewport(vp);

    //     return ImVec4(0, 0, 0, 0);
    // }

    // inline static int ImGuiPlatformIO_CreateVkSurface(ImGuiViewport* vp, ImU64 vk_inst, const void* vk_allocators, ImU64* out_vk_surface)
    // {
    //     auto window = GetWindowFromViewport(vp);

    //     return 0;
    // }

    // Renderer Backend functions (e.g. DirectX, OpenGL, Vulkan) ------------

    // Create swap chain, frame buffers etc. (called after Platform_CreateWindow)
    inline static void
    ImGui_Renderer_CreateWindow(ImGuiViewport* vp)
    {
    }

    // Destroy swap chain, frame buffers etc. (called before Platform_DestroyWindow)
    inline static void ImGui_Renderer_DestroyWindow(ImGuiViewport* vp)
    {
    }

    // Resize swap chain, frame buffers etc. (called after Platform_SetWindowSize)
    inline static void ImGui_Renderer_SetWindowSize(ImGuiViewport* vp, ImVec2 size)
    {
    }

    // (Optional) Clear framebuffer, setup render target, then render the viewport->DrawData. 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    inline static void ImGui_Renderer_RenderWindow(ImGuiViewport* vp, void* render_arg)
    {
    }

    // (Optional) Call Present/SwapBuffers. 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    inline static void ImGui_Renderer_SwapBuffers(ImGuiViewport* vp, void* render_arg)
    {
    }

    void ImGuiManager::Init(Window* primaryWindow)
    {
        m_primaryWindow = primaryWindow;

        m_imguiContext       = ImGui::CreateContext();
        auto [width, height] = m_primaryWindow->GetSize();
        ImGuiIO& io          = ImGui::GetIO();
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.DisplaySize.x = float(width);
        io.DisplaySize.y = float(height);

        primaryWindow->Subscribe([](const WindowEvent& e) -> bool
            {
                return WindowEventsHandler(e);
            });

        // Update ImGui platform IO
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
        // platformIO.Platform_OnChangedViewport       = ImGuiPlatformIO_OnChangedViewport;
        // platformIO.Platform_GetWindowWorkAreaInsets = ImGuiPlatformIO_GetWindowWorkAreaInsets;
        // platformIO.Platform_CreateVkSurface         = ImGuiPlatformIO_CreateVkSurface;
        platformIO.Renderer_CreateWindow       = ImGui_Renderer_CreateWindow;
        platformIO.Renderer_DestroyWindow      = ImGui_Renderer_DestroyWindow;
        platformIO.Renderer_SetWindowSize      = ImGui_Renderer_SetWindowSize;
        platformIO.Renderer_RenderWindow       = ImGui_Renderer_RenderWindow;
        platformIO.Renderer_SwapBuffers        = ImGui_Renderer_SwapBuffers;
    }

    void ImGuiManager::Shutdown()
    {
        ImGui::DestroyContext(m_imguiContext);
    }

} // namespace Engine