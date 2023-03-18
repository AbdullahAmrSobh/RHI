#pragma once
#include "RHI/Swapchain.hpp"

#include "Backend/Vulkan/DeviceObject.hpp"
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{
class Instance;
class PhysicalDevice;
class Image;
class Semaphore;

class Surface final : public ISurface
{
public:
    Surface(const Instance& instance)
        : m_pInstance(&instance)
    {
    }
    ~Surface();

    VkSurfaceKHR GetHandle() const
    {
        return m_handle;
    }

    VkResult Init(const Win32SurfaceDesc& desc);

    VkBool32 QueryQueueFamilyPresentSupport(const PhysicalDevice& physicalDevice, uint32_t familyIndex) const;

    std::vector<VkSurfaceFormatKHR> GetSupportedFormats(const PhysicalDevice& physicalDevice) const;

    std::vector<VkPresentModeKHR> GetSupportedPresentModes(const PhysicalDevice& physicalDevice) const;

    VkSurfaceCapabilitiesKHR GetCapabilities(const PhysicalDevice& physicalDevice) const;

    static VkSurfaceFormatKHR SelectFormat(const std::vector<VkSurfaceFormatKHR>& formats);

    static VkExtent2D ClampExtent(VkExtent2D actualExtent, VkExtent2D currentExtent, VkExtent2D minImageExtent, VkExtent2D maxImageExtent);

    static VkPresentModeKHR SelectPresentMode(const std::vector<VkPresentModeKHR>& presentModes);

private:
    VkSurfaceKHR m_handle;

    const Instance* m_pInstance;
};

class Swapchain final
    : public ISwapchain
    , public DeviceObject<VkSwapchainKHR>
{
public:
    Swapchain(Device& device)
        : DeviceObject(device)
    {
    }

    ~Swapchain();

    VkResult Init(const SwapchainDesc& desc);

    Image& GetCurrentImage();

    Fence& GetCurrentFence()
    {
        return *m_framesInFlightFence[m_currentImageIndex];
    }

    const Semaphore& GetImageReadySemaphore() const
    {
        return *m_imageReadySemaphore;
    }

    Semaphore& GetImageReadySemaphore()
    {
        return *m_imageReadySemaphore;
    }

    void SwapImages() override;

private:
    std::unique_ptr<Semaphore> m_imageReadySemaphore;

    std::vector<std::unique_ptr<Fence>> m_framesInFlightFence;
};

}  // namespace Vulkan
}  // namespace RHI