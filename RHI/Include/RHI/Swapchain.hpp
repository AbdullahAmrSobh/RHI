#pragma once
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Result.hpp"
#include "RHI/Common.hpp"

#include <TL/Flags.hpp>
#include <TL/Containers.hpp>

namespace RHI
{
    class Device;

    /// @todo: add an API to query for supported formats for the user
    /// @todo: add an API to query for supported present mode for the user

    enum class SwapchainAlphaMode
    {
        None,
        PreMultiplied,
        PostMultiplied,
    };

    enum class SwapchainPresentMode
    {
        Immediate,
        Fifo,
        FifoRelaxed,
        Mailbox,
    };

#ifdef RHI_PLATFORM_WINDOWS
    /// @brief struct contains win32 surface handles.
    struct Win32WindowDesc
    {
        void* hwnd;
        void* hinstance;
    };
#endif

    /// @brief Structure specifying the parameters of the swapchain.
    struct SwapchainCreateInfo
    {
        const char*           name;
        ImageSize2D           imageSize;     // The size of the images in the swapchian.
        TL::Flags<ImageUsage> imageUsage;    // Image usage flags applied to all created images.
        Format                imageFormat;   // The format of created swapchain image.
        uint32_t              minImageCount; // The numer of back buffer images in the swapchain.
        SwapchainAlphaMode    alphaMode;     // The alpha mode applied to created images.
        SwapchainPresentMode  presentMode;
#ifdef RHI_PLATFORM_WINDOWS
        Win32WindowDesc win32Window; // win32 surface handles. (Availabe only on windows)
#endif
    };

    struct SurfaceCapabilities
    {
        TL::Flags<ImageUsage>            usages;
        TL::Vector<Format>               formats;
        TL::Vector<SwapchainPresentMode> presentModes;
        TL::Vector<SwapchainAlphaMode>   alphaModes;
    };

    /// @brief Swapchain object which is an interface between the API and a presentation surface.
    class RHI_EXPORT Swapchain
    {
    public:
        RHI_INTERFACE_BOILERPLATE(Swapchain);

        static constexpr uint32_t   MaxImageCount = 4;
        static constexpr uint32_t   MinImageCount = 1;

        /// @brief Get the current image index of the swapchain.
        uint32_t                    GetCurrentImageIndex() const;

        /// @brief Get the number of images in the swapchain.
        uint32_t                    GetImagesCount() const;

        /// @brief Get the current acquired swapchain image.
        virtual Handle<Image>       GetImage() const;

        /// @brief Called to invalidate the current swapchain state, when the window is resized.
        virtual ResultCode          Recreate(ImageSize2D newSize) = 0;

        /// @brief Presents the current image.
        virtual ResultCode          Present()                     = 0;

        // virtual SurfaceCapabilities GetSurfaceCapabilities() = 0;

    protected:
        uint32_t      m_imageIndex;
        uint32_t      m_imageCount;
        Handle<Image> m_image[MaxImageCount];
    };
} // namespace RHI
