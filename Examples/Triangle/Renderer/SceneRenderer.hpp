#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

#include <glm/glm.hpp>

#include "Bindless.hpp"
#include "ImGuiRenderer.hpp"
#include "Mesh.hpp"
#include "PipelineLibrary.hpp"

#ifndef ENGINE_EXPORT
    #define ENGINE_EXPORT
#endif

namespace Examples
{
    class Window;
}

namespace Engine
{
    struct DrawRequest
    {
        RHI::Handle<RHI::PipelineLayout>   layout;
        RHI::Handle<RHI::GraphicsPipeline> pipeline;
        RHI::Handle<RHI::BindGroup>        bindGroup;
        RHI::DrawParameters                parameters;
    };

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

    private:
        void BindGraphicsPassResources(RHI::CommandList& commandList, TL::Span<const DrawRequest> drawCalls);

        void FillGBuffer(RHI::CommandList& commandList);

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

        // ///< Allocator for uniform buffers.
        // BufferSuballocator        m_uniformBuffersAllocator;
        // ///< Allocator for storage buffers.
        // BufferSuballocator        m_storageBuffersAllocator;
        // ///< Bindless textures resource manager.
        // Bindless                  m_bindless;

        ///< Library of rendering pipelines.
        PipelineLibrary           m_pipelineLibrary;
        ///< ImGui renderer.
        ImGuiRenderer             m_imguiRenderer;
        ///< Geometry buffer for meshes.
        MeshUnifiedGeometryBuffer m_unifiedGeometryBuffer;

    private:
        StaticMesh m_testTriangleMesh;
    };
} // namespace Engine