#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

#include <glm/glm.hpp>

#include "BufferPool.hpp"
#include "Geometry.hpp"
#include "PipelineLibrary.hpp"
#include "Scene.hpp"

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
        inline static Renderer* ptr = nullptr;

        ResultCode Init(Examples::Window* window, RHI::BackendType backend);
        void       Shutdown();

        Scene* CreateScene();
        void   DestroyScene(Scene* scene);

        void Render(Scene* scene);

        void ProcessEvent(Examples::Event& event)
        {
            m_imguiPass.ProcessEvent(event);
        }

        void OnWindowResize();

    public:
        RHI::Device* m_device;

        RHI::RenderGraph* m_renderGraph;
    private:
        RHI::Swapchain*   m_swapchain;
        Examples::Window* m_window;

    public:
        struct Allocators
        {
            BufferPool uniformPool;
            BufferPool storagePool;
        } m_allocators;

    private:
        PipelineLibrary    m_pipelineLibrary;
        GeometryBufferPool m_geometryBufferPool;

        // Passes
        CullPass    m_cullPass;
        GBufferPass m_gbufferPass;
        ImGuiPass   m_imguiPass;
    };
} // namespace Engine