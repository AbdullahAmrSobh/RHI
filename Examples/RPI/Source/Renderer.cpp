#include "RPI/Renderer.hpp"

#include <Examples-Base/Window.hpp>

#include <RHI/RHI.hpp>
#include <RHI-Vulkan/Loader.hpp>

#include <TL/Log.hpp>

namespace Examples::RPI
{
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

        m_outputAttachment = m_renderGraph->ImportSwapchain("swapchain-output", *m_swapchain);

        for (auto& frame : m_frameRingbuffer)
        {
            frame.m_fence = m_context->CreateFence();
            frame.m_commandPool = m_context->CreateCommandPool(RHI::CommandPoolFlags::Transient);
        }

        return OnInit();
    }

    void Renderer::Shutdown()
    {
        OnShutdown();

        for (auto& frame : m_frameRingbuffer)
        {
            delete frame.m_fence.release();
            delete frame.m_commandPool.release();
        }

        delete m_swapchain.release();
        delete m_context.release();
    }

    void Renderer::Render()
    {
        auto& frame = m_frameRingbuffer.Get();
        if (frame.m_fence->GetState() != RHI::FenceState::Signaled)
        {
            frame.m_fence->Wait(UINT64_MAX);
        }
        frame.m_fence->Reset();

        OnRender();

        m_context->ExecuteRenderGraph(*m_renderGraph, m_frameRingbuffer.Get().m_fence.get());

        auto resultCode = m_swapchain->Present();
        TL_ASSERT(RHI::IsSucess(resultCode), "Failed to present swapchain");
    }
} // namespace Examples::RPI