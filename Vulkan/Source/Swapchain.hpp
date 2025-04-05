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
        VkResult InitSwapchain(ImageSize2D size, uint32_t minImageCount);

    private:
        IDevice*       m_device;
        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR   m_surface;

        VkResult m_lastPresentResult;

        uint32_t    m_currentImageIndex;
        uint32_t    m_currentSemaphoreIndex;
        VkSemaphore m_imageAcquiredSemaphore[MaxImageCount];
        VkSemaphore m_imagePresentSemaphore[MaxImageCount];
        uint64_t    m_imageReleaseValue[MaxImageCount];
        VkImage     m_images[MaxImageCount];
        VkImageView m_imageViews[MaxImageCount];

        TL::String             m_name;
        SwapchainConfigureInfo m_configuration;
    };
} // namespace RHI::Vulkan