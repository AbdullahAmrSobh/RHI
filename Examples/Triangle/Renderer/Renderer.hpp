#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

#include <glm/glm.hpp>

#include "BufferPool.hpp"
#include "ImGuiRenderer.hpp"
#include "Mesh.hpp"
#include "PipelineLibrary.hpp"
#include "Scene.hpp"

#ifndef ENGINE_EXPORT
    #define ENGINE_EXPORT
#endif

namespace Examples
{
    class Window;
}

namespace Engine
{
    class Scene;

    /// @class Renderer
    /// @brief Manages the rendering process, including initialization, rendering, and shutdown.
    class Renderer
    {
    public:
        /// @brief Initializes the renderer.
        /// @return ResultCode indicating success or failure.
        ResultCode Init(Examples::Window* window);

        /// @brief Shuts down the renderer and releases resources.
        void Shutdown();

        /// @brief Renders the scene.
        void RenderScene();

        void ProcessEvent(Examples::Event& event)
        {
            m_imguiRenderer.ProcessEvent(event);
        }

        void OnWindowResize();

    private:

        void FillGBuffer(const Scene* scene, RHI::CommandList& commandList);

    private:
        ///< Pointer to the rendering device.
        RHI::Device*      m_device;
        ///< Pointer to the render graph.
        RHI::RenderGraph* m_renderGraph;
        ///< Pointer to the swapchain.
        RHI::Swapchain*   m_swapchain;

        Examples::Window* m_window;

        /// Rendering Pipeline stuff

        struct GBufferData
        {
            RHI::RenderGraphImage* colorAttachment;
            RHI::RenderGraphImage* positionAttachment;
            RHI::RenderGraphImage* normalsAttachment;
            RHI::RenderGraphImage* materialAttachment;
            RHI::RenderGraphImage* depthAttachment;
        } m_gBuffer;

        /// @todo: maybe uniformBuffersAllocator, and storageBuffersAllocator should be part of the render graph?

        ///< Allocator for uniform buffers.
        BufferPool m_uniformBuffersAllocator;

        ///< Allocator for storage buffers.
        BufferPool m_storageBuffersAllocator;

        ///< Library of rendering pipelines.
        PipelineLibrary           m_pipelineLibrary;
        ///< ImGui renderer.
        ImGuiRenderer             m_imguiRenderer;
        ///< Geometry buffer for meshes.
        UnifiedGeometryBufferPool m_unifiedGeometryBufferPool;

    private:
        StaticMeshLOD* m_testTriangleMesh;
    };
} // namespace Engine