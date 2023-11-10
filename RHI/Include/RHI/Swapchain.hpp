#pragma once

#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

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
        /// @brief The size of the images in the swapchian.
        ImageSize         imageSize;

        /// @brief Image usage flags applied to all created images.
        Flags<ImageUsage> imageUsage;

        /// @brief The format of created swapchain image.
        Format            imageFormat;

        /// @brief The numer of back buffer images in the swapchain.
        uint32_t          imageCount;

#ifdef RHI_PLATFORM_WINDOWS
        /// @brief win32 surface handles. (Availabe only on windows)
        Win32WindowDesc win32Window;
#endif
    };

    /// @brief Swapchain object which is an interface between the API and a presentation surface.
    class RHI_EXPORT Swapchain
    {
    public:
        Swapchain()          = default;
        virtual ~Swapchain() = default;

        /// @brief Get the current image index of the swapchain.
        inline uint32_t GetCurrentImageIndex() const
        {
            return m_currentImageIndex;
        }

        /// @brief Get the number of images in the swapchain.
        inline uint32_t GetImagesCount() const
        {
            return m_swapchainImagesCount;
        }

        /// @brief Get the current acquired swapchain image.
        inline Handle<Image> GetImage() const
        {
            return m_images[m_currentImageIndex];
        }

        /// @brief Called to invalidate the current swapchain state, when the window is resized.
        virtual ResultCode Resize(uint32_t newWidth, uint32_t newHeight) = 0;

        /// @brief Presents the current image to the window, and acquires the next image in the swapchain.
        virtual ResultCode Present()                                     = 0;

    protected:
        uint32_t                   m_currentImageIndex;

        uint32_t                   m_swapchainImagesCount;

        std::vector<Handle<Image>> m_images;
    };

} // namespace RHI