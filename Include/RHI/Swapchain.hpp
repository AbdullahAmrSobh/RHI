#pragma once
#include <cstdint>
#include <vector>

#include "RHI/Common.hpp"
#include "RHI/Format.hpp"

// TODO move each platform into its own header file

#ifdef RHI_WINDOWS
#include <windows.h>
#endif

namespace RHI
{

class IImage;

using NativeWindowHandle = HWND;
struct Win32SurfaceDesc
{
    HWND      hwnd;
    HINSTANCE instance;
};

// using NativeWindowHandle = void*;
struct X11SurfaceDesc
{
};

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

    virtual EResultCode SwapBuffers()                               = 0;
    virtual EResultCode Resize(Extent2D newExtent)                  = 0;
    virtual EResultCode SetFullscreenExeclusive(bool enable = true) = 0;

protected:
    ISurface*            m_pSurface          = nullptr;
    uint32_t             m_currentImageIndex = 0;
    std::vector<IImage*> m_backBuffers;
};

} // namespace RHI
