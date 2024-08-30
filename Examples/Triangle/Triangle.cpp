#include <Examples-Base/ApplicationBase.hpp>

#include <Assets/Importer.hpp>

#include <RPI/Renderer.hpp>

#include <tracy/Tracy.hpp>

#include "Camera.hpp"

using namespace Examples;

class BasicRenderer final : public ApplicationBase
{
public:
    BasicRenderer()
        : ApplicationBase("Hello, Triangle", 1600, 1200)
        , m_renderer(RPI::Renderer::CreateDeferred())
    {
    }

    void OnInit() override
    {
        // auto assetFile = Assets::Import(m_launchSettings.sceneFileLocation.c_str());

        m_renderer->Init(*m_window);
    }

    void OnShutdown() override
    {
        m_renderer->Shutdown();
    }

    void OnUpdate(Timestep timestep) override
    {
    }

    void Render() override
    {
        m_renderer->Render();
    }

    void OnEvent(Event& e) override
    {
    }

    TL::Ptr<RPI::Renderer> m_renderer;

    RHI::Handle<RHI::Pass> m_pass;
};

#include <Examples-Base/Entry.hpp>

int main(int argc, const char* argv[])
{
    using namespace Examples;
    TL::Span args{ argv, (size_t)argc };
    return Entry<BasicRenderer>(args);
}
