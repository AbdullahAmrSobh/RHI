#include "Examples-Base/ApplicationBase.hpp"
#include "Examples-Base/Window.hpp"

#include <tracy/Tracy.hpp>

#include <chrono>

static bool APP_SHOULD_CLOSE = false;

namespace Engine
{
    ApplicationBase::ApplicationBase(const char* name, uint32_t windowWidth, uint32_t windowHeight)
        : m_window(nullptr)
    {
        WindowManager::Init();
        m_window = WindowManager::CreateWindow(name, WindowFlags::None, WindowSize{windowWidth, windowHeight});
    }

    ApplicationBase::~ApplicationBase()
    {
        WindowManager::Shutdown();
    }

    void ApplicationBase::Init()
    {
        ZoneScoped;

        OnInit();
    }

    void ApplicationBase::Shutdown()
    {
        ZoneScoped;

        OnShutdown();
    }

    void ApplicationBase::Run()
    {
        auto currentTime = std::chrono::high_resolution_clock::now().time_since_epoch();

        double accumulator = 0.0;
        double deltaTime   = 0.01;
        while (!APP_SHOULD_CLOSE)
        {
            auto   newTime   = std::chrono::high_resolution_clock::now().time_since_epoch();
            double frameTime = std::chrono::duration<double>(newTime - currentTime).count();
            currentTime      = newTime;

            frameTime = std::min(frameTime, 0.25);
            accumulator += frameTime;

            while (accumulator >= deltaTime)
            {
                accumulator -= deltaTime;
                m_window->Poll();
                if (m_window->ShouldWindowClose())
                {
                    NotifyShouldClose();
                }
                OnUpdate(Timestep(deltaTime));
            }

            Render();
        }
    }

    void ApplicationBase::NotifyShouldClose()
    {
        APP_SHOULD_CLOSE = true;
    }

} // namespace Engine