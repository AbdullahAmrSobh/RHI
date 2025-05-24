
#include "Renderer.hpp"

#include <Examples-Base/ApplicationBase.hpp>

#include "Scene.hpp"

#include <tracy/Tracy.hpp>

#if RHI_BACKEND_D3D12
    #include <RHI-D3D12/Loader.hpp>
#endif

#if RHI_BACKEND_VULKAN
    #include <RHI-Vulkan/Loader.hpp>
#endif

#if RHI_BACKEND_WEBGPU
    #include <RHI-WebGPU/Loader.hpp>
#endif

namespace Engine
{
    inline static RHI::Device* CreateDevice(RHI::BackendType backend)
    {
        switch (backend)
        {
#if RHI_BACKEND_D3D12
        case RHI::BackendType::DirectX12_2: return RHI::CreateD3D12Device(); break;
#endif
#if RHI_BACKEND_VULKAN
        case RHI::BackendType::Vulkan1_3:
            {
                RHI::ApplicationInfo applicationInfo{
                    .applicationName    = "Example",
                    .applicationVersion = {0, 1, 0},
                    .engineName         = "Neons",
                    .engineVersion      = {0, 1, 0},
                };
                return RHI::CreateVulkanDevice(applicationInfo);
            }
            break;
#endif
#if RHI_BACKEND_WEBGPU
        case RHI::BackendType::WebGPU: return RHI::CreateWebGPUDevice(); break;
#endif
        default: TL_UNREACHABLE_MSG("Must select a backend");
        }

        return nullptr;
    }

    inline static void DestroyDevice(RHI::Device* device)
    {
        switch (device->GetBackend())
        {
#if RHI_BACKEND_D3D12
        case RHI::BackendType::DirectX12_2: RHI::DestroyD3D12Device(device); return;
#endif
#if RHI_BACKEND_VULKAN
        case RHI::BackendType::Vulkan1_3: RHI::DestroyVulkanDevice(device); return;
#endif
#if RHI_BACKEND_WEBGPU
        case RHI::BackendType::WebGPU: RHI::DestroyWebGPUDevice(device); return;
#endif
        default: TL_UNREACHABLE_MSG("Must select a backend");
        };
    }

    ResultCode Renderer::Init(Examples::Window* window, RHI::BackendType backend)
    {
        ZoneScoped;

        m_window = window;

        m_device = CreateDevice(backend);

        auto [width, height] = window->GetWindowSize();
        RHI::SwapchainCreateInfo swapchainInfo{
            .name        = "Swapchain",
            .win32Window = {window->GetNativeHandle()},
        };
        m_swapchain = m_device->CreateSwapchain(swapchainInfo);
        {
            RHI::SwapchainConfigureInfo configuration{
                .size        = {width, height},
                .imageCount  = 1,
                .imageUsage  = RHI::ImageUsage::Color,
                .format      = RHI::Format::RGBA8_UNORM,
                .presentMode = RHI::SwapchainPresentMode::Fifo,
                .alphaMode   = RHI::SwapchainAlphaMode::None
            };
            auto result = m_swapchain->Configure(configuration);
            TL_ASSERT(RHI::IsSuccess(result));
        }

        m_renderGraph = m_device->CreateRenderGraph({});

        RHI::ResultCode result;

        result = m_pipelineLibrary.Init(m_device);
        TL_ASSERT(RHI::IsSuccess(result));

        result = m_geometryBufferPool.Init(*m_device);
        TL_ASSERT(RHI::IsSuccess(result));

        // Init passes

        // result = m_gbufferPass.Init(m_device);
        // TL_ASSERT(RHI::IsSuccess(result));

        result = m_imguiPass.Init(m_device, RHI::Format::RGBA8_UNORM);
        TL_ASSERT(RHI::IsSuccess(result));

        return result;
    }

    void Renderer::Shutdown()
    {
        ZoneScoped;

        // m_gbufferPass.Shutdown();
        m_imguiPass.Shutdown();

        m_geometryBufferPool.Shutdown();
        m_pipelineLibrary.Shutdown();

        m_device->DestroyRenderGraph(m_renderGraph);

        m_device->DestroySwapchain(m_swapchain);
        DestroyDevice(m_device);
    }

    void Renderer::RenderScene()
    {
        m_pipelineLibrary.ReloadInvalidatedShaders();

        // Update scene views
        {
            ZoneScopedN("Update GPU Buffers");
            // m_sceneView->m_sceneViewUB.OnRender(m_renderGraph);
            // m_sceneView->m_drawList.OnRender(m_renderGraph);
        }

        auto [width, height] = m_window->GetWindowSize();
        m_renderGraph->BeginFrame({width, height});

        auto swapchainBackbuffer = m_renderGraph->ImportSwapchain("swapchain-color-attachment", *m_swapchain, RHI::Format::RGBA8_UNORM);

        // m_gbufferPass.AddPass(m_renderGraph);

        if (m_imguiPass.Enabled())
        {
            m_imguiPass.AddPass(m_renderGraph, swapchainBackbuffer, ImGui::GetDrawData());
        }

        m_renderGraph->EndFrame();

        TL_MAYBE_UNUSED auto result = m_swapchain->Present();
        if (RHI::IsError(result))
        {
            result = m_swapchain->Configure({
                .size        = {width, height},
                .imageCount  = 1,
                .imageUsage  = RHI::ImageUsage::Color,
                .format      = RHI::Format::RGBA8_UNORM,
                .presentMode = RHI::SwapchainPresentMode::Fifo,
                .alphaMode   = RHI::SwapchainAlphaMode::None,
            });
            TL_ASSERT(RHI::IsSuccess(result));
        }
    }

    void Renderer::OnWindowResize()
    {
        auto [width, height] = m_window->GetWindowSize();
        auto result          = m_swapchain->Resize({width, height});
        TL_ASSERT(RHI::IsSuccess(result));
    }
} // namespace Engine
