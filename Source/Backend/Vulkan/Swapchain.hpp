#pragma once
#include "RHI/Swapchain.hpp"
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{
    class Instance;

    class Surface final
        : public ISurface
        , public DeviceObject<VkSurfaceKHR>
    {
    public:
        ~Surface();

#ifdef RHI_WINDOWS
        VkResult Init(const Win32SurfaceDesc& desc);
#elif defined(RHI_LINUX)
        VkResult Init(const X11SurfaceDesc& desc);
#endif

        VkBool32 QueueSupportPresent(const class Queue& queue) const;

        std::vector<VkSurfaceFormatKHR> GetSupportedFormats();

        std::vector<VkPresentModeKHR> GetSupportedPresentModes();

        VkSurfaceCapabilities2KHR GetCapabilities2();

        static VkSurfaceFormatKHR SelectFormat(const std::vector<VkSurfaceFormatKHR>& formats);

        static VkExtent2D ClampExtent(VkExtent2D actualExtent, VkExtent2D currentExtent, VkExtent2D minImageExtent, VkExtent2D maxImageExtent);

        static VkPresentModeKHR SelectPresentMode(const std::vector<VkPresentModeKHR>& presentModes);

    private:
        Instance* m_pInstance;
        Device*   m_pDevice;
    };

    class Swapchain final
        : public ISwapchain
        , public DeviceObject<VkSwapchainKHR>
    {
    public:
        ~Swapchain();

        const std::vector<Image*>& GetBackImages() const;

        VkResult Init(const SwapchainDesc& desc);

        inline Semaphore& GetBackbufferReadSemaphore()
        {
            return m_imageAcquiredSemaphore;
        }

        // Swap the back buffers
        virtual EResultCode SwapBuffers() override;
        virtual EResultCode Resize(Extent2D newExtent) override;
        virtual EResultCode SetFullscreenExeclusive(bool enable = true) override;

    private:
        VkResult InitBackbuffers();

        // Operations must wait on that semaphore,
        // before using the current swapchain Image.
        Semaphore m_imageAcquiredSemaphore;
    };

} // namespace Vulkan
} // namespace RHI