#pragma once

#include <RHI/Swapchain.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;

    struct ImageSemaphorePair
    {
        VkSemaphore   semaphore;
        Handle<Image> image;
    };

    class ISwapchain final : public Swapchain
    {
    public:
        ISwapchain();
        ~ISwapchain();

        ResultCode Init(IDevice* device, const SwapchainCreateInfo& createInfo);
        void       Shutdown();

        VkSemaphore GetImageAcquiredSemaphore() const;

        VkSemaphore GetImagePresentSemaphore() const;

        ImageSemaphorePair AcquireNextImage();

        ResultCode Recreate(ImageSize2D newSize) override;

        ResultCode Present() override;

    private:
        VkResult InitSurface(const SwapchainCreateInfo& createInfo);
        VkResult InitSwapchain(ImageSize2D size, uint32_t minImageCount);

    private:
        /// @brief The device object.
        IDevice*            m_device;
        /// @brief The swapchain handle.
        VkSwapchainKHR      m_swapchain;
        /// @brief The surface of the swapchain.
        VkSurfaceKHR        m_surface;
        /// @brief The debug name of the swapchain.
        TL::String          m_name;
        /// @brief The result of the last image presentation.
        VkResult            m_lastPresentResult;
        /// @brief The current semaphore index.
        uint32_t            m_semaphoreIndex;
        /// @brief Swapchain image acquired semaphores.
        VkSemaphore         m_imageAcquiredSemaphore[MaxImageCount];
        /// @brief Swapchain image present semaphores.
        VkSemaphore         m_imagePresentSemaphore[MaxImageCount];
        /// @brief The release signal value for each in-flight image.
        uint64_t            m_imageReleaseValue[MaxImageCount];
        /// Swapchain recreation related members.
        /// @brief The description of the swapchain.
        SwapchainCreateInfo m_createInfo;
    };
} // namespace RHI::Vulkan