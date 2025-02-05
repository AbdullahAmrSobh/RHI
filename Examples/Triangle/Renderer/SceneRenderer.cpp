
#include "SceneRenderer.hpp"

#include <Examples-Base/ApplicationBase.hpp>
#include <RHI-Vulkan/Loader.hpp>

namespace Engine
{
    ResultCode Renderer::Init(Examples::Window* window)
    {
        m_window = window;

        RHI::ApplicationInfo applicationInfo{
            .applicationName    = "Example",
            .applicationVersion = {0, 1, 0},
            .engineName         = "Forge",
            .engineVersion      = {0, 1, 0},
        };
        m_device = RHI::CreateVulkanDevice(applicationInfo);

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

        m_renderGraph = m_device->CreateRenderGraph();

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

        resultCode = m_unifiedGeometryBuffer.Init(*m_device, 64);
        RESULT_CHECK(resultCode);

#undef RESULT_CHECK

        m_testTriangleMesh = m_unifiedGeometryBuffer.CreateMesh(
            "Triangle",
            {0, 1, 2},
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
        [[maybe_unused]] auto pass = m_renderGraph->AddPass({
            .name          = "main-buffer",
            .queue         = RHI::QueueType::Graphics,
            .setupCallback = [&](RHI::RenderGraph& renderGraph, RHI::Pass& pass)
            {
                m_renderGraph->UseRenderTarget(pass, {.attachment = m_gBuffer.colorAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                m_renderGraph->UseRenderTarget(pass, {.attachment = m_gBuffer.positionAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                m_renderGraph->UseRenderTarget(pass, {.attachment = m_gBuffer.normalsAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                m_renderGraph->UseRenderTarget(pass, {.attachment = m_gBuffer.materialAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                m_renderGraph->UseRenderTarget(pass, {.attachment = m_gBuffer.depthAttachment, .clearValue = {.depthStencil = {1.0f, 0}}});
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

                FillGBuffer(commandList);

                m_imguiRenderer.RenderDrawData(ImGui::GetDrawData(), commandList);
            },
        });
        pass->Resize({width, height});

        m_renderGraph->EndFrame();
        m_device->CollectResources();
    }

    void Renderer::BindGraphicsPassResources(RHI::CommandList& commandList, TL::Span<const DrawRequest> drawCalls)
    {
        // commandList.BindIndexBuffer(m_unifiedGeometryBuffer.GetIndexBuffer(), RHI::IndexType::uint32);
        // commandList.BindVertexBuffers(
        //     0,
        //     {
        //         m_unifiedGeometryBuffer.GetVertexBuffer(MeshAttributeType::Position),
        //         m_unifiedGeometryBuffer.GetVertexBuffer(MeshAttributeType::Normal),
        //         m_unifiedGeometryBuffer.GetVertexBuffer(MeshAttributeType::Uv),
        //     });

        // // TODO: Gpu driven
        // for (auto drawCall : drawCalls)
        // {
        //     commandList.BindGraphicsPipeline(drawCall.pipeline, {{.bindGroup = drawCall.bindGroup}});
        //     commandList.Draw(drawCall.parameters);
        // }
    }

    void Renderer::FillGBuffer(RHI::CommandList& commandList)
    {
        commandList.BindIndexBuffer(m_unifiedGeometryBuffer.GetIndexBuffer(), RHI::IndexType::uint32);
        commandList.BindVertexBuffers(
            0,
            {
                m_unifiedGeometryBuffer.GetVertexBuffer(MeshAttributeType::Position),
                m_unifiedGeometryBuffer.GetVertexBuffer(MeshAttributeType::Normal),
                m_unifiedGeometryBuffer.GetVertexBuffer(MeshAttributeType::Uv),
            });
        commandList.BindGraphicsPipeline(m_pipelineLibrary.GetGraphicsPipeline(kGBufferFill), {});
        commandList.DrawIndexed(m_testTriangleMesh.parameters);
    }
} // namespace Engine
