#pragma once

#include <RHI/Swapchain.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

    class ISwapchain final : public Swapchain
    {
    public:
        ISwapchain(IContext* context);
        ~ISwapchain();

        VkResult Init(const SwapchainCreateInfo& createInfo);

        ResultCode Recreate(ImageSize2D newSize) override;
        ResultCode Present() override;

        VkSemaphore GetImageReadySemaphore() const;
        VkSemaphore GetFrameReadySemaphore() const;

    private:
        VkResult InitSurface(const SwapchainCreateInfo& createInfo);
        VkResult InitSwapchain();

    private:
        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR m_surface;
        VkSemaphore m_imageAcquiredSemaphore[MaxImageCount];
        VkSemaphore m_presentWaitSemaphore[MaxImageCount];

        VkResult m_lastPresentResult;
        VkCompositeAlphaFlagBitsKHR m_compositeAlpha;
        VkSurfaceFormatKHR m_surfaceFormat;
    };
} // namespace RHI::Vulkan