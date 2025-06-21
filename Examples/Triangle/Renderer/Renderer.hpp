#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>

#include "BufferPool.hpp"
#include "Geometry.hpp"
#include "PipelineLibrary.hpp"
#include "Scene.hpp"

#include "RendererImpl/DeferredRenderer.hpp"
#include <Examples-Base/Window.hpp>

namespace Engine
{
    class Window;
    class Scene;

    struct PresentationViewport
    {
        Window*         window    = nullptr;
        RHI::Swapchain* swapchain = nullptr;

        RHI::ImageSize2D GetSize() const { return {window->GetSize().width, window->GetSize().height}; }
    };

    // Renderer interface
    class Renderer final : public Singleton<Renderer>
    {
    public:
        ResultCode Init(RHI::BackendType backend);
        void       Shutdown();

        RHI::Device* GetDevice() const { return m_device; }

        RHI::RenderGraph* GetRenderGraph() const { return m_renderGraph; }

        template<typename T>
        UniformBuffer<T> AllocateUniformBuffer(T content)
        {
            return m_allocators.uniformPool.AllocateUniformBuffer(content);
        }

        template<typename T>
        UniformBuffer<T> AllocateUniformBuffer()
        {
            return m_allocators.uniformPool.AllocateUniformBuffer<T>();
        }

        PresentationViewport CreatePresentationViewport(Window* window);
        void                 DestroyPresentationViewport(PresentationViewport& viewport);

        Scene* CreateScene();
        void   DestroyScene(Scene* scene);

        void Render(Scene* scene, const PresentationViewport& viewport);

    private:
        RHI::Device*      m_device;
        RHI::RenderGraph* m_renderGraph;

        struct Allocators
        {
            BufferPool uniformPool;
            BufferPool storagePool;
        } m_allocators;

        PipelineLibrary    m_pipelineLibrary;
        GeometryBufferPool m_geometryBufferPool;
        TL::Ptr<DeferredRenderer> m_deferredRenderer = TL::CreatePtr<DeferredRenderer>();
    };
} // namespace Engine