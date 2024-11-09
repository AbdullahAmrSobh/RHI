#pragma once

#include <RHI/Swapchain.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;

    class ISwapchain final : public Swapchain
    {
    public:
        ISwapchain(IDevice* device);
        ~ISwapchain();

        VkResult Init(const SwapchainCreateInfo& createInfo);

        VkSemaphore GetImageAcquiredSemaphore() const { return m_imageAcquiredSemaphore[m_semaphoreIndex]; }

        VkSemaphore GetImagePresentSemaphore() const { return m_imagePresentSemaphore[m_semaphoreIndex]; }

        void AcquireNextImage();

        VkSwapchainKHR GetHandle() const;

        ResultCode Recreate(ImageSize2D newSize) override;

        ResultCode Present() override;

    private:
        VkResult InitSurface(const SwapchainCreateInfo& createInfo);
        VkResult InitSwapchain();

    private:
        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR   m_surface;
        VkResult       m_lastPresentResult;

        uint32_t    m_semaphoreIndex;
        VkSemaphore m_imageAcquiredSemaphore[MaxImageCount];
        VkSemaphore m_imagePresentSemaphore[MaxImageCount];
    };
} // namespace RHI::Vulkan