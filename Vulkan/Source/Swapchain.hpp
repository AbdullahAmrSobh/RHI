#pragma once

#include <RHI/Swapchain.hpp>

#include <TL/Containers/String.hpp>

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
        void       Shutdown(IDevice* device);

        // Interface
        uint32_t            GetImagesCount() const override;
        Image*              GetImage() const override;
        SurfaceCapabilities GetSurfaceCapabilities() const override;
        ResultCode          Resize(const ImageSize2D& size) override;
        ResultCode          Configure(const SwapchainConfigureInfo& configInfo) override;

        VkResult       AcquireNextImage(VkSemaphore& acquireImageSemaphore);
        uint32_t       GetImageIndex() const;
        VkSwapchainKHR GetHandle() const;

    private:
        IDevice*               m_device                          = nullptr;
        VkSwapchainKHR         m_swapchain                       = VK_NULL_HANDLE;
        VkSurfaceKHR           m_surface                         = VK_NULL_HANDLE;
        uint32_t               m_acquireSemaphoreIndex           = 0;
        VkSemaphore            m_acquireSemaphore[MaxImageCount] = {};
        uint32_t               m_imageIndex                      = {};
        VkImage                m_images[MaxImageCount]           = {};
        VkImageView            m_imageViews[MaxImageCount]       = {};
        TL::String             m_name                            = {};
        SwapchainConfigureInfo m_configuration                   = {};
        uint32_t               m_imageCount                      = 0;
        struct IImage*         m_imageHandle                     = nullptr;
    };
} // namespace RHI::Vulkan