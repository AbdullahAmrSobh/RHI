#ifdef RHI_PLATFORM_WINDOWS
    #define VK_USE_PLATFORM_WIN32_KHR
#else
    #error "Current platfrom is not supported yet"
#endif

#include "Common.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"

// Platform specifc surface creation are contained witihn this file,
// to avoid polluting the global namespace with OS specific symbols

namespace RHI::Vulkan
{
    VkResult ISwapchain::InitSurface(const SwapchainCreateInfo& createInfo)
    {
#ifdef VK_USE_PLATFORM_WIN32_KHR
        VkWin32SurfaceCreateInfoKHR win32SurfaceCI{
            .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .pNext     = nullptr,
            .flags     = 0,
            .hinstance = static_cast<HINSTANCE>(createInfo.win32Window.hinstance),
            .hwnd      = static_cast<HWND>(createInfo.win32Window.hwnd),
        };
        Validate(vkCreateWin32SurfaceKHR(m_device->m_instance, &win32SurfaceCI, nullptr, &m_surface));
#endif

        VkBool32 surfaceSupportPresent;
        Validate(vkGetPhysicalDeviceSurfaceSupportKHR(
            m_device->m_physicalDevice,
            m_device->m_queue[(uint32_t)QueueType::Graphics]->GetFamilyIndex(),
            m_surface,
            &surfaceSupportPresent));
        return VK_SUCCESS;
    }
} // namespace RHI::Vulkan