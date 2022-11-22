#pragma once
#include "RHI/Common.hpp"

namespace RHI
{

struct Win32SurfaceDesc;
struct X11SurfaceDesc;

class ISurface
{
public:
    virtual ~ISurface() = default;
};

} // namespace RHI