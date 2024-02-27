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

    private:
        VkResult InitSurface(const SwapchainCreateInfo& createInfo);
        VkResult InitSwapchain();

    public:
        friend class IFrameScheduler;

        IContext* m_context;

        struct Semaphores
        {
            VkSemaphore imageAcquired;
            VkSemaphore imageRenderComplete;
        } m_semaphores;

        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR m_surface;

        VkResult m_lastPresentResult;
        VkCompositeAlphaFlagBitsKHR m_compositeAlpha;
        VkSurfaceFormatKHR m_surfaceFormat;
    };
} // namespace RHI::Vulkan