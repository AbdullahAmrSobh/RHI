#pragma once

#include <RHI/RHI.hpp>

namespace Examples
{
    class Window;
}

namespace Examples::RPI
{
    template<typename T>
    using Handle = RHI::Handle<T>;

    enum class ResultCode
    {
        Sucess,
        Error,
    };

    class Renderer
    {
    public:
        Renderer() = default;
        ~Renderer() = default;

        static TL::Ptr<Renderer> CreateDeferred();

        ResultCode Init(const Window& window);
        void Shutdown();
        void Render();

    protected:
        const Window* m_window;

        TL::Ptr<RHI::Context> m_context;
        TL::Ptr<RHI::Swapchain> m_swapchain;
        TL::Ptr<RHI::RenderGraph> m_renderGraph;

        struct FrameContext
        {
            TL::Ptr<RHI::Fence> m_fence;
            TL::Ptr<RHI::CommandPool> m_commandPool;
        };

        FrameContext m_frameRingbuffer[RHI::Swapchain::MaxImageCount];
    };
} // namespace Examples::RPI