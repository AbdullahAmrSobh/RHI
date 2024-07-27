#include "Examples-Base/Window.hpp"
#include "Examples-Base/Renderer.hpp"
#include "Examples-Base/Log.hpp"

#include <RHI/RHI.hpp>
#include <RHI-Vulkan/Loader.hpp>

namespace Examples
{
    class DebugCallbacksImpl final : public RHI::DebugCallbacks
    {
    public:
        ~DebugCallbacksImpl() = default;

        void LogInfo(std::string_view message) override
        {
            Core::LogInfo(message);
        }

        void LogWarnning(std::string_view message) override
        {
            Core::LogWarnning(message);
        }

        void LogError(std::string_view message) override
        {
            Core::LogError(message);
        }
    };

    Renderer::Renderer()
    {
    }

    Renderer::~Renderer()
    {
    }

    ResultCode Renderer::Init(const Window& window)
    {
        m_window = &window;

        RHI::ApplicationInfo appInfo{};
        appInfo.applicationName = "Example APP";
        appInfo.applicationVersion.minor = 1;
        appInfo.engineName = "Engine name";
        appInfo.engineVersion.minor = 1;
        m_context = RHI::CreateVulkanContext(appInfo, RHI::CreatePtr<DebugCallbacksImpl>());

        auto windowSize = window.GetWindowSize();
        RHI::SwapchainCreateInfo swapchainCI{};
        swapchainCI.name = "Swapchain";
        swapchainCI.minImageCount = 3;
        swapchainCI.imageSize = { windowSize.width, windowSize.height };
        swapchainCI.imageFormat = RHI::Format::RGBA8_UNORM;
        swapchainCI.imageUsage = RHI::ImageUsage::Color;
        swapchainCI.presentMode = RHI::SwapchainPresentMode::Fifo;
        swapchainCI.win32Window.hwnd = window.GetNativeHandle();
        m_swapchain = m_context->CreateSwapchain(swapchainCI);

        for (auto& cmdPool : m_commandPool)
            cmdPool = m_context->CreateCommandPool(RHI::CommandPoolFlags::Reset);

        for (auto& fence : m_frameFence)
            fence = m_context->CreateFence();

        return OnInit();
    }

    void Renderer::Shutdown()
    {
        OnShutdown(); // must be called first thing here

        for (auto& fence : m_frameFence)
        {
            fence->Wait(UINT64_MAX);
            delete fence.release();
        }

        for (auto& cmdPool : m_commandPool)
        {
            delete cmdPool.release();
        }

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