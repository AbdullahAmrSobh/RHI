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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
            appInfo.engineName         = "RHI example";
            appInfo.engineVersion      = RHI::MakeVersion(0, 0, 1);
            appInfo.applicationName    = "RHI example";
            appInfo.applicationVersion = RHI::MakeVersion(0, 0, 1);

            m_context = RHI::Context::Create({}, std::make_unique<DebugCallbacks>(), RHI::Backend::Vulkan);

            RHI::SwapchainCreateInfo swaphainInfo {};
            swaphainInfo.size               = {800, 600, 0};
            swaphainInfo.imagesFormat       = RHI::Format::R8G8B8A8_UNORM_SRGB;
            swaphainInfo.backBuffersCount   = 2;
            swaphainInfo.nativeWindowHandle = glfwGetWin32Window(m_window);

            m_swapchain = m_context->CreateSwapchain(swaphainInfo);

            m_scheduler = m_context->CreateFrameScheduler();
        }

        {
            glfwSetWindowUserPointer(m_window, m_swapchain.get());
            glfwSetWindowSizeCallback(m_window,
                                      [](GLFWwindow* window, int width, int height)
                                      {
                                          RHI::Swapchain* swapchain = reinterpret_cast<RHI::Swapchain*>(glfwGetWindowUserPointer(window));
                                          swapchain->Resize(width, height);
                                      });
        }

        m_shaderResourceGroupAllocator = m_context->CreateShaderResourceGroupAllocator();
    }

    virtual ~ExampleBase()
    {
        glfwTerminate();
    }

    std::vector<uint8_t> ReadBinrayFile(std::string_view filePath)
    {
        std::ifstream file(std::string(filePath), std::ios::ate | std::ios::binary);

        RHI_ASSERT_MSG(file.is_open(), "Failed to open the file");

        auto                 fileSize = file.tellg();
        std::vector<uint8_t> buffer(fileSize);

        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));

        file.close();

        return buffer;
    }

    ImageData ReadImageData(std::string_view path)
    {
        ImageData data {};
        int32_t   width, height, channels;
        auto      img = stbi_load(path.data(), &width, &height, &channels, 0);

        memcpy(data.imageData.data(), img, width * height * channels);
        data.width    = width;
        data.height   = height;
        data.channels = channels;

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
    std::unique_ptr<RHI::Context>                      m_context;
    std::unique_ptr<RHI::Swapchain>                    m_swapchain;
    std::unique_ptr<RHI::FrameScheduler>               m_scheduler;
    std::unique_ptr<RHI::ShaderResourceGroupAllocator> m_shaderResourceGroupAllocator;
};

#define ENTRY_POINT(exampleName)                                                                                                           \
    int main(int argc, const char* argv[])                                                                                                 \
    {                                                                                                                                      \
        std::unique_ptr<ExampleBase> example = std::make_unique<exampleName>(argc, argv);                                                  \
        example->Run();                                                                                                                    \
    }
