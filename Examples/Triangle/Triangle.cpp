#include <Examples-Base/ApplicationBase.hpp>
#include <Examples-Base/Scene.hpp>
#include <Examples-Base/Camera.hpp>

#include <tracy/Tracy.hpp>

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
            m_camera.m_window = m_window.get();
            m_camera.SetPerspective(60.0f, 1600.0f / 1200.0f, 0.1f, 10000.0f);
            m_camera.SetRotationSpeed(0.0002f);
        }

        void OnShutdown() override
        {
        }

        void OnUpdate(Timestep timestep) override
        {
            m_camera.Update(timestep);
            m_scene->m_viewMatrix = m_camera.GetView();
            m_scene->m_projectionMatrix = m_camera.GetProjection();
        }

        void OnEvent(Event& e) override
        {
            m_camera.ProcessEvent(e);
        }

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
