#include "Examples-Base/ExampleBase.hpp"

#include <RHI-Vulkan/Loader.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>


#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#undef LoadImage

#include "stb_image.h"

class DebugCallbacks final : public RHI::DebugCallbacks
{
public:
    void LogInfo(std::string_view message, ...) override
    {
        std::cout << "INFO: " << message << "\n";
    }

    void LogWarnning(std::string_view message, ...) override
    {
        std::cout << "WARNNING: " << message << "\n";
    }

    void LogError(std::string_view message, ...) override
    {
        std::cout << "ERROR: " << message << "\n";
    }
};

ImageData ExampleBase::LoadImage(std::string_view path) const
{
    // Load the image using stb_image
    int width, height, channels;
    stbi_set_flip_vertically_on_load(0); // Set to 0 to keep image orientation as is
    unsigned char* data = stbi_load(path.data(), &width, &height, &channels, 0);

    // Check if the image was loaded successfully
    if (!data)
    {
        RHI_UNREACHABLE(); // ("Failed to load image: " + std::string(path));
    }

    // Create ImageData object
    ImageData imageData;
    imageData.width           = width;
    imageData.height          = height;
    imageData.depth           = 1; // Assuming single-layer images
    imageData.channels        = channels;
    imageData.bytesPerChannel = 1; // Assuming 8-bit per channel data

    // Convert image data to std::vector<uint8_t>
    imageData.data.assign(data, data + width * height * channels);

    // Free the memory allocated by stb_image
    stbi_image_free(data);

    return imageData;
}

std::vector<uint32_t> ExampleBase::ReadBinaryFile(std::string_view path) const
{
    std::ifstream stream(path.data(), std::ios::ate | std::ios::binary);

    RHI_ASSERT(stream.is_open());

    auto fileSize = (size_t)stream.tellg();

    stream.seekg(0);

    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    stream.read((char*)buffer.data(), fileSize);

    stream.close();

    return buffer;
}

ExampleBase::ExampleBase(std::string name, uint32_t width, uint32_t height)
{
    auto result = glfwInit();
    assert(result);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    auto window = glfwCreateWindow(static_cast<uint32_t>(width), static_cast<uint32_t>(height), name.c_str(), nullptr, nullptr);
    assert(window);

    glfwMakeContextCurrent(window);

    m_window = window;

    RHI::ApplicationInfo appInfo{};
    appInfo.applicationName    = "RHI-App";
    appInfo.applicationVersion = RHI::MakeVersion(0, 1, 0);
    auto debugCallbacks        = std::make_unique<DebugCallbacks>();
    m_context                  = RHI::CreateVulkanRHI(appInfo, std::move(debugCallbacks));
}

void ExampleBase::Init()
{
    WindowInfo windowInfo{};
    windowInfo.hwnd      = glfwGetWin32Window(static_cast<GLFWwindow*>(m_window));
    windowInfo.hinstance = NULL;

    // create swapchain
    RHI::SwapchainCreateInfo createInfo{};
    createInfo.win32Window.hwnd      = windowInfo.hwnd;
    createInfo.win32Window.hinstance = windowInfo.hinstance;
    createInfo.imageSize.width       = windowInfo.width;
    createInfo.imageSize.height      = windowInfo.height;
    createInfo.imageUsage            = RHI::ImageUsage::Color;
    createInfo.imageFormat           = RHI::Format::BGRA8_UNORM;
    createInfo.imageCount            = 3;

    m_swapchain = m_context->CreateSwapchain(createInfo);

    // create frame scheduler
    m_frameScheduler = m_context->CreateFrameScheduler();

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
