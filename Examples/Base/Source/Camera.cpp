#include "Examples-Base/Window.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

Window::Window(const char* title, uint32_t width, uint32_t height)
    : m_window(nullptr)
    , m_title(title)
    , m_width(width)
    , m_height(height)
{
    if (!glfwInit())
        RHI_UNREACHABLE();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window)
    {
        glfwTerminate();
        RHI_UNREACHABLE();
    }
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

Ptr<Window> Window::Create(const char* title, uint32_t width, uint32_t height)
{
    return RHI::CreatePtr<Window>(title, width, height);
}

void* Window::GetNativeWindow() const
{
#ifdef _WIN32
    return glfwGetWin32Window(m_window);
#elif __linux__
    return (void*)glfwGetX11Window(m_Window);
#else
    #error "Unsupported platform"
#endif
}

GLFWwindow* Window::GetGLFWHandle() const
{
    return m_window;
}

const char* Window::GetTitle() const
{
    return m_title.c_str();
}

uint32_t Window::GetWidth() const
{
    return m_width;
}

uint32_t Window::GetHeight() const
{
    return m_height;
}
