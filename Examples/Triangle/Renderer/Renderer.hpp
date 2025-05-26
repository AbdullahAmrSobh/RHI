#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

#include <glm/glm.hpp>

#include "BufferPool.hpp"
#include "Mesh.hpp"
#include "PipelineLibrary.hpp"
#include "Scene.hpp"

#include "DrawList.hpp"

#include "Passes/CullPass.hpp"
#include "Passes/GBufferPass.hpp"
#include "Passes/ImGuiPass.hpp"

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
        // StorageBuffer<GPU::MeshMaterialBindless>  m_materials;

        public:
        IndirectDrawList m_drawList;
        private:

        // Passes
        CullPass    m_cullPass;
        GBufferPass m_gbufferPass;
        ImGuiPass   m_imguiPass;
    };
} // namespace Engine