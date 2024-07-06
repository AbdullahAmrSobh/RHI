#pragma once

#include <Examples-Base/ApplicationBase.hpp>
#include <Examples-Base/Timestep.hpp>
#include <Examples-Base/Common.hpp>
#include <Examples-Base/Window.hpp>
#include <Examples-Base/CommandLine.hpp>

#include <RHI/RHI.hpp>

namespace Examples
{
    class Camera;
    class ImGuiRenderer;

    class ApplicationBase
    {
    public:
        ApplicationBase(const char* name, uint32_t windowWidth, uint32_t windowHeight);
        virtual ~ApplicationBase();

        template<typename ExampleType>
        friend int Entry(TL::Span<const char*> args);

        CommandLine::LaunchSettings m_launchSettings;

    private:
        static ApplicationBase& Get();

        void Init();

        void Shutdown();

        void DispatchEvent(Event& event);

        void Run();

        virtual void OnInit() = 0;

        virtual void OnShutdown() = 0;

        virtual void OnUpdate(Timestep ts) = 0;

        virtual void OnEvent(Event& event) = 0;

    protected:
        Ptr<RHI::Context> m_context;

        Ptr<RHI::Swapchain> m_swapchain;

        Ptr<ImGuiRenderer> m_imguiRenderer;

        Ptr<Window> m_window;

        bool m_isRunning;
    };

    template<typename AppType>
    inline int Entry(TL::Span<const char*> args)
    {
        auto app = AppType();
        app.m_launchSettings = CommandLine::Parse({args.begin() + 1, args.end()});

        {
            ZoneScopedN("Init time");
            app.Init();
        }

        {
            app.Run();
        }

        {
            ZoneScopedN("Shutdown time");
            app.Shutdown();
        }

        return 0;
    }
} // namespace Examples
