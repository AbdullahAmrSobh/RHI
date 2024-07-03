#pragma once

#include "Examples-Base/Common.hpp"

#include <cstdint>

struct GLFWwindow;

class Window
{
public:
    Window(const char* title, uint32_t width, uint32_t height);
    ~Window();

    static Ptr<Window> Create(const char* title, uint32_t width, uint32_t height);

    struct Size
    {
        uint32_t width, height;
    };

    void* GetNativeWindow() const;

    GLFWwindow* GetGLFWHandle() const;

    const char* GetTitle() const;

    Size GetSize() const;

    Size GetCursorPosition() const;

private:
    GLFWwindow* m_window;
    std::string m_title;
    uint32_t m_width;
    uint32_t m_height;
};
