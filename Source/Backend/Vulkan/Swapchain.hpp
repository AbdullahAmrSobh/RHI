#pragma once
#include "RHI/Device.hpp"
#include "RHI/Swapchain.hpp"

#include <vulkan/vulkan.h>

#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

    class Surface final
        : public ISurface
        , public DeviceObject<VkSurfaceKHR>
    {
    public:
        ~Surface();
        
        VkResult Init(const Win32SurfaceDesc& desc);
        
        VkBool32 QueueSupportPresent(const class Queue& queue) const;

        std::vector<VkSurfaceFormatKHR> GetSupportedFormats();
        
        std::vector<VkPresentModeKHR> GetSupportedPresentModes();

        VkSurfaceCapabilities2KHR GetCapabilities2();
        
        static VkSurfaceFormatKHR SelectFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        
        static VkExtent2D ClampExtent(VkExtent2D actualExtent, VkExtent2D currentExtent, VkExtent2D minImageExtent, VkExtent2D maxImageExtent);

        static VkPresentModeKHR SelectPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
    
    private:
        Instance* m_pInstance;
    };
    
    class Swapchain final
        : public ISwapchain
        , public DeviceObject<VkSwapchainKHR>
    {
    public:
        ~Swapchain();
        
        const std::vector<Image*>& GetBackImages() const;
        
        VkResult Init(const SwapchainDesc& desc);

        // Swap the back buffers
        virtual EResultCode SwapBuffers() override;
        virtual EResultCode Resize(Extent2D newExtent) override;
        virtual EResultCode SetFullscreenExeclusive(bool enable = true) override;

    private:
        VkResult InitBackbuffers();
        
        // Operations must wait on that semaphore,
        // before using the current swapchain Image.
        Semaphore m_ImageAcquiredSemaphore;
    };

} // namespace Vulkan
} // namespace RHI