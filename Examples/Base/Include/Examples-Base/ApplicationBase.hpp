#pragma once

#include "Examples-Base/Timestep.hpp"
#include "Examples-Base/Common.hpp"
#include "Examples-Base/CommandLine.hpp"

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
        friend int Entry(TL2::Span<const char*> args);

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
        Ptr<Window> m_window;

        Ptr<Renderer> m_renderer;

        Ptr<Scene> m_scene;
    };
} // namespace Examples
