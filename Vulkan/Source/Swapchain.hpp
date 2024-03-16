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

        inline VkSemaphore GetImageReadySemaphore() { return m_imageAcquiredSemaphore; }

        inline VkSemaphore GetFrameReadySemaphore() { return m_frameReadySemaphore; }

    private:
        VkResult InitSurface(const SwapchainCreateInfo& createInfo);
        VkResult InitSwapchain();

    private:
        IContext* m_context;

        VkSemaphore m_imageAcquiredSemaphore;
        VkSemaphore m_frameReadySemaphore;

        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR m_surface;

        VkResult m_lastPresentResult;
        VkCompositeAlphaFlagBitsKHR m_compositeAlpha;
        VkSurfaceFormatKHR m_surfaceFormat;
    };
} // namespace RHI::Vulkan