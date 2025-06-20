
#include "Renderer.hpp"

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

    ResultCode Renderer::Init(RHI::BackendType backend)
    {
        ZoneScoped;

        // m_window = window;

        m_device = CreateDevice(backend);

        // auto [width, height] = window->GetWindowSize();
        // RHI::SwapchainCreateInfo swapchainInfo{
        //     .name        = "Swapchain",
        //     .win32Window = {window->GetNativeHandle()},
        // };
        // m_swapchain = m_device->CreateSwapchain(swapchainInfo);
        // {
        //     RHI::SwapchainConfigureInfo configuration{
        //         .size        = {width, height},
        //         .imageCount  = 1,
        //         .imageUsage  = RHI::ImageUsage::Color,
        //         .format      = RHI::Format::RGBA8_UNORM,
        //         .presentMode = RHI::SwapchainPresentMode::Fifo,
        //         .alphaMode   = RHI::SwapchainAlphaMode::None
        //     };
        //     auto result = m_swapchain->Configure(configuration);
        //     TL_ASSERT(RHI::IsSuccess(result));
        // }

        m_renderGraph = m_device->CreateRenderGraph({});

#define TRY(expr)                                  \
    {                                              \
        auto result = (expr);                      \
        if (RHI::IsError(result))                  \
        {                                          \
            TL_LOG_ERROR("Renderer::Init failed"); \
            this->Shutdown();                      \
            return result;                         \
        }                                          \
    }

        TRY(m_allocators.uniformPool.Init(*m_device, {"uniform-buffers-pool", true, RHI::BufferUsage::Uniform, sizeof(GPU::SceneView) * 100}));
        TRY(m_allocators.storagePool.Init(*m_device, {"storage-buffers-pool", true, RHI::BufferUsage::Storage, sizeof(GPU::SceneView) * 100}));
        TRY(m_pipelineLibrary.Init(m_device));
        TRY(m_geometryBufferPool.Init(*m_device));
        TRY(m_deferredRenderer->Init(m_device));

#undef TRY

        return ResultCode::Success;
    }

    void Renderer::Shutdown()
    {
        ZoneScoped;

        m_deferredRenderer->Shutdown(m_device);

        m_geometryBufferPool.Shutdown();
        m_pipelineLibrary.Shutdown();
        m_device->DestroyRenderGraph(m_renderGraph);
        DestroyDevice(m_device);
    }

    PresentationViewport Renderer::CreatePresentationViewport(Examples::Window* window)
    {
        RHI::SwapchainCreateInfo swapchainInfo{
            .name        = "Swapchain",
            .win32Window = {window->GetNativeHandle()},
        };
        auto swapchain = m_device->CreateSwapchain(swapchainInfo);

        RHI::SwapchainConfigureInfo configuration{
            .size        = {window->GetWindowSize().width, window->GetWindowSize().height},
            .imageCount  = 1,
            .imageUsage  = RHI::ImageUsage::Color,
            .format      = RHI::Format::RGBA8_UNORM,
            .presentMode = RHI::SwapchainPresentMode::Fifo,
            .alphaMode   = RHI::SwapchainAlphaMode::None
        };
        auto result = swapchain->Configure(configuration);
        TL_ASSERT(RHI::IsSuccess(result));
        if (result != RHI::ResultCode::Success)
        {
            TL_LOG_ERROR("Failed to configure swapchain");
            m_device->DestroySwapchain(swapchain);
            return {};
        }

        return {
            .swapchain = swapchain,
            .window    = window,
        };
    }

    void Renderer::DestroyPresentationViewport(PresentationViewport& viewport)
    {
        TL_ASSERT(viewport.swapchain != nullptr, "Swapchain must not be null");
        m_device->DestroySwapchain(viewport.swapchain);
        viewport.swapchain = nullptr;
        viewport.window    = nullptr;
    }

    Scene* Renderer::CreateScene()
    {
        auto scene  = TL::Construct<Scene>();
        auto result = scene->Init(m_device);
        TL_ASSERT(result == RHI::ResultCode::Success);
        return scene;
    }

    void Renderer::DestroyScene(Scene* scene)
    {
        scene->Shutdown(m_device);
        TL::Destruct(scene);
    }

    void Renderer::Render(Scene* scene, const PresentationViewport& viewport)
    {
        // Update scene views
        {
            // ZoneScopedN("Update GPU Buffers");
            // m_sceneView->m_sceneViewUB.OnRender(m_renderGraph);
            // m_sceneView->m_drawList.OnRender(m_renderGraph);
        }

        m_renderGraph->BeginFrame(viewport.GetSize());
        auto swapchainBackbuffer = m_renderGraph->ImportSwapchain("swapchain-color-attachment", *viewport.swapchain, RHI::Format::RGBA8_UNORM);
        m_deferredRenderer->Render(m_device, m_renderGraph, scene, swapchainBackbuffer);
        m_renderGraph->EndFrame();
    }
} // namespace Engine
