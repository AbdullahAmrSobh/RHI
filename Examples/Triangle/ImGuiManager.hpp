#pragma once

#include <imgui.h>

namespace Examples
{
    class Event;
}

namespace Engine
{
    class IMGUI_IMPL_API ImGuiManager
    {
    public:
        ImGuiManager() = default;

        void ProcessEvent(Examples::Event& event);

        void Init();
        void Shutdown();

    private:
        ImGuiContext* m_imguiContext;
        struct Impl;
        Impl* m_impl;
    };
} // namespace Engine