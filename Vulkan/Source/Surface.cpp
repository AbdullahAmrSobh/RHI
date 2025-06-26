#ifdef RHI_PLATFORM_WINDOWS
    #define VK_USE_PLATFORM_WIN32_KHR
#else
    #error "Current platfrom is not supported yet"
#endif

#include "Common.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "Queue.hpp"

// Platform specifc surface creation are contained witihn this file,
// to avoid polluting the global namespace with OS specific symbols

namespace RHI::Vulkan
{
    VkResult CreateSurface(IDevice& device, const SwapchainCreateInfo& createInfo, VkSurfaceKHR& outSurface)
    {
        VulkanResult result;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        VkWin32SurfaceCreateInfoKHR win32SurfaceCI =
            {
                .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                .pNext     = nullptr,
                .flags     = 0,
                .hinstance = static_cast<HINSTANCE>(createInfo.win32Window.hinstance),
                .hwnd      = static_cast<HWND>(createInfo.win32Window.hwnd),
            };
        result = vkCreateWin32SurfaceKHR(device.m_instance, &win32SurfaceCI, nullptr, &outSurface);
#endif

        if (!result)
        {
            TL_LOG_ERROR("Failed to create swapchain surface with error: {}", result.AsString());
        }

        IQueue*  queue = device.GetDeviceQueue(QueueType::Graphics);
        VkBool32 surfaceSupportPresent;
        result = vkGetPhysicalDeviceSurfaceSupportKHR(device.m_physicalDevice, queue->GetFamilyIndex(), outSurface, &surfaceSupportPresent);
        TL_ASSERT(result);

        if (surfaceSupportPresent != VK_TRUE)
        {
            TL_LOG_ERROR("surface does not support present: {}", result.AsString());
            vkDestroySurfaceKHR(device.m_instance, outSurface, nullptr);
            outSurface = VK_NULL_HANDLE;
        }

        return VK_SUCCESS;
    }
} // namespace RHI::Vulkan