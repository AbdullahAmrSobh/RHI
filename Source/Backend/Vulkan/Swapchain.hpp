#pragma once
#include "RHI/Common.hpp"
#include "RHI/Swapchain.hpp"
#include "Backend/Vulkan/Device.hpp"
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
        Surface(const Instance& instance)
            : DeviceObject(nullptr)
            , m_pInstance(&instance)
        {
        }
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
        const Instance* m_pInstance;
    };

    class Swapchain final
        : public ISwapchain
        , public DeviceObject<VkSwapchainKHR>
    {
    public:
        Swapchain(const Device& device)
            : DeviceObject(&device)
            , m_imageAcquiredSemaphore(CreateUnique<Semaphore>(device))
        {
        }
        ~Swapchain();
        
        const std::vector<Image*>& GetBackImages() const;

        VkResult Init(const SwapchainDesc& desc);

        inline const Semaphore& GetBackbufferReadSemaphore() const
        {
            return *m_imageAcquiredSemaphore;
        }

        // Swap the back buffers
        virtual EResultCode SwapBuffers() override;
        virtual EResultCode Resize(Extent2D newExtent) override;
        virtual EResultCode SetFullscreenExeclusive(bool enable = true) override;

    private:
        VkResult InitBackbuffers();

        // Operations must wait on that semaphore,
        // before using the current swapchain Image.
        Unique<Semaphore> m_imageAcquiredSemaphore;
    };

} // namespace Vulkan
} // namespace RHI