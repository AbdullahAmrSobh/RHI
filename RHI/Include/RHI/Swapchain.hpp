#pragma once
#include "RHI/Export.hpp"
#include "RHI/Flags.hpp"
#include "RHI/Format.hpp"
#include "RHI/Handle.hpp"
#include "RHI/Object.hpp"

#ifdef RHI_PLATFORM_WINDOWS
    #include "RHI/Platform/Windows/Windows.h"
#endif

namespace RHI
{

#ifdef RHI_PLATFORM_WINDOWS
/// @brief struct contains win32 surface handles.
struct Win32WindowDesc
{
    HWND      hwnd;
    HINSTANCE hinstance;
};
#endif

/// @brief Swapchain creation info struct.
struct SwapchainCreateInfo
{
    /// @brief The size of the images in the swapchian.
    ImageSize imageSize;

    /// @brief Image usage flags applied to all created images.
    Flags<ImageUsage> imageUsage;

    /// @brief The format of created swapchain image.
    Format imageFormat;

    /// @brief The numer of back buffer images in the swapchain.
    uint32_t imageCount;

#ifdef RHI_PLATFORM_WIN32
    /// @brief win32 surface handles.
    Win32WindowDesc win32Window;
#endif
};

/// @brief Swapchain
class RHI_EXPORT Swapchain : public Object
{
public:
    using Object::Object;
    virtual ~Swapchain() = default;

    /// @brief Set the VSync interval between Present calls.
    virtual void SetVSync(uint32_t vsync) = 0;

    /// @brief Called to invalidate the current swapchain state, when the window is resized.
    virtual ResultCode Resize(uint32_t newWidth, uint32_t newHeight) = 0;

    /// @brief Sets the Fullscreen mode on or off
    virtual ResultCode SetExclusiveFullScreenMode(bool enable_fullscreen) = 0;

    /// @brief Get the current image index of the swapchain.
    virtual uint32_t GetCurrentImageIndex() const = 0;

    /// @brief Get the number of images in the swapchain.
    virtual uint32_t GetImageCount() const = 0;

    /// @brief Get the current acquired swapchain image.
    virtual Image& GetCurrentImage() = 0;

    /// @brief Presents the current image to the window, and acquires the next image in the swapchain.
    virtual ResultCode Present() = 0;
};

}  // namespace RHI