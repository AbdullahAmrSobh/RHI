#pragma once
#include <Examples-Base/CommandLine.hpp>

namespace Engine
{
    template<typename AppType>
    inline int Entry(TL::Span<const char*> args)
    {
        AppType app{};

        app.m_launchSettings = CommandLine::Parse({ args.begin() + 1, args.end() });

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
} // namespace Engine