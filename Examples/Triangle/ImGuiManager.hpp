#pragma once

#include <imgui.h>

namespace Examples
{
    class Event;
}

namespace Engine
{
    class ImGuiManager
    {
    public:
        ImGuiManager() = default;

        void ProcessEvent(Examples::Event& event);

        void Init();
        void Shutdown();

    private:
        ImGuiContext* m_imguiContext;
    };
} // namespace Engine