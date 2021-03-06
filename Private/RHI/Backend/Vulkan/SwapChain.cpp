#include "RHI/Backend/Vulkan/SwapChain.hpp"
#include <algorithm>

namespace RHI
{
namespace Vulkan
{

    SwapChain::~SwapChain() { vkDestroySwapchainKHR(m_pDevice->GetHandle(), m_handle, nullptr); }
    
    VkResult SwapChain::Init(const SwapChainDesc& desc)
    {
        Surface& surface = Surface::FindSurfaceOrCreate(desc.windowHandle);
        
        VkSurfaceCapabilitiesKHR        surfaceCapabilities = surface.GetSupportedCapabilities();
        std::vector<VkSurfaceFormatKHR> surfaceFormats      = surface.GetSupportedFormats();
        std::vector<VkPresentModeKHR>   presentModes        = surface.GetSupportedPresentModes();
        
        VkSurfaceFormatKHR selectedSurfaceFormat = Surface::SelectFormat(surfaceFormats);
        
        // create swapchain
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.surface                  = surface.GetHandle();
        createInfo.minImageCount            = std::clamp(desc.bufferCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
        createInfo.imageFormat              = selectedSurfaceFormat.format;
        createInfo.imageColorSpace          = select21edSurfaceFormat.colorSpace;
        createInfo.imageExtent              = Surface::ClampExtent({desc.extent.sizeX, desc.extent.sizeY}, surfaceCapabilities.currentExtent,
                                                                   surfaceCapabilities.minImageExtent, surfaceCapabilities.maxImageExtent);
        createInfo.imageArrayLayers         = 1;
        createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
        createInfo.preTransform             = surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode              = Surface::SelectPresentMode(presentModes);
        createInfo.clipped                  = VK_TRUE;
        createInfo.oldSwapchain             = VK_NULL_HANDLE;
        
        VkResult result = vkCreateSwapchainKHR(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
        if (result != VK_SUCCESS)
            return result;

        ObtainBackBuffers();
        return result;
    }

    EResultCode SwapChain::SwapBuffers()
    {
        VkFence     fence;        //
        VkSemaphore imageIsReady; // semaphore to signal when the image acquired. commands that operate on this image will wait on this semaphore
        VkResult    result = vkAcquireNextImageKHR(m_pDevice->GetHandle(), m_handle, UINT64_MAX, imageIsReady, fence, &m_currentImageIndex);
        return ToResultCode(result);
    }
	
    void SwapChain::ObtainBackBuffers()
    {
        VkResult result = vkGetSwapchainImagesKHR(m_pDevice->GetHandle(), m_handle, &m_imageCount, nullptr);
        assert(result == VK_SUCCESS);
        std::vector<VkImage> images(m_imageCount);
        result = vkGetSwapchainImagesKHR(m_pDevice->GetHandle(), m_handle, &m_imageCount, images.data());
        assert(result == VK_SUCCESS);
        for (uint32_t i = 0; i < m_imageCount; i++)
            m_images[i] = new Texture(*m_pDevice, images[i], this);
    }

} // namespace Vulkan
} // namespace RHI
