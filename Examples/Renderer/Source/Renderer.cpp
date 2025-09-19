
#include "DeferredRenderer.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/PipelineLibrary.hpp"
#include "Renderer/Scene.hpp"

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

#include <TL/Literals.hpp>

namespace Engine
{
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

    TL::Error Renderer::init(RHI::BackendType backend)
    {
        ZoneScoped;

        switch (backend)
        {
#if RHI_BACKEND_D3D12
        case RHI::BackendType::DirectX12_2: m_device = RHI::CreateD3D12Device(); break;
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
                m_device = RHI::CreateVulkanDevice(applicationInfo);
            }
            break;
#endif
#if RHI_BACKEND_WEBGPU
        case RHI::BackendType::WebGPU: m_device = RHI::CreateWebGPUDevice(); break;
#endif
        default:
            return TL::Error("Failed to create RHI::Device with specified backend");
        }

        RHI::RenderGraphCreateInfo renderGraphCI{
            // .name = "primary-render-graph",
        };
        m_renderGraph = m_device->CreateRenderGraph(renderGraphCI);

        // Initialize renderer's systemss

        if (auto err = PipelineLibrary::ptr->init(m_device); err.IsError())
        {
            shutdown();
            return err;
        }

        if (auto err = GpuSceneData::ptr->init(m_device); err.IsError())
        {
            shutdown();
            return err;
        }

        if (auto err = DeferredRenderer::ptr->init(m_device); err.IsError())
        {
            shutdown();
            return err;
        }

        return TL::NoError;
    }

    void Renderer::shutdown()
    {
        ZoneScoped;

        DeferredRenderer::ptr->shutdown(m_device);
        GpuSceneData::ptr->shutdown();
        PipelineLibrary::ptr->shutdown();
        m_device->DestroyRenderGraph(m_renderGraph);
        DestroyDevice(m_device);
    }

    PresentationViewport Renderer::CreatePresentationViewport(Window* window)
    {
        RHI::SwapchainCreateInfo swapchainInfo{
            .name        = window->GetTitle().empty() ? window->GetTitle().data() : "swapchain",
            .win32Window = {window->GetNativeHandle()},
        };
        auto swapchain = m_device->CreateSwapchain(swapchainInfo);

        RHI::SwapchainConfigureInfo configuration{
            .size        = {window->GetSize().width, window->GetSize().height},
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
            .window    = window,
            .swapchain = swapchain,
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
        auto scene = TL::Construct<Scene>();
        auto result = scene->init();
        TL_ASSERT(result.IsSuccess(), result.GetMessage());
        return scene;
    }

    void Renderer::DestroyScene(Scene* scene)
    {
        // scene->Shutdown(m_device);
        TL::Destruct(scene);
    }

    void Renderer::Render(Scene* scene, const PresentationViewport& viewport)
    {
        m_renderGraph->BeginFrame(viewport.GetSize());

        auto swapchainBackbuffer = m_renderGraph->ImportSwapchain("swapchain-color-attachment", *viewport.swapchain, RHI::Format::RGBA8_UNORM);

        DeferredRenderer::ptr->render(m_device, m_renderGraph, scene, swapchainBackbuffer);

        m_renderGraph->EndFrame();

        // TODO: move out of here
        PipelineLibrary::ptr->updatePipelinesIfChanged();
    }
} // namespace Engine
