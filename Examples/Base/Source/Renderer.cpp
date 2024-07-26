#include "Examples-Base/Window.hpp"
#include "Examples-Base/Renderer.hpp"

#include <RHI/Common/Result.hpp>
#include <RHI/RHI.hpp>
#include <RHI-Vulkan/Loader.hpp>
#include <RHI/Resources.hpp>

namespace Examples
{
    class DebugCallbacks : public RHI::DebugCallbacks
    {
        void LogInfo(std::string_view message) override
        {
            (void)message;
        }

        void LogWarnning(std::string_view message) override
        {
            (void)message;
        }

        void LogError(std::string_view message) override
        {
            (void)message;
        }
    };

    ResultCode Renderer::Init(const Window& window)
    {
        ZoneScoped;

        m_window = &window;

        RHI::ApplicationInfo appInfo{};
        appInfo.applicationName = "Example APP";
        appInfo.applicationVersion.minor = 1;
        appInfo.engineName = "Engine name";
        appInfo.engineVersion.minor = 1;
        m_context = RHI::CreateVulkanContext(appInfo);

        RHI::SwapchainCreateInfo swapchainCI{};
        swapchainCI.name = "Swapchain";
        swapchainCI.minImageCount = 3;
        swapchainCI.imageSize.width = window.GetWindowSize().width;
        swapchainCI.imageSize.height = window.GetWindowSize().height;
        swapchainCI.imageFormat = RHI::Format::RGBA8_UNORM;
        swapchainCI.imageUsage = RHI::ImageUsage::Color;
        swapchainCI.presentMode = RHI::SwapchainPresentMode::Fifo;
        swapchainCI.win32Window.hwnd = window.GetNativeHandle();
        m_swapchain = m_context->CreateSwapchain(swapchainCI);

        m_renderGraph = m_context->CreateRenderGraph();

        m_commandPool[0] = m_context->CreateCommandPool(RHI::CommandPoolFlags::Reset);
        m_commandPool[1] = m_context->CreateCommandPool(RHI::CommandPoolFlags::Reset);

        m_frameInFlightFence[0] = m_context->CreateFence();
        m_frameInFlightFence[1] = m_context->CreateFence();

        return OnInit();
    }

    void Renderer::Shutdown()
    {
        OnShutdown();

        delete m_commandPool[0].release();
        delete m_commandPool[1].release();
        delete m_renderGraph.release();
        delete m_swapchain.release();
        delete m_context.release();
    }

    void Renderer::Render()
    {
        {
            // RHI::ImageSize2D size = { m_window->GetWindowSize().width, m_window->GetWindowSize().height };
            // if (m_swapchain.get)
            // auto result = m_swapchain->Recreate(size);
            // RHI_ASSERT(RHI::IsSucess(result) && "Failed to recreate swapchain on resize");
        }

        OnRender();

        [[maybe_unused]] auto result = m_swapchain->Present();
    }

} // namespace Examples