#include "Examples-Base/Window.hpp"
#include "Examples-Base/Event.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <TL/Assert.hpp>

namespace Examples
{
    void Window::Init()
    {
        auto result = glfwInit();
        TL_ASSERT(result);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    void Window::Shutdown()
    {
        glfwTerminate();
    }

    Window::Window(const char* name, Size size, const EventHandler& eventHandler)
        : m_name(name)
        , m_currentSize(size)
        , m_previousCursorPosition({})
        , m_window(nullptr)
        , m_handler(eventHandler)
    {
        m_window = glfwCreateWindow(int(m_currentSize.width), int(m_currentSize.height), name, nullptr, nullptr);

        glfwMakeContextCurrent(m_window);

        glfwSetWindowUserPointer(m_window, this);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
        {
            Window& self = *(Window*)glfwGetWindowUserPointer(window);
            self.m_currentSize.width = (uint32_t)width;
            self.m_currentSize.height = (uint32_t)height;

            WindowResizeEvent event((uint32_t)width, (uint32_t)height);
            self.m_handler(event);
        });

        glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
        {
            Window& self = *(Window*)glfwGetWindowUserPointer(window);

            WindowCloseEvent event;
            self.m_handler(event);
        });

        glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            (void)scancode;
            (void)mods;

            Window& self = *(Window*)glfwGetWindowUserPointer(window);

            switch (action)
            {
            case GLFW_PRESS:
                {
                    KeyPressedEvent event((KeyCode)key, 0);
                    self.m_handler(event);
                    break;
                }
            case GLFW_RELEASE:
                {
                    KeyReleasedEvent event((KeyCode)key);
                    self.m_handler(event);
                    break;
                }
            case GLFW_REPEAT:
                {
                    KeyPressedEvent event((KeyCode)key, true);
                    self.m_handler(event);
                    break;
                }
            }
        });

        glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int keycode)
        {
            Window& self = *(Window*)glfwGetWindowUserPointer(window);

            KeyTypedEvent event((KeyCode)keycode);
            self.m_handler(event);
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods)
        {
            (void)mods;

            Window& self = *(Window*)glfwGetWindowUserPointer(window);

            switch (action)
            {
            case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event((MouseCode)button);
                    self.m_handler(event);
                    break;
                }
            case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event((MouseCode)button);
                    self.m_handler(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            Window& self = *(Window*)glfwGetWindowUserPointer(window);

            MouseScrolledEvent event((float)xOffset, (float)yOffset);
            self.m_handler(event);
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos)
        {
            Window& self = *(Window*)glfwGetWindowUserPointer(window);

            MouseMovedEvent event((float)xPos, (float)yPos);
            self.m_handler(event);
        });
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_window);
    }

    bool Window::IsKeyPressed(KeyCode key) const
    {
        auto state = glfwGetKey(m_window, (int32_t)key);
        return state == GLFW_PRESS;
    }

    bool Window::IsMouseButtonPressed(MouseCode button) const
    {
        auto state = glfwGetMouseButton(m_window, (int32_t)button);
        return state == GLFW_PRESS;
    }

    Window::Size Window::GetWindowSize() const
    {
        return m_currentSize;
    }

    Window::Cursor Window::GetCursorPosition() const
    {
        double xpos, ypos;
        glfwGetCursorPos(m_window, &xpos, &ypos);
        return Cursor{ (float)xpos, (float)ypos };
    }

    Window::Cursor Window::GetCursrorDeltaPosition() const
    {
        auto currentPosition = GetCursorPosition();
        auto cursorDeltaX = currentPosition.x - m_previousCursorPosition.x;
        auto cursorDeltaY = currentPosition.y - m_previousCursorPosition.y;
        return { cursorDeltaX, cursorDeltaY };
    }

    void* Window::GetNativeHandle() const
    {
        return glfwGetWin32Window(m_window);
    }

    GLFWwindow* Window::GetGlfwWindow() const
    {
        return m_window;
    }

    void Window::SetEventCallback(const EventHandler& eventHandler)
    {
        m_handler = eventHandler;
    }

    void Window::OnUpdate()
    {
        glfwPollEvents();
        m_previousCursorPosition = GetCursorPosition();
    }

} // namespace Examples