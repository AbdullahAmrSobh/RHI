#pragma once
#include <cstdint>
#include <vector>
#include "RHI/Common.hpp"
#include "RHI/Format.hpp"

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

class IImage;

struct X11SurfaceDesc {};

class ISurface
{
public:
    virtual ~ISurface() = default;
};

struct SwapchainDesc
{
    ISurface* pSurface;
    Extent2D  extent;
    uint32_t  backImagesCount;
    EFormat   imageFormat;
};

class ISwapchain
{
public:
    virtual ~ISwapchain() = default;

    const std::vector<IImage*>& GetBackImages() const;

    // Swap the back buffers
    virtual EResultCode SwapBuffers()                               = 0;
    virtual EResultCode Resize(Extent2D newExtent)                  = 0;
    virtual EResultCode SetFullscreenExeclusive(bool enable = true) = 0;

protected:
    uint32_t             m_currentImageIndex;
    std::vector<IImage*> m_backBuffers;
    ISurface*            m_pSurface;
};

} // namespace RHI
