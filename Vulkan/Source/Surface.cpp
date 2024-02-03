#include <algorithm>

#ifdef RHI_PLATFORM_WINDOWS
    #define VK_USE_PLATFORM_WIN32_KHR
#else
    #error "Current platfrom is not supported yet"
#endif

#include "Resources.hpp"
#include "Context.hpp"
#include "Common.hpp"
#include "Format.inl"

// Platform specifc surface creation are contained witihn this file,
// to avoid polluting the global namespace with OS specific symbols

namespace Vulkan
{
    VkResult Swapchain::InitSurface(const RHI::SwapchainCreateInfo& createInfo)
    {
        auto context = static_cast<Context*>(m_context);

#ifdef VK_USE_PLATFORM_WIN32_KHR
        // create win32 surface
        VkWin32SurfaceCreateInfoKHR vkCreateInfo;
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        // vkCreateInfo.hinstance = static_cast<HINSTANCE>(createInfo.win32Window.hinstance);
        vkCreateInfo.hwnd = static_cast<HWND>(createInfo.win32Window.hwnd);
        auto result = vkCreateWin32SurfaceKHR(context->m_instance, &vkCreateInfo, nullptr, &m_surface);
        VULKAN_ASSERT_SUCCESS(result);
#endif

        VkBool32 surfaceSupportPresent;
        result = vkGetPhysicalDeviceSurfaceSupportKHR(context->m_physicalDevice, context->m_graphicsQueueFamilyIndex, m_surface, &surfaceSupportPresent);
        RHI_ASSERT(result == VK_SUCCESS && surfaceSupportPresent == VK_TRUE);
        return result;
    }
} // namespace Vulkan