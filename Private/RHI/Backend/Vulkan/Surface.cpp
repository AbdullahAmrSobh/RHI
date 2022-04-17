#define VK_USE_PLATFORM_WIN32_KHR
#include "RHI/Backend/Vulkan/Surface.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

namespace RHI
{
namespace Vulkan
{

    Surface::~Surface() { vkDestroySurfaceKHR(m_instanceHandle, m_handle, nullptr); }
    
    VkResult Surface::Init(VkInstance instance, NativeWindowHandle nativeWindowHandle)
    {
		m_instanceHandle = instance;
        VkWin32SurfaceCreateInfoKHR createInfo           = {};
        createInfo.sType                                 = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.flags                                 = 0;
        createInfo.pNext                                 = 0;
        createInfo.hinstance                             = GetModuleHandle(nullptr);
        createInfo.hwnd                                  = static_cast<HWND>(nativeWindowHandle);
        static PFN_vkCreateWin32SurfaceKHR createSurface = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
        return createSurface(instance, &createInfo, nullptr, &m_handle);
    }

    std::vector<VkSurfaceFormatKHR> Surface::GetSupportedFormats()
    {
        uint32_t count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(count);
        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, formats.data());
        assert(result == VK_SUCCESS);
        return formats;
    }

    std::vector<VkPresentModeKHR> Surface::GetSupportedPresentModes()
    {
        uint32_t count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, nullptr);
        std::vector<VkPresentModeKHR> modes(count);
        VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, modes.data());
        assert(result == VK_SUCCESS);
        return modes;
    }

    VkSurfaceCapabilitiesKHR Surface::GetSupportedCapabilities()
    {
        VkSurfaceCapabilitiesKHR capabilities;
        VkResult                 result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &capabilities);
        assert(result == VK_SUCCESS);
        return capabilities;
    }

    VkSurfaceFormatKHR Surface::SelectFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        for (const auto& availableFormat : formats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }
        return formats[0];
    }

    VkExtent2D Surface::ClampExtent(VkExtent2D actualExtent, VkExtent2D currentExtent, VkExtent2D minImageExtent, VkExtent2D maxImageExtent)
    {
        if (actualExtent.width != UINT32_MAX)
            return currentExtent;
        else
            return {std::clamp(actualExtent.width, minImageExtent.width, maxImageExtent.width),
                    std::clamp(actualExtent.height, minImageExtent.height, maxImageExtent.height)};
    }

    VkPresentModeKHR Surface::SelectPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
    {
        return std::find_if(presentModes.begin(), presentModes.end(), [](VkPresentModeKHR mode) { return mode == VK_PRESENT_MODE_MAILBOX_KHR; }) ==
                       presentModes.end()
                   ? VK_PRESENT_MODE_MAILBOX_KHR
                   : VK_PRESENT_MODE_FIFO_KHR;
    }

} // namespace Vulkan
} // namespace RHI
