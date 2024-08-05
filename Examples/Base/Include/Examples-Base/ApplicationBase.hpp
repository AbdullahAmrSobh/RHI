#pragma once

#include "Examples-Base/Timestep.hpp"
#include "Examples-Base/CommandLine.hpp"

#include <TL/UniquePtr.hpp>

namespace Examples
{
    class Window;
    class Event;
    class Renderer;
    class Scene;

    class ApplicationBase
    {
    public:
        ApplicationBase(const char* name, uint32_t windowWidth, uint32_t windowHeight);
        virtual ~ApplicationBase();

        template<typename ExampleType>
        friend int Entry(TL::Span<const char*> args);

        CommandLine::LaunchSettings m_launchSettings;

    private:
        void Init();

        void Shutdown();

        void DispatchEvent(Event& event);

        void Run();

        virtual void OnInit() = 0;

        virtual void OnShutdown() = 0;

        virtual void OnUpdate(Timestep ts) = 0;

        virtual void OnEvent(Event& event) = 0;

    protected:
        TL::Ptr<Window> m_window;

        TL::Ptr<Renderer> m_renderer;

        TL::Ptr<Scene> m_scene;
    };
} // namespace Examples
