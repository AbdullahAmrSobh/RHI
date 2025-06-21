#pragma once

#include <imgui.h>

#include <RHI/RHI.hpp>

namespace Engine
{
    class Window;

    class ImGuiManager
    {
    public:
        ImGuiManager() = default;

        void Init(Window* primaryWindow);
        void Shutdown();

    private:
        Window*       m_primaryWindow;
        ImGuiContext* m_imguiContext;
    };
} // namespace Engine