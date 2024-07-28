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

        inline VkSemaphore GetImageAcquiredSemaphore() const { return m_imageAcquiredSemaphores[m_currentFrameInFlight]; }

        inline VkSemaphore GetImageSignaledSemaphore() const { return m_imageReleasedSemaphores[m_currentFrameInFlight]; }

    private:
        VkResult InitSurface(const SwapchainCreateInfo& createInfo);
        VkResult InitSwapchain();

    private:
        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR m_surface;

        uint32_t m_currentFrameInFlight;

        VkResult m_lastPresentResult;

        VkSemaphore m_imageAcquiredSemaphores[MaxImageCount];
        VkSemaphore m_imageReleasedSemaphores[MaxImageCount];
    };
} // namespace RHI::Vulkan