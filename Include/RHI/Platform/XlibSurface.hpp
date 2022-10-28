#pragma once
#include <cstdint>

namespace RHI
{

using NativeWindowHandle = uint64_t;

struct X11SurfaceDesc
{
    void*    pDisplay;
    uint64_t window;
};

} // namespace RHI
