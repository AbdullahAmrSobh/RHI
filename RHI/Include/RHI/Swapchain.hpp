#pragma once
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Common/Result.hpp"

namespace RHI
{
    class Context;

    // @todo: add an API to query for supported formats for the user
    // @todo: add an API to query for supported present mode for the user

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
        const char*          name;
        ImageSize2D          imageSize;     // The size of the images in the swapchian.
        TL::Flags<ImageUsage>    imageUsage;    // Image usage flags applied to all created images.
        Format               imageFormat;   // The format of created swapchain image.
        uint32_t             minImageCount; // The numer of back buffer images in the swapchain.
        SwapchainPresentMode presentMode;
#ifdef RHI_PLATFORM_WINDOWS
        Win32WindowDesc win32Window; // win32 surface handles. (Availabe only on windows)
#endif
    };

    /// @brief Swapchain object which is an interface between the API and a presentation surface.
    class RHI_EXPORT Swapchain
    {
    public:
        static constexpr uint32_t MaxImageCount = 3;
        static constexpr uint32_t MinImageCount = 1;

        Swapchain(Context* context);
        virtual ~Swapchain() = default;

        /// @brief Get the current image index of the swapchain.
        uint32_t GetCurrentImageIndex() const;

        /// @brief Get the number of images in the swapchain.
        uint32_t GetImagesCount() const;

        /// @brief Get the current acquired swapchain image.
        Handle<Image> GetImage() const;

        /// @brief Get the indexed image in the swapchain images.
        Handle<Image> GetImage(uint32_t index) const;

        /// @brief Called to invalidate the current swapchain state, when the window is resized.
        virtual ResultCode Recreate(ImageSize2D newSize) = 0;

        /// @brief Presents the current image.
        virtual ResultCode Present() = 0;

    protected:
        Context*            m_context;
        TL::String          m_name;
        uint32_t            m_imageIndex;
        uint32_t            m_imageCount;
        SwapchainCreateInfo m_createInfo;
        Handle<Image>       m_images[MaxImageCount];
    };
} // namespace RHI
