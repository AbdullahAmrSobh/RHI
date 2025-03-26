#include <RHI/RHI.hpp>

#include <RHI-WebGPU/Loader.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>

int main(int argc, const char* argv[])
{
    auto device = RHI::CreateWebGPUDevice();

    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    auto hwnd = glfwGetWin32Window(window);

    RHI::SwapchainCreateInfo swapchainCI{
        .name          = "Swapchain",
        .imageSize     = {800, 480},
        .imageUsage    = {RHI::ImageUsage::Color},
        .imageFormat   = RHI::Format::RGBA8_UNORM,
        .minImageCount = 2,
        .alphaMode     = RHI::SwapchainAlphaMode::PreMultiplied,
        .presentMode   = RHI::SwapchainPresentMode::Fifo,
    };
    auto swapchain = device->CreateSwapchain(swapchainCI);

    while (!glfwWindowShouldClose(window))
    {
        auto result = swapchain->Present();
        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

    RHI::DestroyWebGPUDevice(device);

    return 0;
}