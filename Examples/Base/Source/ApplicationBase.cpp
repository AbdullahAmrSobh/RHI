#include "Examples-Base/ApplicationBase.hpp"

#include <RHI-Vulkan/Loader.hpp>

#undef LoadImage

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <tracy/Tracy.hpp>

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <chrono>

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

ApplicationBase::ApplicationBase(std::string name, uint32_t width, uint32_t height)
{
    auto result = glfwInit();
    assert(result);

    auto window = Window::Create(name.c_str(), width, height);
    assert(window);

    // m_window = window;

    m_camera.SetPerspective(60.0f, float(m_window->GetWidth()) / float(m_window->GetHeight()), 0.1f, 10000.0f);
    m_camera.SetMovementSpeed(1.0);
    m_camera.SetRotationSpeed(0.5);
}

void ApplicationBase::Init()
{
    ZoneScoped;

    // Create RHI context
    RHI::ApplicationInfo appInfo{
        .applicationName = "RHI-App",
        .applicationVersion = { 0, 1, 0 },
        .engineName = "RHI-Engine",
        .engineVersion = { 0, 1, 0 },
    };
    m_context = RHI::CreateVulkanContext(appInfo, RHI::CreatePtr<DebugCallbacks>());

    // create swapchain
    auto size = m_window->GetSize();
    RHI::SwapchainCreateInfo createInfo{
        .name = "Swapchain",
        .imageSize = { size.width, size.height },
        .imageUsage = RHI::ImageUsage::Color,
        .imageFormat = RHI::Format::BGRA8_UNORM,
        .imageCount = 2,
        .presentMode = RHI::SwapchainPresentMode::Fifo,
        .win32Window = {
            .hwnd = m_window->GetNativeWindow(),
            .hinstance = NULL,
        }
    };
    m_swapchain = m_context->CreateSwapchain(createInfo);

    OnInit();

    // ImGuiRendererCreateInfo imguiRendererCreateInfo{};
    // imguiRendererCreateInfo.context = m_context.get();
    // imguiRendererCreateInfo.shaderBlob = ReadBinaryFile("./Shaders/ImGui.spv");
    // m_imguiRenderer->Init(imguiRendererCreateInfo);

    // ImGuiIO& io = ImGui::GetIO();
    // io.DisplaySize.x = float(m_window->GetWidth());
    // io.DisplaySize.y = float(m_window->GetHeight());

    // m_imguiRenderer->m_window = (GLFWwindow*)m_window;
    // glfwSetWindowUserPointer((GLFWwindow*)m_window, m_imguiRenderer.get());
    // m_imguiRenderer->InstallGlfwCallbacks((GLFWwindow*)m_window);
}

void ApplicationBase::Shutdown()
{
    ZoneScoped;
    // m_imguiRenderer->Shutdown();
    OnShutdown();
    glfwTerminate();
}

void ApplicationBase::Run()
{
    GLFWwindow* window = (GLFWwindow*)(m_window->GetGLFWHandle());

    auto currentTime = std::chrono::high_resolution_clock::now().time_since_epoch();

    double accumulator = 0.0;
    double deltaTime = 0.01;

    using CursorPos = glm::vec<3, double>;
    CursorPos previousCursorPos = {};

    while (!glfwWindowShouldClose(window))
    {
        auto newTime = std::chrono::high_resolution_clock::now().time_since_epoch();
        double frameTime = std::chrono::duration<double>(newTime - currentTime).count();
        currentTime = newTime;

        frameTime = min(frameTime, 0.25);
        accumulator += frameTime;

        while (accumulator >= deltaTime)
        {
            CursorPos currentCursorPos = {};
            glfwGetCursorPos(window, &currentCursorPos.x, &currentCursorPos.y);
            auto cursorDelta = currentCursorPos - previousCursorPos;
            previousCursorPos = currentCursorPos;

            ProcessInput();

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                m_camera.Rotate({ -cursorDelta.y, cursorDelta.x, 0.0 });
            }

            glfwPollEvents();

            accumulator -= deltaTime;
        }

        // Render
        {
            ZoneScopedN("app-base-update");
            OnUpdate(Timestep(deltaTime));
        }
    }
}

void ApplicationBase::ProcessInput()
{
    GLFWwindow* window = (GLFWwindow*)(m_window->GetGLFWHandle());
    m_camera.keys.up = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    m_camera.keys.down = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    m_camera.keys.right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    m_camera.keys.left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
}