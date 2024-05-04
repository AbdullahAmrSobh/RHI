#pragma once
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Common/Result.hpp"

namespace RHI
{
    class Context;

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
        const char*       name;
        ImageSize2D       imageSize;   // The size of the images in the swapchian.
        Flags<ImageUsage> imageUsage;  // Image usage flags applied to all created images.
        Format            imageFormat; // The format of created swapchain image.
        uint32_t          imageCount;  // The numer of back buffer images in the swapchain.
#ifdef RHI_PLATFORM_WINDOWS
        Win32WindowDesc win32Window; // win32 surface handles. (Availabe only on windows)
#endif
        SwapchainPresentMode presentMode;
    };

    /// @brief Swapchain object which is an interface between the API and a presentation surface.
    class RHI_EXPORT Swapchain
    {
    public:
        Swapchain(Context* context);
        virtual ~Swapchain() = default;

        /// @brief Get the current image index of the swapchain.
        uint32_t           GetCurrentImageIndex() const;

        /// @brief Get the number of images in the swapchain.
        uint32_t           GetImagesCount() const;

        /// @brief Get the current acquired swapchain image.
        Handle<Image>      GetImage() const;

        /// @brief Get the indexed image in the swapchain images.
        Handle<Image>      GetImage(uint32_t index) const;

        /// @brief Called to invalidate the current swapchain state, when the window is resized.
        virtual ResultCode Recreate(ImageSize2D newSize) = 0;

        /// @brief Presents the current image.
        virtual ResultCode Present()                     = 0;

        /// @brief Get the swapchain image view.
        Handle<ImageView>  GetImageView(const ImageViewCreateInfo& createInfo);

    protected:
        Context*            m_context;
        uint32_t            m_currentImageIndex;
        uint32_t            m_swapchainImagesCount;
        SwapchainCreateInfo m_createInfo;
        Handle<Image>       m_images[c_MaxSwapchainBackBuffersCount];

        // clang-format off
        TL::UnorderedMap<ImageViewCreateInfo, Handle<ImageView>> m_imageViewsLRU;
        // clang-format on

        TL::String m_name;
    };
} // namespace RHI

namespace RHI
{
    inline Swapchain::Swapchain(Context* context)
        : m_context(context)
        , m_currentImageIndex(0)
        , m_swapchainImagesCount(0)
        , m_images()
    {
    }

    inline uint32_t Swapchain::GetCurrentImageIndex() const
    {
        return m_currentImageIndex;
    }

    inline uint32_t Swapchain::GetImagesCount() const
    {
        return m_swapchainImagesCount;
    }

    inline Handle<Image> Swapchain::GetImage() const
    {
        return m_images[m_currentImageIndex];
    }

} // namespace RHI