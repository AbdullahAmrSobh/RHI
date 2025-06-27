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

    /// @brief struct contains win32 surface handles.
    struct Win32WindowDesc
    {
        void* hwnd;
        void* hinstance;
    };

    /// @brief Structure specifying the parameters of the swapchain.
    struct SwapchainCreateInfo
    {
        const char* name;

        union
        {
            Win32WindowDesc win32Window; // win32 surface handles. (Availabe only on windows)
        };
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
        ImageSize2D                     minImageSize;
        ImageSize2D                     maxImageSize;
        uint32_t                        minImageCount;
        uint32_t                        maxImageCount;
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

        /// @brief Returns the total number of images in the swapchain's back buffer.
        virtual uint32_t            GetImagesCount() const = 0;

        /// @brief Returns the handle to the currently acquired swapchain image.
        virtual Handle<Image>       GetImage() const = 0;

        /// @brief Queries the capabilities of the presentation surface.
        virtual SurfaceCapabilities GetSurfaceCapabilities() const = 0;

        /// @brief Resizes the swapchain to match new window dimensions.
        virtual ResultCode          Resize(const ImageSize2D& size) = 0;

        /// @brief Reconfigures the swapchain with new parameters.
        virtual ResultCode          Configure(const SwapchainConfigureInfo& configInfo) = 0;
    };
} // namespace RHI
