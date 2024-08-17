#include <Examples-Base/ApplicationBase.hpp>

#include <RPI/Renderer.hpp>

#include <tracy/Tracy.hpp>

#include "Camera.hpp"

namespace Examples
{
    class BasicRenderer final : public ApplicationBase
    {
    public:
        BasicRenderer()
            : ApplicationBase("Hello, Triangle", 1600, 1200)
        {
        }

        void OnInit() override
        {
            m_renderer = TL::Ptr<RPI::Renderer>(RPI::CreateDeferredRenderer());

            auto result = m_renderer->Init(*m_window);
            TL_ASSERT(IsSucess(result));

            m_scene = m_renderer->CreateScene();
            LoadScene(*m_renderer, *m_scene, m_launchSettings.sceneFileLocation.c_str());

            m_camera.m_window = m_window.get();
            m_camera.SetPerspective(60.0f, 1600.0f / 1200.0f, 0.1f, 10000.0f);
            m_camera.SetRotationSpeed(0.0002f);
        }

        void OnShutdown() override
        {
            m_renderer->Shutdown();
        }

        void OnUpdate(Timestep timestep) override
        {
            m_camera.Update(timestep);
            m_scene->m_viewMatrix = m_camera.GetView();
            m_scene->m_projectionMatrix = m_camera.GetProjection();
        }

        void Render() override
        {
            m_renderer->Render(*m_scene);
        }

        void OnEvent(Event& e) override
        {
            m_camera.ProcessEvent(e);
        }

        TL::Ptr<RPI::Renderer> m_renderer;
        TL::Ptr<RPI::Scene> m_scene;
        Camera m_camera;
    };
} // namespace Examples

#include <Examples-Base/Entry.hpp>

int main(int argc, const char* argv[])
{
    using namespace Examples;
    TL::Span args{ argv, (size_t)argc };
    return Entry<BasicRenderer>(args);
}
