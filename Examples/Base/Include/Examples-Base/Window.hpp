#pragma once

#include <TL/Containers.hpp>
#include <TL/Result.hpp>
#include <TL/String.hpp>
#include <TL/Flags.hpp>
#include <TL/Event.hpp>

#include <cstdint>

struct GLFWwindow;
struct GLFWmonitor;

namespace Engine
{
    enum class KeyState
    {
        None,
        Release,
        Press,
        Repeat,
    };

    enum class KeyMod
    {
        None     = 0,
        Shift    = 0x0001,
        Ctrl     = 0x0002,
        Alt      = 0x0004,
        Super    = 0x0008,
        CapsLock = 0x0010,
        NumLock  = 0x0020,
    };

    enum class KeyCode // From glfw3.h
    {
        Space      = 32,
        Apostrophe = 39, /* ' */
        Comma      = 44, /* , */
        Minus      = 45, /* - */
        Period     = 46, /* . */
        Slash      = 47, /* / */

        D0 = 48, /* 0 */
        D1 = 49, /* 1 */
        D2 = 50, /* 2 */
        D3 = 51, /* 3 */
        D4 = 52, /* 4 */
        D5 = 53, /* 5 */
        D6 = 54, /* 6 */
        D7 = 55, /* 7 */
        D8 = 56, /* 8 */
        D9 = 57, /* 9 */

        Semicolon = 59, /* ; */
        Equal     = 61, /* = */

        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,

        LeftBracket  = 91, /* [ */
        Backslash    = 92, /* \ */
        RightBracket = 93, /* ] */
        GraveAccent  = 96, /* ` */

        World1 = 161, /* non-US #1 */
        World2 = 162, /* non-US #2 */

        /* Function keys */
        Escape      = 256,
        Enter       = 257,
        Tab         = 258,
        Backspace   = 259,
        Insert      = 260,
        Delete      = 261,
        Right       = 262,
        Left        = 263,
        Down        = 264,
        Up          = 265,
        PageUp      = 266,
        PageDown    = 267,
        Home        = 268,
        End         = 269,
        CapsLock    = 280,
        ScrollLock  = 281,
        NumLock     = 282,
        PrintScreen = 283,
        Pause       = 284,
        F1          = 290,
        F2          = 291,
        F3          = 292,
        F4          = 293,
        F5          = 294,
        F6          = 295,
        F7          = 296,
        F8          = 297,
        F9          = 298,
        F10         = 299,
        F11         = 300,
        F12         = 301,
        F13         = 302,
        F14         = 303,
        F15         = 304,
        F16         = 305,
        F17         = 306,
        F18         = 307,
        F19         = 308,
        F20         = 309,
        F21         = 310,
        F22         = 311,
        F23         = 312,
        F24         = 313,
        F25         = 314,

        /* Keypad */
        KP0        = 320,
        KP1        = 321,
        KP2        = 322,
        KP3        = 323,
        KP4        = 324,
        KP5        = 325,
        KP6        = 326,
        KP7        = 327,
        KP8        = 328,
        KP9        = 329,
        KPDecimal  = 330,
        KPDivide   = 331,
        KPMultiply = 332,
        KPSubtract = 333,
        KPAdd      = 334,
        KPEnter    = 335,
        KPEqual    = 336,

        LeftShift    = 340,
        LeftControl  = 341,
        LeftAlt      = 342,
        LeftSuper    = 343,
        RightShift   = 344,
        RightControl = 345,
        RightAlt     = 346,
        RightSuper   = 347,
        Menu         = 348
    };

    enum class MouseCode // From glfw3.h
    {
        Button0 = 0,
        Button1 = 1,
        Button2 = 2,
        Button3 = 3,
        Button4 = 4,
        Button5 = 5,
        Button6 = 6,
        Button7 = 7,

        ButtonLeft    = Button0,
        ButtonRight   = Button1,
        ButtonMiddle  = Button2,
        ButtonForward = Button3,
        ButtonBack    = Button4,
        ButtonCount   = Button7,
    };

    enum class WindowEventType
    {
        Resized,
        Closed,
        Focused,
        Unfocused,
        Moved,
        CursorMoved,
        MouseScrolled,
        MouseInput,
        KeyInput,
        KeyTyped,
    };

    enum class WindowFlags
    {
        None          = 0,
        NonResizable  = 1 << 0, // Window can be resized
        NoDecorations = 1 << 2, // Window has no decorations (title bar, borders, etc.)
        Hidden        = 1 << 3, // Window is hidden by default and ShowWindow must be called first
    };

