#pragma once
#include "RHI/Common.hpp"

#ifdef RHI_WINDOWs
#include <windows.h>
namespace RHI
{

using NativeWindowHandle = HWND;
struct Win32SurfaceDesc
{
    HWND      hwnd;
    HINSTANCE instance;
};

} // namespace RHI

#elif defined(RHI_LINUX)
#include <X11/X.h>
#include <X11/Xlib.h>

namespace RHI_LINUX
{

struct X11SurfaceDesc
{
    Display* pDisplay;
    Window   window;
};

} // namespace RHI_LINUX

#endif

namespace RHI
{

class ISurface
{
public:
    virtual ~ISurface() = default;
};

} // namespace RHI