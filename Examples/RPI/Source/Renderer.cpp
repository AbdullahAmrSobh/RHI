#include "RPI/Renderer.hpp"

#include <Examples-Base/Window.hpp>

#include <RHI/RHI.hpp>
#include <RHI-Vulkan/Loader.hpp>

#include <TL/Log.hpp>

namespace Examples::RPI
{

    TL::Ptr<Renderer> Renderer::CreateDeferred()
    {
        return TL::CreatePtr<Renderer>();
    }

    ResultCode Renderer::Init(const Window& window)
    {
        m_window = &window;

        RHI::ApplicationInfo appInfo{
            .applicationName = "Example",
            .applicationVersion = {0,  1, 0},
            .engineName = "Forge",
            .engineVersion = { 0, 1, 0}
        };
        m_context = RHI::CreateVulkanContext(appInfo);

        auto windowSize = m_window->GetWindowSize();
        RHI::SwapchainCreateInfo swapchainInfo{
            .name = "Swapchain",
            .imageSize = { windowSize.width, windowSize.height },
            .imageUsage = RHI::ImageUsage::Color,
            .imageFormat = RHI::Format::RGBA8_UNORM,
            .minImageCount = 2,
            .presentMode = RHI::SwapchainPresentMode::Fifo,
            .win32Window = { m_window->GetNativeHandle() }
        };
        m_swapchain = m_context->CreateSwapchain(swapchainInfo);

        m_renderGraph = m_context->CreateRenderGraph();

        return ResultCode::Sucess;
    }

    void Renderer::Shutdown()
    {
        // m_context.destroySwapchain(m_swapchain);
        // RHI::DestroyContext(m_context),
        delete m_swapchain.release();
        delete m_context.release();
    }

    void Renderer::Render()
    {
        m_context->ExecuteRenderGraph(*m_renderGraph);

        auto resultCode = m_swapchain->Present();
        TL_ASSERT(RHI::IsSucess(resultCode), "Failed to present swapchain");
    }
} // namespace Examples::RPI