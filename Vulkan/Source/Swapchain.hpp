#pragma once

#include <RHI/Swapchain.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;

    VkResult CreateSurface(IDevice& device, const SwapchainCreateInfo& createInfo, VkSurfaceKHR& outSurface);

    class ISwapchain final : public Swapchain
    {
    public:
        constexpr static auto MaxImageCount = 4;

        ISwapchain();
        ~ISwapchain();

        ResultCode Init(IDevice* device, const SwapchainCreateInfo& createInfo);
        void       Shutdown();

        VkSemaphore GetImageAcquiredSemaphore() const;

        SurfaceCapabilities GetSurfaceCapabilities() override;
        ResultCode          Resize(ImageSize2D size) override;
        ResultCode          Configure(const SwapchainConfigureInfo& configInfo) override;

        VkResult AcquireNextImage(VkSemaphore& acquiredSemaphore);

        void CleanupOldSwapchain(VkSwapchainKHR oldSwapchain, uint32_t oldImageCount);

        VkSwapchainKHR GetHandle () const { return m_swapchain; }
        uint32_t GetCurrentImageIndex () const { return m_imageIndex; }

    // private:
        IDevice*               m_device;
        VkSwapchainKHR         m_swapchain;
        VkSurfaceKHR           m_surface;
        uint32_t               m_acquireSemaphoreIndex;
        VkSemaphore            m_imageAcquiredSemaphore[MaxImageCount];
        uint32_t               m_imageIndex;
        VkImage                m_images[MaxImageCount];
        VkImageView            m_imageViews[MaxImageCount];
        TL::String             m_name;
        SwapchainConfigureInfo m_configuration;
    };
} // namespace RHI::Vulkan