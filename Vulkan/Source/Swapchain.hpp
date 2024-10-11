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

        ResultCode AcquireNextImage();

        ResultCode Recreate(ImageSize2D newSize) override;
        ResultCode Present() override;

    private:
        VkResult InitSurface(const SwapchainCreateInfo& createInfo);
        VkResult InitSwapchain();

    private:
        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR m_surface;

        uint32_t m_currentFrameInFlight;

        VkResult m_lastPresentResult;
    };
} // namespace RHI::Vulkan