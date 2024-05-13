#include "Examples-Base/ApplicationBase.hpp"

#include <RHI-Vulkan/Loader.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#undef LoadImage

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <tracy/Tracy.hpp>

#include <cassert>
#include <fstream>
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

ImageData LoadImage(std::string_view path)
{
    // Load the image using stb_image
    int width, height, channels;
    stbi_set_flip_vertically_on_load(0); // Set to 0 to keep image orientation as is
    unsigned char* data = stbi_load(path.data(), &width, &height, &channels, 4);

    // Check if the image was loaded successfully
    if (!data)
    {
        RHI_UNREACHABLE(); // ("Failed to load image: " + std::string(path));
    }

    // Create ImageData object
    ImageData imageData;
    imageData.width = uint32_t(width);
    imageData.height = uint32_t(height);
    imageData.depth = 1; // Assuming single-layer images
    imageData.channels = uint32_t(channels);
    imageData.bytesPerChannel = 1; // Assuming 8-bit per channel data

    // Convert image data to std::vector<uint8_t>
    imageData.data.assign(data, data + width * height * channels);

    // Free the memory allocated by stb_image
    stbi_image_free(data);

    return imageData;
}

TL::Vector<uint8_t> ReadBinaryFile(std::string_view path)
{
    std::ifstream stream(path.data(), std::ios::ate | std::ios::binary);
    RHI_ASSERT(stream.is_open());
    auto fileSize = (size_t)stream.tellg();
    stream.seekg(0);
    TL::Vector<uint8_t> buffer(fileSize);
    stream.read((char*)buffer.data(), (std::streamsize)fileSize);
    stream.close();
    return buffer;
}

ApplicationBase::ApplicationBase(std::string name, uint32_t width, uint32_t height)
    : m_windowWidth(width)
    , m_windowHeight(height)
    , m_imguiRenderer(RHI::CreatePtr<ImGuiRenderer>())
{
    auto result = glfwInit();
    assert(result);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    auto window = glfwCreateWindow(int(m_windowWidth), int(m_windowHeight), name.c_str(), nullptr, nullptr);
    assert(window);

    glfwMakeContextCurrent(window);

    m_window = window;

    m_camera.SetPerspective(60.0f, float(m_windowWidth) / float(m_windowHeight), 0.1f, 10000.0f);
    m_camera.SetMovementSpeed(1.0);
    m_camera.SetRotationSpeed(0.5);

    RHI::ApplicationInfo appInfo{};
    appInfo.applicationName = "RHI-App";
    appInfo.applicationVersion = { 0, 1, 0 };
    auto debugCallbacks = RHI::CreatePtr<DebugCallbacks>();
    m_context = RHI::CreateVulkanContext(appInfo, std::move(debugCallbacks));
}

void ApplicationBase::Init()
{
    ZoneScoped;

    // create swapchain
    RHI::SwapchainCreateInfo createInfo{};
    createInfo.win32Window.hwnd = glfwGetWin32Window(static_cast<GLFWwindow*>(m_window));
    createInfo.win32Window.hinstance = NULL;
    createInfo.imageSize.width = m_windowWidth;
    createInfo.imageSize.height = m_windowHeight;
    createInfo.imageUsage = RHI::ImageUsage::Color;
    createInfo.imageUsage |= RHI::ImageUsage::ShaderResource;
    createInfo.imageFormat = RHI::Format::BGRA8_UNORM;
    createInfo.imageCount = 3;

    m_swapchain = m_context->CreateSwapchain(createInfo);

    m_commandPool = m_context->CreateCommandPool(RHI::CommandPoolFlags::Reset);

    OnInit();

    ImGuiRendererCreateInfo imguiRendererCreateInfo{};
    imguiRendererCreateInfo.context = m_context.get();
    imguiRendererCreateInfo.shaderBlob = ReadBinaryFile("./Shaders/ImGui.spv");
    imguiRendererCreateInfo.commandAllocator = m_commandPool.get();
    m_imguiRenderer->Init(imguiRendererCreateInfo);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = float(m_windowWidth);
    io.DisplaySize.y = float(m_windowHeight);

    m_imguiRenderer->m_window = (GLFWwindow*)m_window;
    glfwSetWindowUserPointer((GLFWwindow*)m_window, m_imguiRenderer.get());
    m_imguiRenderer->InstallGlfwCallbacks((GLFWwindow*)m_window);
}

void ApplicationBase::Shutdown()
{
    ZoneScoped;
    m_imguiRenderer->Shutdown();
    OnShutdown();
    glfwTerminate();
}

void ApplicationBase::Run()
{
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(m_window);

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
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(m_window);

    m_camera.keys.up = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    m_camera.keys.down = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    m_camera.keys.right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    m_camera.keys.left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
}