    enum class WindowAttribute
    {
        None,
        Iconified,
        Maximized,
    };

    struct WindowSize
    {
        uint32_t width, height;
    };

    struct WindowPosition
    {
        float x, y;
    };

    struct KeyboardEvent
    {
        KeyCode           code;
        KeyState          state;
        TL::Flags<KeyMod> mods;
    };

    struct MouseEvent
    {
        MouseCode         code;
        KeyState          state;
        TL::Flags<KeyMod> mods;
    };
    class Window;
    class Monitor;

    struct WindowEvent
    {
        Window*         window;
        WindowEventType type;

        union
        {
            KeyboardEvent  keyInput;       // WindowEventType::KeyPressed, KeyReleased, KeyTyped
            MouseEvent     mouseInput;     // WindowEventType::MouseButtonPressed, MouseButtonReleased
            WindowSize     size;           // WindowEventType::Resized
            WindowPosition position;       // WindowEventType::Moved
            WindowPosition cursorPosition; // WindowEventType::MouseMoved
            WindowPosition scrolled;       // WindowEventType::MouseScrolled
            uint32_t       keyCode;        // WindowEventType::KeyTyped
        };
    };

#ifdef CreateWindow
    #warning "CreateWindow macro is defined, which may cause conflicts with Engine::WindowManager::CreateWindow."
#endif

    class WindowManager
    {
    public:
        static TL::Error               Init();
        static void                    Shutdown();
        static Window*                 CreateWindow(TL::StringView title, TL::Flags<WindowFlags> flags, WindowSize size);
        static void                    DestroyWindow(Window* window);
        static TL::Span<const Monitor> GetMonitors();
    };

    class Monitor
    {
        friend class WindowManager;
        friend class TL::IAllocator;

    public:
        Monitor()  = default;
        ~Monitor() = default;
        // Returns the native handle of this monitor
        void*          GetNativeHandle() const;
        // Returns the position of the monitor in virtual screen coordinates
        WindowPosition GetPosition() const;
        // Returns the physical size of the monitor in millimeters
        WindowSize     GetPhysicalSize() const;
        // Returns the content scale of the monitor (DPI scaling)
        WindowPosition GetContentScale() const;
        // Returns the current video mode of the monitor
        WindowSize     GetCurrentVideoMode() const;
        // Returns true if this monitor is the primary monitor
        bool           IsPrimary() const;

    private:
        GLFWmonitor* m_monitor;
    };

    /// @brief Window abstraction class that wraps GLFW functionality
    class Window
    {
    private:
        friend class WindowManager;
        friend class TL::IAllocator;

    public:
        Window(TL::StringView title, TL::Flags<WindowFlags> flags, WindowSize size);
        ~Window();

        using EventHandler = TL::EventQueue<WindowEvent>::Handler;
        using HandlerID    = TL::EventQueue<WindowEvent>::SubID;

        // Subscribe to window event queue
        HandlerID      Subscribe(EventHandler&& handler);
        void           Unsubscribe(HandlerID id);
        // Return the native OS window handle (e.g. HWND on windows)
        void*          GetNativeHandle() const;
        // Newly created windows are initially hidden so SetWindowPos/Size/Title can be called on them before showing the window
        void           Show() const;
        // title
        TL::StringView GetTitle() const;
        void           SetTitle(TL::StringView title);
        // size
        WindowSize     GetSize() const;
        void           SetSize(WindowSize size);
        // size limits
        void           SetSizeLimits(WindowSize minSize, WindowSize maxSize);
        // position
        WindowPosition GetPosition() const;
        void           SetPosition(WindowPosition position);
        // Mouse cursor
        WindowPosition GetCursorPosition() const;
        WindowPosition GetCursorDeltaPosition() const;
        void           SetCursorPosition(WindowPosition position);
        // Mouse Keyboard inputs
        bool           GetKeyState(KeyCode key, KeyState state) const;
        bool           GetMouseState(MouseCode button, KeyState state) const;
        // Opacity
        void           SetOpacity(float opacity);
        // Should close
        bool           ShouldWindowClose() const;
        void           SetWindowShouldClose(bool shouldClose);
        // Clipboard
        TL::StringView GetClipboardText() const;
        void           SetClipboardText(TL::StringView text);
        // Foucs
        bool           IsFocused() const;
        void           SetFocus();
        //
        bool           GetAttribute(WindowAttribute attribute) const;
        // Poll must be called to update all window state
        void           Poll();

    private:
        GLFWwindow*                 m_window;
        TL::EventQueue<WindowEvent> m_eventQueue;
        WindowPosition              m_previousCursorPosition;
    };
} // namespace Engine