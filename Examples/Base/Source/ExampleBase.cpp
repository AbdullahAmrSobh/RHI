#include <cassert>
#include <iostream>
#include <memory>
#include <string>

#include "Examples-Base/ExampleBase.hpp"

#include <RHI-Vulkan/Loader.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#undef LoadImage

class DebugCallbacks final : public RHI::DebugCallbacks
{
public:
    void LogInfo(std::string_view message) override
    {
        std::cout << "INFO: " << message << "\n";
    }

    void LogWarnning(std::string_view message) override
    {
        std::cout << "WARNNING: " << message << "\n";
    }

    void LogError(std::string_view message) override
    {
        std::cout << "ERROR: " << message << "\n";
    }
};

ImageData ExampleBase::LoadImage(std::string_view path) const
{
    return {};
}

std::vector<uint8_t> ExampleBase::ReadBinaryFile(std::string_view path) const
{
    return {};
}

ExampleBase::ExampleBase(std::string name, uint32_t width, uint32_t height)
{
    auto result = glfwInit();
    assert(result);

    auto window = glfwCreateWindow(static_cast<uint32_t>(width), static_cast<uint32_t>(height), name.c_str(), nullptr, nullptr);
    assert(window);

    glfwMakeContextCurrent(window);

    m_window = window;

    RHI::DeviceSelectionCallback callback = [=](RHI::TL::Span<const RHI::DeviceProperties> properties)
    {
        (void)properties;
        return 0u;
    };

    RHI::ApplicationInfo appInfo {};
    m_context = RHI::CreateVulkanRHI(appInfo, callback, std::make_unique<DebugCallbacks>());
}

void ExampleBase::Init()
{
    WindowInfo windowInfo {};
    windowInfo.hwnd      = glfwGetWin32Window(static_cast<GLFWwindow*>(m_window));
    windowInfo.hinstance = NULL;

    OnInit(windowInfo);
}

void ExampleBase::Shutdown()
{
    OnShutdown();
    glfwTerminate();
}

void ExampleBase::Run()
{
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(m_window);

    while (!glfwWindowShouldClose(window))
    {
        OnUpdate();

        glfwPollEvents();
    }
}
