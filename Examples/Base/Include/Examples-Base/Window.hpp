#pragma once

#include "Examples-Base/Common.hpp"

#include <cstdint>

struct GLFWwindow;

namespace Examples
{
    enum class KeyCode;
    enum class MouseCode;

    /// @brief Window abstraction class that wraps GLFW functionality
    class Window
    {
    public:
        // TODO: Remove this
        /// @brief Initializes GLFW. Must be called at the start of the application only once
        static void Init();

        /// @brief Shuts down GLFW. Must be called at the end of the application only once
        static void Shutdown();

        /// @brief Type definition for event handler function
        using EventHandler = std::function<void(class Event&)>;

        /// @brief Structure to represent window size
        struct Size
        {
            uint32_t width, height;
        };

        /// @brief Structure to represent cursor position
        struct Cursor
        {
            float x, y;
        };

        /// @brief Constructor for Window class
        /// @param name The name of the window
        /// @param size The initial size of the window
        /// @param eventHandler The event handler function
        Window(const char* name, Size size, const EventHandler& eventHandler);

        /// @brief Destructor for Window class
        ~Window();

        /// @brief Checks if a specific key is pressed
        /// @param key The key code to check
        /// @return True if the key is pressed, false otherwise
        bool IsKeyPressed(KeyCode key) const;

        /// @brief Checks if a specific mouse button is pressed
        /// @param button The mouse button code to check
        /// @return True if the mouse button is pressed, false otherwise
        bool IsMouseButtonPressed(MouseCode button) const;

        /// @brief Gets the current window size
        /// @return The current Size of the window
        Size GetWindowSize() const;

        /// @brief Gets the current cursor position
        /// @return The current Cursor position
        Cursor GetCursorPosition() const;

        /// @brief Gets the change in cursor position since the last frame
        /// @return The Cursor delta position
        Cursor GetCursrorDeltaPosition() const;

        /// @brief Gets the native handle of the window
        /// @return Pointer to the native window handle
        void* GetNativeHandle() const;

        /// @brief Gets the GLFW window pointer
        /// @return Pointer to the GLFWwindow
        GLFWwindow* GetGlfwWindow() const;

        /// @brief Sets the event callback function
        /// @param eventHandler The new event handler function
        void SetEventCallback(const EventHandler& eventHandler);

        /// @brief Updates the window state
        void OnUpdate();

    private:
        /// @brief The name of the window
        TL::String m_name;

        /// @brief The current size of the window
        Size m_currentSize;

        /// @brief The previous cursor position
        Cursor m_previousCursorPosition;

        /// @brief pointer to the current GLFW window
        GLFWwindow* m_window;

        /// @brief Event handler. calls
        EventHandler m_handler;
    };

} // namespace Examples