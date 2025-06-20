#include "ImGuiManager.hpp"

#include "Examples-Base/Event.hpp"
#include "Examples-Base/Window.hpp"

#include <TL/UniquePtr.hpp>

using namespace Examples;

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

    void ImGui_Platform_CreateWindow(ImGuiViewport* vp)
    {
    }

    void ImGui_Platform_DestroyWindow(ImGuiViewport* vp)
    {
    }

    void ImGui_Platform_ShowWindow(ImGuiViewport* vp)
    {

    }

    void ImGui_Platform_SetWindowPos(ImGuiViewport* vp, ImVec2 pos)
    {
    }

    ImVec2 ImGui_Platform_GetWindowPos(ImGuiViewport* vp)
    {
        return ImVec2(0, 0);
    }

    void ImGui_Platform_SetWindowSize(ImGuiViewport* vp, ImVec2 size)
    {
    }

    ImVec2 ImGui_Platform_GetWindowSize(ImGuiViewport* vp)
    {
        return ImVec2(0, 0);
    }

    void ImGui_Platform_SetWindowFocus(ImGuiViewport* vp)
    {
    }

    bool ImGui_Platform_GetWindowFocus(ImGuiViewport* vp)
    {
        return false;
    }

    bool ImGui_Platform_GetWindowMinimized(ImGuiViewport* vp)
    {
        return false;
    }

    void ImGui_Platform_SetWindowTitle(ImGuiViewport* vp, const char* str)
    {
    }

    void ImGui_Platform_SetWindowAlpha(ImGuiViewport* vp, float alpha)
    {
    }

    void ImGui_Platform_UpdateWindow(ImGuiViewport* vp)
    {
    }

    void ImGui_Platform_RenderWindow(ImGuiViewport* vp, void* render_arg)
    {
    }

    void ImGui_Platform_SwapBuffers(ImGuiViewport* vp, void* render_arg)
    {
    }

    float ImGui_Platform_GetWindowDpiScale(ImGuiViewport* vp)
    {
        return 1.0f;
    }

    void ImGui_Platform_OnChangedViewport(ImGuiViewport* vp)
    {
    }

    ImVec4 ImGui_Platform_GetWindowWorkAreaInsets(ImGuiViewport* vp)
    {
        return ImVec4(0, 0, 0, 0);
    }

    int ImGui_Platform_CreateVkSurface(ImGuiViewport* vp, ImU64 vk_inst, const void* vk_allocators, ImU64* out_vk_surface)
    {
        return 0;
    }

    // Renderer Backend functions (e.g. DirectX, OpenGL, Vulkan) ------------
    void ImGui_Renderer_CreateWindow(ImGuiViewport* vp)
    {
    }

    void ImGui_Renderer_DestroyWindow(ImGuiViewport* vp)
    {
    }

    void ImGui_Renderer_SetWindowSize(ImGuiViewport* vp, ImVec2 size)
    {
    }

    void ImGui_Renderer_RenderWindow(ImGuiViewport* vp, void* render_arg)
    {
    }

    void ImGui_Renderer_SwapBuffers(ImGuiViewport* vp, void* render_arg)
    {
    }

    void ImGuiManager::ProcessEvent(Event& e)
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

    void ImGuiManager::Init()
    {
        m_imguiContext = ImGui::CreateContext();

        // auto& platformIO                            = ImGui::GetPlatformIO();
        // platformIO.Platform_CreateWindow            = ImGui_Platform_CreateWindow;
        // platformIO.Platform_DestroyWindow           = ImGui_Platform_DestroyWindow;
        // platformIO.Platform_ShowWindow              = ImGui_Platform_ShowWindow;
        // platformIO.Platform_SetWindowPos            = ImGui_Platform_SetWindowPos;
        // platformIO.Platform_GetWindowPos            = ImGui_Platform_GetWindowPos;
        // platformIO.Platform_SetWindowSize           = ImGui_Platform_SetWindowSize;
        // platformIO.Platform_GetWindowSize           = ImGui_Platform_GetWindowSize;
        // platformIO.Platform_SetWindowFocus          = ImGui_Platform_SetWindowFocus;
        // platformIO.Platform_GetWindowFocus          = ImGui_Platform_GetWindowFocus;
        // platformIO.Platform_GetWindowMinimized      = ImGui_Platform_GetWindowMinimized;
        // platformIO.Platform_SetWindowTitle          = ImGui_Platform_SetWindowTitle;
        // platformIO.Platform_SetWindowAlpha          = ImGui_Platform_SetWindowAlpha;
        // platformIO.Platform_UpdateWindow            = ImGui_Platform_UpdateWindow;
        // platformIO.Platform_RenderWindow            = ImGui_Platform_RenderWindow;
        // platformIO.Platform_SwapBuffers             = ImGui_Platform_SwapBuffers;
        // platformIO.Platform_GetWindowDpiScale       = ImGui_Platform_GetWindowDpiScale;
        // platformIO.Platform_OnChangedViewport       = ImGui_Platform_OnChangedViewport;
        // platformIO.Platform_GetWindowWorkAreaInsets = ImGui_Platform_GetWindowWorkAreaInsets;
        // platformIO.Platform_CreateVkSurface         = ImGui_Platform_CreateVkSurface;
        // platformIO.Renderer_CreateWindow            = ImGui_Renderer_CreateWindow;
        // platformIO.Renderer_DestroyWindow           = ImGui_Renderer_DestroyWindow;
        // platformIO.Renderer_SetWindowSize           = ImGui_Renderer_SetWindowSize;
        // platformIO.Renderer_RenderWindow            = ImGui_Renderer_RenderWindow;
        // platformIO.Renderer_SwapBuffers             = ImGui_Renderer_SwapBuffers;
    }

    void ImGuiManager::Shutdown()
    {
        ImGui::DestroyContext(m_imguiContext);
    }

} // namespace Engine