
#include "Renderer.hpp"

#include <Examples-Base/ApplicationBase.hpp>
#include <RHI-Vulkan/Loader.hpp>
#include <RHI-WebGPU/Loader.hpp>

#include "Scene.hpp"


namespace Engine
{
    ResultCode Renderer::Init(Examples::Window* window)
    {
        m_window = window;

        RHI::ApplicationInfo applicationInfo{
            .applicationName    = "Example",
            .applicationVersion = {0, 1, 0},
            .engineName         = "neonlights",
            .engineVersion      = {0, 1, 0},
        };
        // m_device = RHI::CreateVulkanDevice(applicationInfo);
        m_device = RHI::CreateWebGPUDevice();

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
        RHI::DestroyVulkanDevice(m_device);
    }

    void Renderer::RenderScene()
    {
        m_pipelineLibrary.ReloadInvalidatedShaders();
        // m_uniformBuffersAllocator.Reset();
        // m_storageBuffersAllocator.Reset();

        auto [width, height] = m_window->GetWindowSize();

        m_renderGraph->BeginFrame({width, height});
        m_renderGraph->AddPass({
            .name          = "main-buffer",
            .queue         = RHI::QueueType::Graphics,
            .setupCallback = [&](RHI::RenderGraph& renderGraph, RHI::Pass& pass)
            {
                m_renderGraph->UseColorAttachment(pass, {.view = m_gBuffer.colorAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                m_renderGraph->UseColorAttachment(pass, {.view = m_gBuffer.positionAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                m_renderGraph->UseColorAttachment(pass, {.view = m_gBuffer.normalsAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                m_renderGraph->UseColorAttachment(pass, {.view = m_gBuffer.materialAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                // Depth attachment
                m_renderGraph->UseDepthStencilAttachment(pass, {
                                                                   .view = m_gBuffer.depthAttachment, .clearValue = {1.0f, 0}
                });
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

                FillGBuffer(nullptr, commandList);
            },
        })->Resize({width, height});
        m_imguiRenderer.RenderDrawData(ImGui::GetDrawData(), *m_renderGraph, m_gBuffer.colorAttachment)->Resize({width, height});

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
