#pragma once
#include "Backend/Vulkan/Resource.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{
namespace Vulkan
{
class Instance;
class PhysicalDevice;

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

    VkBool32 QueryQueueFamilyPresentSupport(uint32_t familyIndex) const;

    std::vector<VkSurfaceFormatKHR> GetSupportedFormats(
        const PhysicalDevice& physicalDevice);

    std::vector<VkPresentModeKHR> GetSupportedPresentModes(
        const PhysicalDevice& physicalDevice);

    VkSurfaceCapabilitiesKHR GetCapabilities(
        const PhysicalDevice& physicalDevice);

    static VkSurfaceFormatKHR SelectFormat(
        const std::vector<VkSurfaceFormatKHR>& formats);

    static VkExtent2D ClampExtent(VkExtent2D actualExtent,
                                  VkExtent2D currentExtent,
                                  VkExtent2D minImageExtent,
                                  VkExtent2D maxImageExtent);

    static VkPresentModeKHR SelectPresentMode(
        const std::vector<VkPresentModeKHR>& presentModes);

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
    {
    }

    ~Swapchain();

    VkResult Init(const SwapchainDesc& desc);

    Image& GetCurrentImage()
    {
        return static_cast<Image&>(ISwapchain::GetCurrentImage());
    }

    const Semaphore& GetImageReadySemaphore() const
    {
        return *m_imageReadySemaphore;
    }

    void SwapImages() override;

private:
    Unique<Semaphore> m_imageReadySemaphore;
};

}  // namespace Vulkan
}  // namespace RHI