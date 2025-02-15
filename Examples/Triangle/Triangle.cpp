// Fix swapchain resizing and moving into other monitor
// Fix validation errors and change API if needed!
// Fix all memory and resource leaks!

#include <RHI/RHI.hpp>

#include <TL/Defer.hpp>
#include <TL/FileSystem/FileSystem.hpp>
#include <TL/Log.hpp>
#include <TL/Utils.hpp>
// #include <TL/Allocator/MemPlumber.hpp>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <tracy/Tracy.hpp>

#include "Camera.hpp"
#include "Examples-Base/ApplicationBase.hpp"
#include "Renderer/SceneRenderer.hpp"

class Playground final : public ApplicationBase
{
public:
    TL::Ptr<Engine::Renderer> m_renderer;

    Playground()
        : ApplicationBase("Playground", 1600, 900)
        , m_renderer(TL::CreatePtr<Engine::Renderer>())
    {
    }

    void OnInit() override
    {
        ZoneScoped;

        auto result = m_renderer->Init(m_window.get());
    }

    void OnShutdown() override
    {
        ZoneScoped;

        m_renderer->Shutdown();
    }

    void OnUpdate(Timestep ts) override
    {
        ZoneScoped;
    }

    void Render() override
    {
        ZoneScoped;

        auto [width, height] = m_window->GetWindowSize();
        ImGuiIO& io          = ImGui::GetIO();
        io.DisplaySize.x     = float(width);
        io.DisplaySize.y     = float(height);

        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();

        m_renderer->RenderScene();

        FrameMark;
    }

    void OnEvent(Event& event) override
    {
        ZoneScoped;
        switch (event.GetEventType())
        {
        case EventType::None:
        case EventType::WindowClose:
        case EventType::WindowResize:
            m_renderer->OnWindowResize();
            break;
        case EventType::WindowFocus:
        case EventType::WindowLostFocus:
        case EventType::WindowMoved:
        case EventType::AppTick:
        case EventType::AppUpdate:
        case EventType::AppRender:
        case EventType::KeyPressed:
        case EventType::KeyReleased:
        case EventType::KeyTyped:
        case EventType::MouseButtonPressed:
        case EventType::MouseButtonReleased:
        case EventType::MouseMoved:
        case EventType::MouseScrolled:
        default:
            break;
        }
        m_renderer->ProcessEvent(event);
    }
};

#include <Examples-Base/Entry.hpp>

int main(int argc, const char* argv[])
{
    using namespace Examples;
    TL::Span args{argv, (size_t)argc};
    // TL::MemPlumber::start();
    auto     result = Entry<Playground>(args);
    // size_t memLeakCount, memLeakSize;
    // TL::MemPlumber::memLeakCheck(memLeakCount, memLeakSize);
    return result;
}
