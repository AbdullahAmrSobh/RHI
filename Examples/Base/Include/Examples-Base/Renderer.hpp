#pragma once

#include "Examples-Base/Common.hpp"

#include <Examples-Base/ImGuiRenderer.hpp>

#include <RHI/RHI.hpp>

namespace Examples
{
    class Window;

    class Scene;

    class Renderer
    {
    public:
        Renderer();

        virtual ~Renderer();

        ResultCode Init(const class Window& window);

        void Shutdown();

        void Render(const Scene& scene);

        RHI::Result<Handle<RHI::Image>> CreateImageWithData(const RHI::ImageCreateInfo& createInfo, TL::Block content);
        RHI::Result<Handle<RHI::Buffer>> CreateBufferWithData(TL::Flags<RHI::BufferUsage> usageFlags, TL::Block content);
        Handle<RHI::Image> CreateImage(const char* filePath);

       TL::Ptr<Scene> CreateScene();

        virtual ResultCode OnInit() = 0;

        virtual void OnShutdown() = 0;

        virtual void OnRender(const Scene& scene) = 0;

    // protected:
        const Window* m_window;

       TL::Ptr<RHI::Context> m_context;
       TL::Ptr<RHI::Swapchain> m_swapchain;
       TL::Ptr<RHI::CommandPool> m_commandPool[2];
       TL::Ptr<RHI::Fence> m_frameFence[2];

        //TL::Ptr<>
    };

} // namespace Examples