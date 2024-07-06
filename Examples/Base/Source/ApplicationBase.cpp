#include "Examples-Base/ApplicationBase.hpp"
#include "Examples-Base/Window.hpp"
#include "Examples-Base/ImGuiRenderer.hpp"
#include "Examples-Base/Event.hpp"
#include "Examples-Base/FileSystem.hpp"
#include "Examples-Base/Camera.hpp"

#include <RHI-Vulkan/Loader.hpp>

#undef LoadImage

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <tracy/Tracy.hpp>

#include <cassert>
#include <iostream>
#include <chrono>

namespace Examples
{
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

    ApplicationBase::ApplicationBase(const char* name, uint32_t windowWidth, uint32_t windowHeight)
        : m_context(nullptr)
        , m_swapchain(nullptr)
        , m_imguiRenderer(nullptr)
        , m_window(nullptr)
        , m_isRunning(true)

    {
        Window::Init();

        auto windowEventDispatcher = [this](Event& event)
        {
            this->DispatchEvent(event);
        };
        m_window = RHI::CreatePtr<Window>(name, Window::Size{ windowWidth, windowHeight }, windowEventDispatcher);

        m_imguiRenderer = RHI::CreatePtr<ImGuiRenderer>();
    }

    ApplicationBase::~ApplicationBase()
    {
        Window::Shutdown();
    }

    void ApplicationBase::Init()
    {
        ZoneScoped;

        RHI::ApplicationInfo appInfo{};
        appInfo.applicationName = "RHI-App";
        appInfo.applicationVersion = { 0, 1, 0 };
        auto debugCallbacks = RHI::CreatePtr<DebugCallbacks>();
        m_context = RHI::CreateVulkanContext(appInfo, std::move(debugCallbacks));

        // create swapchain
        RHI::SwapchainCreateInfo createInfo{};
        createInfo.win32Window.hwnd = m_window->GetNativeHandle();
        createInfo.win32Window.hinstance = NULL;
        createInfo.imageSize.width = m_window->GetWindowSize().width;
        createInfo.imageSize.height = m_window->GetWindowSize().height;
        createInfo.imageUsage = RHI::ImageUsage::Color;
        createInfo.imageUsage |= RHI::ImageUsage::ShaderResource;
        createInfo.imageFormat = RHI::Format::BGRA8_UNORM;
        createInfo.imageCount = 3;

        m_swapchain = m_context->CreateSwapchain(createInfo);

        ImGuiRenderer::CreateInfo imguiRendererCreateInfo{};
        imguiRendererCreateInfo.context = m_context.get();
        imguiRendererCreateInfo.shaderBlob = ReadBinaryFile("./Shaders/ImGui.spv");
        m_imguiRenderer->Init(imguiRendererCreateInfo);

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize.x = float(m_window->GetWindowSize().width);
        io.DisplaySize.y = float(m_window->GetWindowSize().height);

        OnInit();
    }

    void ApplicationBase::Shutdown()
    {
        ZoneScoped;
        m_imguiRenderer->Shutdown();
        OnShutdown();
    }

    void ApplicationBase::DispatchEvent(Event& event)
    {
        m_imguiRenderer->ProcessEvent(event);
        if (event.Handled)
            return;

        // propagate to application level event handling.
        OnEvent(event);
        if (event.Handled)
            return;
    }

    void ApplicationBase::Run()
    {
        auto currentTime = std::chrono::high_resolution_clock::now().time_since_epoch();

        double accumulator = 0.0;
        double deltaTime = 0.01;
        while (m_isRunning)
        {
            auto newTime = std::chrono::high_resolution_clock::now().time_since_epoch();
            double frameTime = std::chrono::duration<double>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = std::min(frameTime, 0.25);
            accumulator += frameTime;

            while (accumulator >= deltaTime)
            {
                m_window->OnUpdate();

                accumulator -= deltaTime;
            }

            // Render
            {
                ZoneScopedN("app-base-update");
                OnUpdate(Timestep(deltaTime));
            }
        }
    }

} // namespace Examples