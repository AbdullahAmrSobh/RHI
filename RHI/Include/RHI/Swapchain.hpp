#pragma once
#include "RHI/Handle.hpp"
#include "RHI/Export.hpp"
#include "RHI/Flags.hpp"
#include "RHI/Format.hpp"
#include "RHI/Object.hpp"

namespace RHI
{

struct Win32WindowDesc
{
    // NOTE hwnd, and hinstance are defined as void* to avoid including windows.h
    // headers which pollute the files with macro definitions
    void* hwnd;
    void* hinstance;
};

struct SwapchainCreateInfo
{
    ImageSize         imageSize;
    Flags<ImageUsage> imageUsage;
    Format            imageFormat;
    uint32_t          imageCount;
    Win32WindowDesc   win32Window;
};

/// @brief Swapchain
class RHI_EXPORT Swapchain : public Object
{
public:
    using Object::Object;
    virtual ~Swapchain() = default;

    // Set the VSync interval between Present calls.
    virtual void SetVSync(uint32_t vsync) = 0;

    // Called to invalidate the current swapchain state, when the window is resized.
    virtual ResultCode Resize(uint32_t newWidth, uint32_t newHeight) = 0;

    // Sets the Fullscreen mode on or off
    virtual ResultCode SetExclusiveFullScreenMode(bool enable_fullscreen) = 0;

    // Get the current image index of the swapchain.
    virtual uint32_t GetCurrentImageIndex() const = 0;

    // Get the number of images in the swapchain.
    virtual uint32_t GetImageCount() const = 0;

    // Get the current acquired swapchain image.
    virtual Image& GetCurrentImage() = 0;

    // Presents the current image to the window, and acquires the next image in the swapchain.
    virtual ResultCode Present() = 0;
};

}  // namespace RHI