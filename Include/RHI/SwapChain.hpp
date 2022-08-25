#pragma once
#include <cstdint

#ifdef _WIN32
#include <windows.h>
namespace RHI
{
using NativeWindowHandle = HWND;
}
#else
namespace RHI
{
using NativeWindowHandle = void*;
}
#endif

namespace RHI
{
    
class ISurface
{
public:
    virtual ~ISurface() = default;
};

struct SwapChainDesc
{
    SwapChainDesc() = default;
    
    ISurface* pSurface;
    Extent2D  extent;
    uint32_t  backImagesCount;
    EFormat   imageFormat;
};

class ISwapChain
{
public:
    virtual ~ISwapChain() = default;

    // TODO
};

} // namespace RHI
