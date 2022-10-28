#pragma once
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