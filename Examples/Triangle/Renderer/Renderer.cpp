
#include "Renderer.hpp"

#include <Examples-Base/ApplicationBase.hpp>

#include "Scene.hpp"

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
    ResultCode Renderer::Init(Examples::Window* window, RHI::BackendType backend)
    {
        m_window = window;

        m_backend = backend;
        switch (m_backend)
        {
#if RHI_BACKEND_D3D12
        case RHI::BackendType::DirectX12_2:
            {
            }
            break;

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
        case RHI::BackendType::WebGPU:
            m_device = RHI::CreateWebGPUDevice();
            break;
#endif
        default:
            TL_UNREACHABLE_MSG("Must select a backend");
            break;
            ;
        }

        auto [width, height] = window->GetWindowSize();
        RHI::SwapchainCreateInfo swapchainInfo{
            .name          = "Swapchain",
            .imageSize     = {width, height},
            .imageUsage    = RHI::ImageUsage::Color,
            .imageFormat   = RHI::Format::RGBA8_UNORM,
            .minImageCount = 3,
            .presentMode   = RHI::SwapchainPresentMode::Fifo,
            .win32Window   = {window->GetNativeHandle()},
        };
        m_swapchain = m_device->CreateSwapchain(swapchainInfo);

        m_renderGraph = m_device->CreateRenderGraph({});

        m_gBuffer.colorAttachment    = m_renderGraph->ImportSwapchain("Color", *m_swapchain, RHI::Format::RGBA8_UNORM);
        m_gBuffer.normalsAttachment  = m_renderGraph->CreateImage({
             .name       = "Normal",
             .usageFlags = RHI::ImageUsage::Depth,
             .type       = RHI::ImageType::Image2D,
             .size       = {width, height, 1},
             .format     = RHI::Format::RGBA32_FLOAT,
        });
        m_gBuffer.positionAttachment = m_renderGraph->CreateImage({
            .name       = "Position",
            .usageFlags = RHI::ImageUsage::Depth,
            .type       = RHI::ImageType::Image2D,
            .size       = {width, height, 1},
            .format     = RHI::Format::RGBA32_FLOAT,
        });
        m_gBuffer.materialAttachment = m_renderGraph->CreateImage({
            .name       = "Material",
            .usageFlags = RHI::ImageUsage::Depth,
            .type       = RHI::ImageType::Image2D,
            .size       = {width, height, 1},
            .format     = RHI::Format::RG8_UNORM,
        });
        m_gBuffer.depthAttachment    = m_renderGraph->CreateImage({
               .name       = "Depth",
               .usageFlags = RHI::ImageUsage::Depth,
               .type       = RHI::ImageType::Image2D,
               .size       = {width, height, 1},
               .format     = RHI::Format::D32,
        });

        ResultCode resultCode;

#define RESULT_CHECK(resultCode) \
    if (IsError(resultCode))     \
    {                            \
        Shutdown();              \
        return resultCode;       \
    }

        resultCode = m_pipelineLibrary.Init(m_device);
        RESULT_CHECK(resultCode);

        resultCode = m_imguiRenderer.Init(m_device, RHI::Format::RGBA8_UNORM);
        RESULT_CHECK(resultCode);

        resultCode = m_unifiedGeometryBufferPool.Init(*m_device);
        RESULT_CHECK(resultCode);

#undef RESULT_CHECK

        m_testTriangleMesh = m_unifiedGeometryBufferPool.CreateStaticMeshLOD(
            {
                0, 1, 2
        },
            {
                {0.5f, 0.5f, 0.0f},
                {0.0f, -0.5f, 0.0f},
                {-0.5f, 0.5f, 0.0f},
            },
            {
                {0.0f, -1.0f, 0.0f},
                {1.0f, 1.0f, 0.0f},
                {-1.0f, 1.0f, 0.0f},
            },
            {
                {0.0f, -0.5f},
                {0.5f, 0.5f},
                {-0.5f, 0.5f},
            });

        return ResultCode::Success;
    }

    void Renderer::Shutdown()
    {
        m_renderGraph->DestroyImage(m_gBuffer.depthAttachment);
        m_renderGraph->DestroyImage(m_gBuffer.materialAttachment);
        m_renderGraph->DestroyImage(m_gBuffer.positionAttachment);
        m_renderGraph->DestroyImage(m_gBuffer.normalsAttachment);
        m_renderGraph->DestroyImage(m_gBuffer.colorAttachment);
        m_device->DestroyRenderGraph(m_renderGraph);

        m_unifiedGeometryBufferPool.Shutdown();
        m_imguiRenderer.Shutdown();
        m_pipelineLibrary.Shutdown();

        m_device->DestroySwapchain(m_swapchain);

        switch (m_backend)
        {
        case RHI::BackendType::DirectX12_2:
#if RHI_BACKEND_D3D12
#endif
        case RHI::BackendType::Vulkan1_3:
#if RHI_BACKEND_VULKAN
            RHI::DestroyVulkanDevice(m_device);
#endif
            break;
        case RHI::BackendType::WebGPU:
#if RHI_BACKEND_WEBGPU
            RHI::DestroyWebGPUDevice(m_device);
#endif
            break;
        default:
            TL_UNREACHABLE_MSG("Must select a backend");
            break;
            ;
        };
    }

    Scene* Renderer::CreateScene()
    {
        auto scene = m_activeScenes.emplace_back(new Scene());
        auto res   = scene->Init(m_device);
        return scene;
    }

    void Renderer::DestroyScene(Scene* scene)
    {
        scene->Shutdown();

        auto it = std::find(m_activeScenes.begin(), m_activeScenes.end(), scene);
        if (it != m_activeScenes.end())
        {
            delete *it;
            m_activeScenes.erase(it);
        }
    }

    void Renderer::RenderScene()
    {
        m_pipelineLibrary.ReloadInvalidatedShaders();
        // m_uniformBuffersAllocator.Reset();
        // m_storageBuffersAllocator.Reset();

        auto [width, height] = m_window->GetWindowSize();

        m_renderGraph->BeginFrame({width, height});

        // m_renderGraph->AddPass({
        //     .name          = "prepare-draw",
        //     .queue         = RHI::QueueType::Compute,
        //     .setupCallback = [&](RHI::RenderGraph& renderGraph, RHI::Pass& pass)
        //     {
        //     },
        //     .compileCallback = [&](RHI::RenderGraph& renderGraph, RHI::Pass& pass)
        //     {
        //     },
        //     .executeCallback = [&](RHI::CommandList& commandList)
        //     {
        //         commandList.BindComputePipeline(Handle<ComputePipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups)
        //             commandList.Dispatch({});
        //     },
        // });

        m_renderGraph->AddPass({
            .name          = "main-buffer",
            .queue         = RHI::QueueType::Graphics,
            .size          = {width, height},
            .setupCallback = [&](RHI::RenderGraph& renderGraph, RHI::Pass& pass)
            {
                m_renderGraph->UseColorAttachment(pass, {.view = m_gBuffer.colorAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                m_renderGraph->UseColorAttachment(pass, {.view = m_gBuffer.positionAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                m_renderGraph->UseColorAttachment(pass, {.view = m_gBuffer.normalsAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                m_renderGraph->UseColorAttachment(pass, {.view = m_gBuffer.materialAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                // Depth attachment
                m_renderGraph->UseDepthStencilAttachment(pass, {
                                                                   .view = m_gBuffer.depthAttachment, .clearValue = {1.0f, 0}});
                              },
            .compileCallback = [&](RHI::RenderGraph& renderGraph, RHI::Pass& pass)
            {
                              },
            .executeCallback = [&](RHI::CommandList& commandList)
            {
                commandList.SetViewport({
                    .offsetX  = 0.0f,
                    .offsetY  = 0.0f,
                    .width    = (float)width,
                    .height   = (float)height,
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f,
                });
                commandList.SetScissor({
                    .offsetX = 0,
                    .offsetY = 0,
                    .width   = width,
                    .height  = height,
                });

                for (auto scene : m_activeScenes)
                {
                    FillGBuffer(scene, commandList);
                }
                              },
        });

        // ImGui
        m_imguiRenderer.RenderDrawData(ImGui::GetDrawData(), *m_renderGraph, {width, height}, m_gBuffer.colorAttachment);

        m_renderGraph->EndFrame();
        m_device->CollectResources();
    }

    void Renderer::OnWindowResize()
    {
        auto [width, height] = m_window->GetWindowSize();
        auto res             = m_swapchain->Recreate({width, height});
        TL_ASSERT(RHI::IsSuccess(res));
    }

    void Renderer::FillGBuffer(const Scene* scene, RHI::CommandList& commandList)
    {
        auto pipeline = m_pipelineLibrary.GetGraphicsPipeline(kGBufferFill);
        commandList.BindGraphicsPipeline(m_pipelineLibrary.GetGraphicsPipeline(kGBufferFill), {});
        commandList.BindIndexBuffer(m_unifiedGeometryBufferPool.GetAttributeBindingInfo(MeshAttributeType::Index), RHI::IndexType::uint32);
        commandList.BindVertexBuffers(
            0,
            {
                m_unifiedGeometryBufferPool.GetAttributeBindingInfo(MeshAttributeType::Position),
                m_unifiedGeometryBufferPool.GetAttributeBindingInfo(MeshAttributeType::Normal),
                m_unifiedGeometryBufferPool.GetAttributeBindingInfo(MeshAttributeType::TexCoord),
                scene->GetTransformsInstanceBuffer(),
            });
        // commandList.DrawIndexed({
        //     .indexCount    = m_testTriangleMesh->GetIndexCount(),
        //     .instanceCount = 1,
        //     .firstIndex    = m_testTriangleMesh->GetIndexOffset(),
        //     .vertexOffset  = (I32)m_testTriangleMesh->GetVertexOffset(),
        //     .firstInstance = 0,
        // });
        commandList.DrawIndexedIndirect(m_unifiedGeometryBufferPool.m_drawParams.GetBindingInfo(), {}, 1, sizeof(RHI::DrawIndexedParameters));
    }
} // namespace Engine
