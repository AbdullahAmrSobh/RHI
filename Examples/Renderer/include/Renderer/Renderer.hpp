#pragma once

#include <RHI/RHI.hpp>

#include <Examples-Base/Window.hpp>

#include "Renderer/Common.hpp"
#include "Renderer/Geometry.hpp"

namespace Engine
{
    class Window;
    class Scene;

    class DeferredRenderer;

    struct PresentationViewport
    {
        Window*         window    = nullptr;
        RHI::Swapchain* swapchain = nullptr;

        RHI::ImageSize2D GetSize() const { return {window->GetSize().width, window->GetSize().height}; }
    };

    class Renderer final : public Singleton<Renderer>
    {
    public:
        TL::Error init(RHI::BackendType backend);
        void      shutdown();

        RHI::Device* GetDevice() const { return m_device; }

        RHI::RenderGraph* GetRenderGraph() const { return m_renderGraph; }

        PresentationViewport CreatePresentationViewport(Window* window);
        void                 DestroyPresentationViewport(PresentationViewport& viewport);

        Scene* CreateScene();
        void   DestroyScene(Scene* scene);

        void Render(Scene* scene, const PresentationViewport& viewport);

    private:
        RHI::Device*      m_device;
        RHI::RenderGraph* m_renderGraph;
    };
} // namespace Engine