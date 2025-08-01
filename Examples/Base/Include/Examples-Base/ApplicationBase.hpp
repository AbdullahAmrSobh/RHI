#pragma once

#include "Examples-Base/Timestep.hpp"
#include "Examples-Base/Window.hpp"
#include "Examples-Base/CommandLine.hpp"

#include <TL/UniquePtr.hpp>
#include <TL/Span.hpp>

namespace Engine
{
    class Window;

    class ApplicationBase
    {
        template<typename ExampleType>
        friend int Entry(TL::Span<const char*> args);

    public:
        ApplicationBase(const char* name, uint32_t windowWidth, uint32_t windowHeight);
        virtual ~ApplicationBase();

        const CommandLine::LaunchSettings GetLaunchSettings() const
        {
            return m_launchSettings;
        }

    private:
        void Init();

        void Shutdown();

        void Run();

    protected:
        void NotifyShouldClose();

        virtual void OnInit() = 0;

        virtual void OnShutdown() = 0;

        virtual void OnUpdate(Timestep ts) = 0;

        virtual void Render() = 0;

    protected:
        Window* m_window;

    private:
        CommandLine::LaunchSettings m_launchSettings;
    };
} // namespace Engine
