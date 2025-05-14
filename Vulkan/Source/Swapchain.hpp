#pragma once

#include <RHI/Swapchain.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;

    struct AcquiredImageInfo
    {
        VkSemaphore semaphore;
        VkImage     image;
        VkImageView view;
    };

    class ISwapchain final : public Swapchain
    {
    public:
        constexpr static auto MaxImageCount = 4;

        ISwapchain();
        ~ISwapchain();

        ResultCode Init(IDevice* device, const SwapchainCreateInfo& createInfo);
        void       Shutdown();

        VkSemaphore GetImageAcquiredSemaphore() const;
        VkSemaphore GetImagePresentSemaphore() const;

        SurfaceCapabilities GetSurfaceCapabilities() override;
        ResultCode          Resize(ImageSize2D size) override;
        ResultCode          Configure(const SwapchainConfigureInfo& configInfo) override;
        ResultCode          Present() override;

    private:
        VkResult InitSurface(const SwapchainCreateInfo& createInfo);
        VkResult AcquireNextImage();

        void CleanupOldSwapchain(VkSwapchainKHR oldSwapchain, uint32_t oldImageCount);

        void WaitForFrameInFlightReady(uint32_t frameIndex);

    private:
        IDevice*       m_device;
        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR   m_surface;

        ImageSize2D m_size;

        VkResult m_lastPresentResult;

        uint32_t m_acquireSemaphoreIndex     = 0;
        uint32_t m_acquireSemaphoreNextIndex = 0;
        uint32_t m_presentSemaphoreIndex     = 0;
        uint32_t m_imageIndex                = 0;

        VkSemaphore m_imageAcquiredSemaphore[MaxImageCount];
        VkSemaphore m_imagePresentSemaphore[MaxImageCount];
        VkImage     m_images[MaxImageCount];
        VkImageView m_imageViews[MaxImageCount];
        uint64_t    m_timelineValue[MaxImageCount];

        TL::String             m_name;
        SwapchainConfigureInfo m_configuration;
    };
} // namespace RHI::Vulkan