#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

#include <glm/glm.hpp>

#include "BufferPool.hpp"
#include "Mesh.hpp"
#include "PipelineLibrary.hpp"
#include "Scene.hpp"

#include "Passes/ImGuiPass.hpp"
#include "Passes/GBufferPass.hpp"

namespace Examples
{
    class Window;
}

namespace Engine
{
    class Scene;

    class Renderer
    {
    public:
        ResultCode Init(Examples::Window* window, RHI::BackendType backend);
        void       Shutdown();

        /// @brief Renders the scene.
        void RenderScene();

        void ProcessEvent(Examples::Event& event)
        {
            m_imguiPass.ProcessEvent(event);
        }

        void OnWindowResize();

    private:
        RHI::Device*      m_device;
        RHI::RenderGraph* m_renderGraph;
        RHI::Swapchain*   m_swapchain;
        Examples::Window* m_window;

        struct Allocators
        {
            BufferPool uniformPool;
            BufferPool storagePool;
        } m_allocators;

        PipelineLibrary    m_pipelineLibrary;
        GeometryBufferPool m_geometryBufferPool;

        // Per game/level
        StorageBuffer<GpuScene::MeshMaterialBindless>  m_materials;
        StorageBuffer<GpuScene::MeshUniform>           m_meshUniform;
        StorageBuffer<GpuScene::DrawIndexedParameters> m_drawParameters;

        // Passes
        ImGuiPass   m_imguiPass;
        // GBufferPass m_gbufferPass;

        SceneView* m_sceneView;

        RHI::Handle<RHI::BindGroup> m_bindGroup;
    };
} // namespace Engine