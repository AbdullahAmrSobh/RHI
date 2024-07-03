#pragma once

#include <Examples-Base/Timestep.hpp>
#include <Examples-Base/Camera.hpp>
// #include <Examples-Base/ImGuiRenderer.hpp>
#include <Examples-Base/SceneGraph.hpp>
#include <Examples-Base/Window.hpp>

#include <RHI/RHI.hpp>

#include "Examples-Base/Common.hpp"

class ApplicationBase
{
public:
    ApplicationBase(std::string name, uint32_t width, uint32_t height);
    virtual ~ApplicationBase() = default;

    template<typename ExampleType>
    static int Entry(TL::Span<const char*> args);

private:
    void Init();

    void Shutdown();

    void Run();

    void ProcessInput();

    virtual void OnInit() = 0;

    virtual void OnShutdown() = 0;

    virtual void OnUpdate(Timestep timestep) = 0;

protected:
    Camera m_camera;

    Ptr<RHI::Context> m_context;

    Ptr<RHI::Swapchain> m_swapchain;

    // Ptr<ImGuiRenderer> m_imguiRenderer;

    Ptr<Window> m_window;

    Ptr<Scene> m_scene;
};
