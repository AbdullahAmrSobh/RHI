#include "Examples-Base/ApplicationBase.hpp"
#include "Examples-Base/Window.hpp"
#include "Examples-Base/Event.hpp"
#include "Examples-Base/Camera.hpp"
#include "Examples-Base/Renderer.hpp"

#include <tracy/Tracy.hpp>

#include <chrono>

static bool APP_SHOULD_CLOSE = false;

namespace Examples
{
    extern Renderer* CreateDeferredRenderer();

    ApplicationBase::ApplicationBase(const char* name, uint32_t windowWidth, uint32_t windowHeight)
        : m_window(nullptr)
    {
        Window::Init();

        auto windowEventDispatcher = [this](Event& event)
        {
            this->DispatchEvent(event);
        };

        m_window = RHI::CreatePtr<Window>(name, Window::Size{ windowWidth, windowHeight }, windowEventDispatcher);
        m_renderer = Ptr<Renderer>(CreateDeferredRenderer());
    }

    ApplicationBase::~ApplicationBase()
    {
        Window::Shutdown();
    }

    void ApplicationBase::Init()
    {
        ZoneScoped;

        ResultCode result;

        result = m_renderer->Init(*m_window);
        RHI_ASSERT(IsSucess(result));

        OnInit();
    }

    void ApplicationBase::Shutdown()
    {
        ZoneScoped;

        OnShutdown();

        // m_renderer->Shutdown();
    }

    void ApplicationBase::DispatchEvent(Event& event)
    {
        if (event.GetEventType() == EventType::WindowClose)
        {
            APP_SHOULD_CLOSE = true;
        }

        // m_imguiRenderer->ProcessEvent(event);
        if (event.Handled)
            return;

        // propagate to application level event handling.
        OnEvent(event);
        if (event.Handled)
            return;
    }

    void ApplicationBase::Run()
    {
        auto currentTime = std::chrono::high_resolution_clock::now().time_since_epoch();

        double accumulator = 0.0;
        double deltaTime = 0.01;
        while (!APP_SHOULD_CLOSE)
        {
            auto newTime = std::chrono::high_resolution_clock::now().time_since_epoch();
            double frameTime = std::chrono::duration<double>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = std::min(frameTime, 0.25);
            accumulator += frameTime;

            while (accumulator >= deltaTime)
            {
                accumulator -= deltaTime;
                m_window->OnUpdate();
                OnUpdate(Timestep(deltaTime));
            }

            // Render
            m_renderer->Render();
        }
    }

} // namespace Examples