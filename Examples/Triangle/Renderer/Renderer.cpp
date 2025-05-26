
#include "Renderer.hpp"

#include <Examples-Base/ApplicationBase.hpp>

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

    struct TestCode
    {
        RHI::Handle<RHI::BindGroup> bindGroup;
        Suballocation               primarySceneView;
    };

    static TestCode s_testData = {};

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

        result = m_allocators.uniformPool.Init(*m_device, {"uniform-buffers-pool", true, RHI::BufferUsage::Uniform, sizeof(GPU::SceneView) * 100});
        TL_ASSERT(RHI::IsSuccess(result));

        result = m_allocators.storagePool.Init(*m_device, {"storage-buffers-pool", true, RHI::BufferUsage::Storage, sizeof(GPU::SceneView) * 100});
        TL_ASSERT(RHI::IsSuccess(result));

        result = m_pipelineLibrary.Init(m_device);
        TL_ASSERT(RHI::IsSuccess(result));

        result = m_geometryBufferPool.Init(*m_device);
        TL_ASSERT(RHI::IsSuccess(result));

        // Init passes

        result = m_cullPass.Init(m_device);
        TL_ASSERT(RHI::IsSuccess(result));

        result = m_gbufferPass.Init(m_device);
        TL_ASSERT(RHI::IsSuccess(result));

        result = m_imguiPass.Init(m_device, RHI::Format::RGBA8_UNORM);
        TL_ASSERT(RHI::IsSuccess(result));

        result = m_drawList.Init(m_device, 1024);
        TL_ASSERT(RHI::IsSuccess(result));

        // Some plate test code
        {
            s_testData.bindGroup = m_device->CreateBindGroup({.name = "test-bind-group", .layout = m_pipelineLibrary.GetBindGroupLayout()});
            m_device->UpdateBindGroup(s_testData.bindGroup, {.buffers = {{BINDING_SCENEVIEW, 0, {{m_allocators.uniformPool.GetBuffer(), 0}}}}});
            s_testData.primarySceneView = m_allocators.uniformPool.Allocate(sizeof(GPU::SceneView), sizeof(GPU::SceneView)).GetValue();

            GPU::SceneView view{};
            view.worldToViewMatrix = glm::identity<glm::mat4>();
            view.viewToClipMatrix  = glm::identity<glm::mat4>();
            view.worldToClipMatrix = glm::identity<glm::mat4>();
            view.clipToWorldMatrix = glm::identity<glm::mat4>();
            m_device->BufferWrite(m_allocators.uniformPool.GetBuffer(), 0, TL::Block::Create(view));

            // Fullscreen quad (two triangles)
            // TL::Vector<uint32_t>  indcies  = {0, 1, 2, 2, 3, 0};
            // TL::Vector<glm::vec3> vertcies = {
            //     {-1.0f, -1.0f, 0.0f},
            //     {1.0f,  -1.0f, 0.0f},
            //     {1.0f,  1.0f,  0.0f},
            //     {-1.0f, 1.0f,  0.0f},
            // };
            // TL::Vector<glm::vec3> normals = {
            //     {0.0f, 0.0f, 1.0f},
            //     {0.0f, 0.0f, 1.0f},
            //     {0.0f, 0.0f, 1.0f},
            //     {0.0f, 0.0f, 1.0f}
            // };
            // TL::Vector<glm::vec2> uvs = {
            //     {0.0f, 0.0f},
            //     {1.0f, 0.0f},
            //     {1.0f, 1.0f},
            //     {0.0f, 1.0f}
            // };
            //  s_testData.m_staticMesh.push_back(m_geometryBufferPool.CreateStaticMeshLOD(indcies, vertcies, normals, uvs));

            // GPU::MeshUniform uniform {};
            // m_drawList.AddStaticMesh(s_testData.m_staticMesh[0], uniform);
        }

        return result;
    }

    void Renderer::Shutdown()
    {
        ZoneScoped;

        m_drawList.Shutdown(m_device);

        m_imguiPass.Shutdown();
        m_gbufferPass.Shutdown();

        m_geometryBufferPool.Shutdown();
        m_pipelineLibrary.Shutdown();

        m_device->DestroyRenderGraph(m_renderGraph);

        m_device->DestroySwapchain(m_swapchain);
        DestroyDevice(m_device);
    }

    void Renderer::RenderScene()
    {
        // Update scene views
        {
            ZoneScopedN("Update GPU Buffers");
            // m_sceneView->m_sceneViewUB.OnRender(m_renderGraph);
            // m_sceneView->m_drawList.OnRender(m_renderGraph);
        }

        auto [width, height] = m_window->GetWindowSize();
        m_renderGraph->BeginFrame({width, height});

        auto swapchainBackbuffer = m_renderGraph->ImportSwapchain("swapchain-color-attachment", *m_swapchain, RHI::Format::RGBA8_UNORM);

        m_cullPass.AddPass(m_renderGraph, m_drawList);

        m_gbufferPass.AddPass(m_renderGraph, m_cullPass, [this](RHI::CommandList& cmd)
            {
                auto pipeline = this->m_pipelineLibrary.GetGraphicsPipeline(ShaderNames::GBufferFill);
                cmd.BindGraphicsPipeline(pipeline, {{s_testData.bindGroup}});

                // Bind index buffer
                cmd.BindIndexBuffer(m_geometryBufferPool.GetAttribute(MeshAttributeType::Index), RHI::IndexType::uint32);
                cmd.BindVertexBuffers(
                    0,
                    {
                        m_geometryBufferPool.GetAttribute(MeshAttributeType::Position),
                        m_geometryBufferPool.GetAttribute(MeshAttributeType::Normal),
                        m_geometryBufferPool.GetAttribute(MeshAttributeType::TexCoord),
                        m_geometryBufferPool.GetAttribute(MeshAttributeType::TexCoord),
                    });

                auto argBuffer = RHI::BufferBindingInfo{m_renderGraph->GetBufferHandle(m_cullPass.m_drawIndirectArgs), 64};
                cmd.DrawIndexedIndirect(argBuffer, m_drawList.m_drawRequests.GetCountBindingInfo(), m_drawList.m_drawRequests.GetCount(), sizeof(RHI::DrawIndexedParameters));
            });

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
