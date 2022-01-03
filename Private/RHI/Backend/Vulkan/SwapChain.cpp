#include "RHI/Backend/Vulkan/SwapChain.hpp"
#include <algorithm>

namespace RHI
{
namespace Vulkan
{
    static VkSurfaceFormatKHR SelectSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        for (const auto& availableFormat : formats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }
        return formats[0];
    }
    
    static VkExtent2D ClampSurfaceExtent(VkExtent2D actualExtent, VkExtent2D currentExtent, VkExtent2D minImageExtent, VkExtent2D maxImageExtent)
    {
        if (actualExtent.width != UINT32_MAX)
            return currentExtent;
        else
            return {std::clamp(actualExtent.width, minImageExtent.width, maxImageExtent.width),
                    std::clamp(actualExtent.height, minImageExtent.height, maxImageExtent.height)};
    }

    static VkPresentModeKHR SelectSurfacePresentMode(const std::vector<VkPresentModeKHR>& presentModes)
    {
        return std::find_if(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) == presentModes.end() ? VK_PRESENT_MODE_MAILBOX_KHR
                                                                                                                         : VK_PRESENT_MODE_FIFO_KHR;
    }

    VkResult Surface::Init(VkInstance instance, NativeWindowHandle nativeWindowHandle)
    {
        VkWin32SurfaceCreateInfoKHR createInfo           = {};
        createInfo.sType                                 = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.flags                                 = 0;
        createInfo.pNext                                 = 0;
        createInfo.hinstance                             = GetModuleHandle(nullptr);
        createInfo.hwnd                                  = static_cast<HWND>(nativeWindowHandle);
        static PFN_vkCreateWin32SurfaceKHR createSurface = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
        return createSurface(instance, &createInfo, nullptr, &m_handle);
    }

    SwapChain::~SwapChain() { vkDestroySwapchainKHR(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult SwapChain::Init(const SwapChainDesc& desc)
    {
        VkResult result;
        Surface* surface;

        VkSurfaceCapabilitiesKHR        surfaceCapabilities = surface->GetSupportedCapabilities();
        std::vector<VkSurfaceFormatKHR> surfaceFormats      = surface->GetSupportedFormats();
        std::vector<VkPresentModeKHR>   presentModes        = surface->GetSupportedPresentModes();

        VkSurfaceFormatKHR selectedSurfaceFormat = SelectSurfaceFormat(surfaceFormats);

        // create swapchain
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.surface                  = surface->GetHandle();
        createInfo.minImageCount            = std::clamp(desc.bufferCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
        createInfo.imageFormat              = selectedSurfaceFormat.format;
        createInfo.imageColorSpace          = selectedSurfaceFormat.colorSpace;
        createInfo.imageExtent              = ClampSurfaceExtent({desc.extent.sizeX, desc.extent.sizeY}, surfaceCapabilities.currentExtent,
                                                                 surfaceCapabilities.minImageExtent, surfaceCapabilities.maxImageExtent);
        createInfo.imageArrayLayers         = 1;
        createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
        createInfo.preTransform             = surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode              = SelectSurfacePresentMode(presentModes);
        createInfo.clipped                  = VK_TRUE;
        createInfo.oldSwapchain             = VK_NULL_HANDLE;

        result = vkCreateSwapchainKHR(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
        if (result != VK_SUCCESS)
            return result;

        // Obtain the swapchain images
        uint32_t imageCount = 0;
        result              = vkGetSwapchainImagesKHR(m_pDevice->GetHandle(), m_handle, &imageCount, nullptr);
        std::vector<VkImage> images(imageCount);
        result = vkGetSwapchainImagesKHR(m_pDevice->GetHandle(), m_handle, &imageCount, images.data());
        if (result != VK_SUCCESS)
            return result;

        for (uint32_t i = 0; i < imageCount; i++)
        {
			// Create render target view attachment;
        }
        
        return result;
    }

    EResultCode SwapChain::SwapBuffers() { return EResultCode::Success; }

} // namespace Vulkan
} // namespace RHI
