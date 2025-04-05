#pragma once
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Result.hpp"
#include "RHI/Common.hpp"

#include <TL/Flags.hpp>
#include <TL/Containers.hpp>

namespace RHI
{
    enum class SwapchainAlphaMode : uint32_t
    {
        None           = 0 << 0,
        PreMultiplied  = 1 << 0,
        PostMultiplied = 1 << 1,
    };

    enum class SwapchainPresentMode : uint32_t
    {
        None        = 0 << 0,
        Immediate   = 1 << 0,
        Fifo        = 1 << 1,
        FifoRelaxed = 1 << 2,
        Mailbox     = 1 << 3,
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
        const char* name;
#ifdef RHI_PLATFORM_WINDOWS
        Win32WindowDesc win32Window; // win32 surface handles. (Availabe only on windows)
#endif
    };

    struct SwapchainConfigureInfo
    {
        ImageSize2D          size;
        uint32_t             imageCount;
        ImageUsage           imageUsage;
        Format               format;
        SwapchainPresentMode presentMode;
        SwapchainAlphaMode   alphaMode;
    };

    /// @brief Structure containing the capabilities of a presentation surface
    struct SurfaceCapabilities
    {
        TL::Flags<ImageUsage>           usages;       ///< Supported image usage flags for the surface
        TL::Flags<SwapchainPresentMode> presentModes; ///< List of supported presentation modes
        TL::Flags<SwapchainAlphaMode>   alphaModes;   ///< List of supported alpha compositing modes
        TL::Vector<Format>              formats;      ///< List of supported image formats
    };

    /// @brief Swapchain object which is an interface between the API and a presentation surface.
    class RHI_EXPORT Swapchain
    {
    public:
        RHI_INTERFACE_BOILERPLATE(Swapchain);

        /// @brief Returns the total number of images in the swapchain's back buffer
        /// @return Number of swapchain images
        uint32_t                    GetImagesCount() const { return m_imageCount; }

        /// @brief Returns the handle to the currently acquired swapchain image
        /// @return Handle to the current swapchain image
        Handle<Image>               GetImage() const { return m_image; }

        /// @brief Queries the capabilities of the presentation surface
        /// @return Structure containing supported formats, modes, and capabilities
        virtual SurfaceCapabilities GetSurfaceCapabilities()                            = 0;

        /// @brief Resizes the swapchain to match new window dimensions
        /// @param size New dimensions for the swapchain images
        /// @return Result code indicating success or failure
        virtual ResultCode          Resize(ImageSize2D size)                            = 0;

        /// @brief Reconfigures the swapchain with new parameters
        /// @note  Must be called before using the swapchain
        /// @param configInfo ...
        /// @return Result code indicating success or failure
        virtual ResultCode          Configure(const SwapchainConfigureInfo& configInfo) = 0;

        /// @brief Presents the current swapchain image to the display
        /// @return Result code indicating success or failure of the presentation
        virtual ResultCode          Present()                                           = 0;

    protected:
        uint32_t      m_imageCount;
        Handle<Image> m_image;
    };
} // namespace RHI
