#pragma once

#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "RHI/RHI.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#pragma warning(disable : 4081)

#define STB_IMAGE_IMPLEMENTATION
#pragma warning(disable : 4365)
#pragma warning(disable : 4083)
#pragma warning(disable : 4242)
#include "stb_image.h"
#pragma warning(default : 4365)
#pragma warning(default : 4083)
#pragma warning(default : 4242)

struct ImageData
{
    std::vector<uint8_t> imageData;
    uint32_t             width;
    uint32_t             height;
    uint32_t             channels;
};

class DebugCallbacks final : public RHI::DebugCallbacks
{
    void Log(std::string_view message) override
    {
        std::cout << "Log: " << message << std::endl;
    }

    void Warn(std::string_view message) override
    {
        std::cout << "Warn: " << message << std::endl;
    }

    void Error(std::string_view message) override
    {
        std::cout << "Error: " << message << std::endl;
    }
};

class ExampleBase
{
public:
    ExampleBase()
    {
        {
            if (!glfwInit())
                std::terminate();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

            m_window = glfwCreateWindow(800, 600, "RHI Example", nullptr, nullptr);
            assert(m_window);  // failed to create window
        }

        {
            RHI::ApplicationInfo appInfo {};
            appInfo.graphicsBackend = RHI::Backend::Vulkan;
            appInfo.engineName         = "RHI example";
            appInfo.engineVersion      = RHI::MakeVersion(0, 0, 1);
            appInfo.applicationName    = "RHI example";
            appInfo.applicationVersion = RHI::MakeVersion(0, 0, 1);

            m_context = RHI::Context::Create(appInfo, std::make_unique<DebugCallbacks>());

            RHI::SwapchainCreateInfo swapchainInfo {};
            swapchainInfo.imageSize          = {800, 600, 0};
            swapchainInfo.imageUsage         = RHI::ImageUsage::Color;
            swapchainInfo.imageFormat        = RHI::Format::RGBA8;
            swapchainInfo.imageCount         = 2;
            swapchainInfo.nativeWindowHandle = glfwGetWin32Window(m_window);

            m_swapchain = m_context->CreateSwapchain(swapchainInfo);

            // m_scheduler = m_context->CreateFrameScheduler();
        }

        {
            glfwSetWindowUserPointer(m_window, m_swapchain.get());
            glfwSetWindowSizeCallback(m_window,
                                      [](GLFWwindow* window, int width, int height)
                                      {
                                          RHI::Swapchain* swapchain = reinterpret_cast<RHI::Swapchain*>(glfwGetWindowUserPointer(window));
                                          swapchain->Resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
                                      });
        }
    }

    virtual ~ExampleBase()
    {
        glfwTerminate();
    }

    std::vector<uint32_t> ReadBinrayFile(std::string_view filePath)
    {
        std::ifstream file(std::string(filePath), std::ios::ate | std::ios::binary);

        RHI_ASSERT_MSG(file.is_open(), "Failed to open the file");

        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint32_t> data;
        data.resize(fileSize / 4); 
        file.read(reinterpret_cast<char*>(data.data()), fileSize);

        file.close();

        return data;
    }

    ImageData ReadImageData(std::string_view path)
    {
        ImageData data {};
        int32_t   width, height, channels;
        auto      img = stbi_load(path.data(), &width, &height, &channels, 0);

        memcpy(data.imageData.data(), img, width * height * channels);
        data.width    = static_cast<uint32_t>(width);
        data.height   = static_cast<uint32_t>(height);
        data.channels = static_cast<uint32_t>(channels);

        return data;
    }

    virtual void OnUpdate(double timeStep) = 0;

private:
    friend int main(int, const char*[]);

    void Run()
    {
        m_timestep = glfwGetTime();

        while (!glfwWindowShouldClose(m_window))
        {
            m_timestep = glfwGetTime() - m_timestep;

            this->OnUpdate(m_timestep);

            glfwPollEvents();
        }
    }

private:
    double      m_timestep;  // current time in ms
    GLFWwindow* m_window;

protected:
    std::unique_ptr<RHI::Context>   m_context;
    std::unique_ptr<RHI::Swapchain> m_swapchain;
    // std::unique_ptr<RHI::FrameScheduler>               m_scheduler;
    // std::unique_ptr<RHI::ShaderResourceGroupAllocator> m_shaderResourceGroupAllocator;
};

#define ENTRY_POINT(exampleName)                                                                                                           \
    int main(int argc, const char* argv[])                                                                                                 \
    {                                                                                                                                      \
        std::unique_ptr<ExampleBase> example = std::make_unique<exampleName>(argc, argv);                                                  \
        example->Run();                                                                                                                    \
    }
