#define WINDOWS_LEAN_AND_MEAN
#include "Examples-Base/Window.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <TL/Assert.hpp>

namespace Engine
{
    TL::Error WindowManager::Init()
    {
        auto result = glfwInit();
        if (result == GLFW_FALSE)
        {
            return TL::Error("Failed to initialize GLFW");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        return TL::Error();
    }

    void WindowManager::Shutdown()
    {
        glfwTerminate();
    }

    // clang-format off
    // Thanks windows
    #ifdef CreateWindow
    #undef CreateWindow
    #endif
    // clang-format on

    Window* WindowManager::CreateWindow(TL::StringView title, TL::Flags<WindowFlags> flags, WindowSize size)
    {
        return TL::construct<Window>(title, flags, size);
    }

    void WindowManager::DestroyWindow(Window* window)
    {
        TL::destruct(window);
    }

    TL::Span<const Monitor> WindowManager::GetMonitors()
    {
        int           count        = 0;
        GLFWmonitor** glfwMonitors = glfwGetMonitors(&count);

        static TL::Vector<Monitor> s_monitors;
        s_monitors.clear();
        for (int i = 0; i < count; ++i)
        {
            Monitor monitor{};
            monitor.m_monitor = glfwMonitors[i];
            s_monitors.push_back(monitor);
        }
        return s_monitors;
    }

    /// Monitor

    void* Monitor::GetNativeHandle() const
    {
        return m_monitor;
    }

    WindowPosition Monitor::GetPosition() const
    {
        int x = 0, y = 0;
        glfwGetMonitorPos(m_monitor, &x, &y);
        return WindowPosition{(float)x, (float)y};
    }

    WindowSize Monitor::GetPhysicalSize() const
    {
        int width = 0, height = 0;
        glfwGetMonitorPhysicalSize(m_monitor, &width, &height);
        return WindowSize{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    WindowPosition Monitor::GetContentScale() const
    {
        float xscale = 1.0f, yscale = 1.0f;
        glfwGetMonitorContentScale(m_monitor, &xscale, &yscale);
        return WindowPosition{xscale, yscale};
    }

    WindowSize Monitor::GetCurrentVideoMode() const
    {
        if (const GLFWvidmode* mode = glfwGetVideoMode(m_monitor))
        {
            return WindowSize{static_cast<uint32_t>(mode->width), static_cast<uint32_t>(mode->height)};
        }
        return WindowSize{0, 0};
    }

    WindowRect Monitor::GetWorkArea() const
    {
        int x = 0, y = 0, width = 0, height = 0;
        glfwGetMonitorWorkarea(m_monitor, &x, &y, &width, &height);
        return WindowRect{x, y, width, height};
    }

    WindowSize Monitor::GetDpiScale() const
    {
        int w = 0, h = 0;
#if GLFW_HAS_PER_MONITOR_DPI
        glfwGetMonitorWorkarea(glfw_monitors[n], &x, &y, &w, &h);
        if (w > 0 && h > 0) // Workaround a small GLFW issue reporting zero on monitor changes: https://github.com/glfw/glfw/pull/1761
        {
            monitor.WorkPos  = ImVec2((float)x, (float)y);
            monitor.WorkSize = ImVec2((float)w, (float)h);
        }
#endif
        return WindowSize{(uint32_t)w, (uint32_t)h};
    }

    bool Monitor::IsPrimary() const
    {
        return m_monitor == glfwGetPrimaryMonitor();
    }

    /// Window

    Window::Window(TL::StringView title, TL::Flags<WindowFlags> flags, WindowSize size)
    {
        glfwWindowHint(GLFW_RESIZABLE, (flags & WindowFlags::NonResizable) ? GLFW_FALSE : GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, (flags & WindowFlags::NoDecorations) ? GLFW_FALSE : GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, (flags & WindowFlags::Hidden) ? GLFW_FALSE : GLFW_TRUE);
        m_window = glfwCreateWindow(int(size.width), int(size.height), title.data(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
            {
                Window&     self = *(Window*)glfwGetWindowUserPointer(window);
                WindowEvent event{&self, WindowEventType::Resized};
                event.size = {(uint32_t)width, (uint32_t)height};
                self.m_eventQueue.broadcast(event);
            });
        glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
            {
                Window&     self = *(Window*)glfwGetWindowUserPointer(window);
                WindowEvent event{&self, WindowEventType::Closed};
                self.m_eventQueue.broadcast(event);
            });
        glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
            {
                Window&     self = *(Window*)glfwGetWindowUserPointer(window);
                WindowEvent event{&self, WindowEventType::KeyInput};
                switch (action)
                {
                case GLFW_PRESS:
                    event.keyInput.code  = static_cast<KeyCode>(key);
                    event.keyInput.state = KeyState::Press;
                    event.keyInput.mods  = static_cast<KeyMod>(mods);
                    break;
                case GLFW_RELEASE:
                    event.keyInput.code  = static_cast<KeyCode>(key);
                    event.keyInput.state = KeyState::Release;
                    event.keyInput.mods  = static_cast<KeyMod>(mods);
                    break;
                case GLFW_REPEAT:
                    event.keyInput.code  = static_cast<KeyCode>(key);
                    event.keyInput.state = KeyState::Repeat;
                    event.keyInput.mods  = static_cast<KeyMod>(mods);
                    break;
                default:
                    return;
                }
                self.m_eventQueue.broadcast(event);
            });
        glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int keycode)
            {
                Window&     self = *(Window*)glfwGetWindowUserPointer(window);
                WindowEvent event{&self, WindowEventType::KeyTyped};
                event.keyCode = keycode;
                self.m_eventQueue.broadcast(event);
            });
        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods)
            {
                Window&     self = *(Window*)glfwGetWindowUserPointer(window);
                WindowEvent event{&self, WindowEventType::MouseInput};
                switch (action)
                {
                case GLFW_PRESS:
                    event.mouseInput.code  = static_cast<MouseCode>(button);
                    event.mouseInput.state = KeyState::Press;
                    event.mouseInput.mods  = static_cast<KeyMod>(mods);
                    break;
                case GLFW_RELEASE:
                    event.mouseInput.code  = static_cast<MouseCode>(button);
                    event.mouseInput.state = KeyState::Release;
                    event.mouseInput.mods  = static_cast<KeyMod>(mods);
                    break;
                default:
                    return;
                }
                self.m_eventQueue.broadcast(event);
            });
        glfwSetWindowPosCallback(m_window, [](GLFWwindow* window, int xpos, int ypos)
            {
                Window&     self = *(Window*)glfwGetWindowUserPointer(window);
                WindowEvent event{&self, WindowEventType::Moved};
                event.position.x = (float)(xpos);
                event.position.y = (float)(ypos);
                self.m_eventQueue.broadcast(event);
            });
        glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos)
            {
                Window&     self = *(Window*)glfwGetWindowUserPointer(window);
                WindowEvent event{&self, WindowEventType::CursorMoved};
                event.cursorPosition.x = xPos;
                event.cursorPosition.y = yPos;
                self.m_eventQueue.broadcast(event);
            });
        glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset)
            {
                Window&     self = *(Window*)glfwGetWindowUserPointer(window);
                WindowEvent event{&self, WindowEventType::MouseScrolled};
                event.scrolled.x = xOffset;
                event.scrolled.y = yOffset;
                self.m_eventQueue.broadcast(event);
            });
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_window);
    }

    Window::HandlerID Window::Subscribe(EventHandler&& handler)
    {
        return m_eventQueue.subscribe(std::move(handler));
    }

    void Window::Unsubscribe(HandlerID id)
    {
        m_eventQueue.unsubscribe(id);
    }

    void* Window::GetNativeHandle() const
    {
        return glfwGetWin32Window(m_window);
    }

    bool Window::ShouldClose() const
    {
        return glfwWindowShouldClose(m_window) == GLFW_TRUE;
    }

    void Window::Show() const
    {
        glfwShowWindow(m_window);
    }

    TL::StringView Window::GetTitle() const
    {
        const char* title = glfwGetWindowTitle(m_window);
        if (title)
        {
            return TL::StringView(title);
        }
        return {};
    }

    void Window::SetTitle(TL::StringView title)
    {
        if (title.empty())
        {
            TL_ASSERT(false, "Title cannot be empty");
        }
        glfwSetWindowTitle(m_window, title.data());
    }

    WindowSize Window::GetFramebufferSize() const
    {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    WindowSize Window::GetSize() const
    {
        int width, height;
        glfwGetWindowSize(m_window, &width, &height);
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    void Window::SetSize(WindowSize size)
    {
        if (size.width == 0 || size.height == 0)
        {
            TL_ASSERT(false, "Size cannot be zero");
        }
        glfwSetWindowSize(m_window, int(size.width), int(size.height));
    }

    void Window::SetSizeLimits(WindowSize minSize, WindowSize maxSize)
    {
        if (minSize.width > maxSize.width || minSize.height > maxSize.height)
        {
            TL_ASSERT(false, "Minimum size cannot be greater than maximum size");
        }
        glfwSetWindowSizeLimits(m_window, int(minSize.width), int(minSize.height), int(maxSize.width), int(maxSize.height));
    }

    WindowPosition Window::GetPosition() const
    {
        int x, y;
        glfwGetWindowPos(m_window, &x, &y);
        return WindowPosition{(float)x, (float)y};
    }

    void Window::SetPosition(WindowPosition position)
    {
        glfwSetWindowPos(m_window, position.x, position.y);
    }

    WindowPosition Window::GetCursorPosition() const
    {
        double xpos, ypos;
        glfwGetCursorPos(m_window, &xpos, &ypos);
        return {(float)xpos, (float)ypos};
    }

    WindowPosition Window::GetCursorDeltaPosition() const
    {
        auto prevDelta       = m_previousCursorPosition;
        auto currentPosition = GetCursorPosition();
        auto cursorDeltaX    = currentPosition.x - prevDelta.x;
        auto cursorDeltaY    = currentPosition.y - prevDelta.y;
        return {cursorDeltaX, cursorDeltaY};
    }

    void Window::SetCursorPosition(WindowPosition position)
    {
        glfwSetCursorPos(m_window, position.x, position.y);
    }

    bool Window::GetKeyState(KeyCode key, KeyState state) const
    {
        int glfwState = glfwGetKey(m_window, static_cast<int>(key));
        switch (state)
        {
        case KeyState::None:    return glfwState == GLFW_RELEASE;
        case KeyState::Release: return glfwState == GLFW_RELEASE;
        case KeyState::Press:   return glfwState == GLFW_PRESS;
        case KeyState::Repeat:  return glfwState == GLFW_REPEAT;
        default:                return false;
        }
    }

    bool Window::GetMouseState(MouseCode button, KeyState state) const
    {
        int glfwState = glfwGetMouseButton(m_window, static_cast<int>(button));
        switch (state)
        {
        case KeyState::None:    return glfwState == GLFW_RELEASE;
        case KeyState::Release: return glfwState == GLFW_RELEASE;
        case KeyState::Press:   return glfwState == GLFW_PRESS;
        case KeyState::Repeat:
        default:                return false; // GLFW does not support mouse button repeat, so always return falsereturn false;
        }
    }

    void Window::SetOpacity(float opacity)
    {
        if (opacity < 0.0f || opacity > 1.0f)
        {
            TL_ASSERT(false, "Opacity must be between 0.0 and 1.0");
        }
        glfwSetWindowOpacity(m_window, opacity);
    }

    bool Window::ShouldWindowClose() const
    {
        return glfwWindowShouldClose(m_window) == GLFW_TRUE;
    }

    void Window::SetWindowShouldClose(bool shouldClose)
    {
        glfwSetWindowShouldClose(m_window, shouldClose ? GLFW_TRUE : GLFW_FALSE);
    }

    TL::StringView Window::GetClipboardText() const
    {
        if (const char* clipboardText = glfwGetClipboardString(m_window))
            return TL::StringView(clipboardText);
        return {};
    }

    void Window::SetClipboardText(TL::StringView text)
    {
        glfwSetClipboardString(m_window, text.data());
    }

    bool Window::IsFocused() const
    {
        return glfwGetWindowAttrib(m_window, GLFW_FOCUSED) == GLFW_TRUE;
    }

    void Window::SetFocus()
    {
        glfwFocusWindow(m_window);
    }

    bool Window::GetAttribute(WindowAttribute attribute) const
    {
        switch (attribute)
        {
        case WindowAttribute::None:
            return false;
        case WindowAttribute::Iconified:
            return glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) == GLFW_TRUE;
        case WindowAttribute::Maximized:
            return glfwGetWindowAttrib(m_window, GLFW_MAXIMIZED) == GLFW_TRUE;
        default:
            TL_ASSERT(false, "Unknown window attribute");
            return false;
        }
    }

    void Window::Poll()
    {
        glfwPollEvents();
        m_previousCursorPosition = GetCursorPosition();
        m_eventQueue.poll();
    }

} // namespace Engine