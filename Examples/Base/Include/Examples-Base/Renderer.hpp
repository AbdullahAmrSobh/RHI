#pragma once

#include "Examples-Base/Common.hpp"

#include <Examples-Base/ImGuiRenderer.hpp>

#include <RHI/RHI.hpp>

namespace Examples
{
    class Window;

    class Renderer
    {
    public:
        virtual ~Renderer() = default;

        ResultCode Init(const class Window& window);
        void Shutdown();

        void Render();

        virtual ResultCode OnInit() = 0;

        virtual void OnShutdown() = 0;

        virtual void OnRender() = 0;

    protected:
        const Window* m_window;

        Ptr<RHI::Context> m_context;
        Ptr<RHI::RenderGraph> m_renderGraph;
        Ptr<RHI::Swapchain> m_swapchain;
        Ptr<RHI::CommandPool> m_commandPool[2];

        Ptr<ImGuiRenderer> m_imguiRenderer;
    };
} // namespace Examples