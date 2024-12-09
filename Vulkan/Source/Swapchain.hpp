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
        bool SelectSurfaceFormat(VkSurfaceFormatKHR& selectedFormat);

        VkCompositeAlphaFlagBitsKHR SelectCompositeAlpha(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

        VkPresentModeKHR SelectPresentMode();

        VkResult InitSurface(const SwapchainCreateInfo& createInfo);
        VkResult InitSwapchain();

    private:
        IDevice*       m_device;
        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR   m_surface;
        VkResult       m_lastPresentResult;
        uint32_t       m_semaphoreIndex;
        VkSemaphore    m_imageAcquiredSemaphore[MaxImageCount];
        VkSemaphore    m_imagePresentSemaphore[MaxImageCount];
        uint64_t       m_imageReleaseValue[MaxImageCount];
    };
} // namespace RHI::Vulkan