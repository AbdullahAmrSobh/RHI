#include "Swapchain.hpp"
#include "Context.hpp"
#include "Common.hpp"
#include "Resources.hpp"

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    VkPresentModeKHR ConvertPresentMode(SwapchainPresentMode presentMode)
    {
        switch (presentMode)
        {
        case SwapchainPresentMode::Immediate:   return VK_PRESENT_MODE_IMMEDIATE_KHR;
        case SwapchainPresentMode::Fifo:        return VK_PRESENT_MODE_FIFO_KHR;
        case SwapchainPresentMode::FifoRelaxed: return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        case SwapchainPresentMode::Mailbox:     return VK_PRESENT_MODE_MAILBOX_KHR;
        default:                                return VK_PRESENT_MODE_MAX_ENUM_KHR;
        }
    }

    ISwapchain::ISwapchain(IContext* context)
        : m_context(context)
        , m_semaphores(VK_NULL_HANDLE, VK_NULL_HANDLE)
        , m_swapchain(VK_NULL_HANDLE)
        , m_surface(VK_NULL_HANDLE)
        , m_lastPresentResult(VK_ERROR_UNKNOWN)
    {
        ZoneScoped;
    }

    ISwapchain::~ISwapchain()
    {
        ZoneScoped;

        auto context = static_cast<IContext*>(m_context);
        vkDestroySwapchainKHR(context->m_device, m_swapchain, nullptr);
        vkDestroySurfaceKHR(context->m_instance, m_surface, nullptr);
        vkDestroySemaphore(context->m_device, m_semaphores.imageAcquired, nullptr);
        vkDestroySemaphore(context->m_device, m_semaphores.imageRenderComplete, nullptr);
    }

    VkResult ISwapchain::Init(const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        auto context = static_cast<IContext*>(m_context);

        m_createInfo = createInfo;

        m_semaphores.imageAcquired = context->CreateSemaphore();
        m_semaphores.imageRenderComplete = context->CreateSemaphore();

        InitSurface(createInfo);

        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context->m_physicalDevice, m_surface, &surfaceCapabilities);
        VULKAN_RETURN_VKERR_CODE(result);

        m_swapchainImagesCount = createInfo.imageCount;
        if (m_swapchainImagesCount < surfaceCapabilities.minImageCount || m_swapchainImagesCount > surfaceCapabilities.maxImageCount)
        {
            return VK_ERROR_UNKNOWN;
        }

        auto size = m_createInfo.imageSize;
        auto minSize = surfaceCapabilities.minImageExtent;
        auto maxSize = surfaceCapabilities.maxImageExtent;
        if ((size.width >= minSize.width && size.width <= maxSize.width &&
             size.height >= minSize.height && size.height <= maxSize.height) == false)
        {
            return VK_ERROR_UNKNOWN;
        }

        {
            uint32_t formatsCount;
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->m_physicalDevice, m_surface, &formatsCount, nullptr);
            VULKAN_ASSERT_SUCCESS(result);
            std::vector<VkSurfaceFormatKHR> formats{};
            formats.resize(formatsCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->m_physicalDevice, m_surface, &formatsCount, formats.data());

            m_surfaceFormat.format = VK_FORMAT_MAX_ENUM;
            for (auto surfaceFormat : formats)
            {
                if (surfaceFormat.format == ConvertFormat(createInfo.imageFormat))
                    m_surfaceFormat = surfaceFormat;
            }

            if (m_surfaceFormat.format == VK_FORMAT_MAX_ENUM)
            {
                return VK_ERROR_FORMAT_NOT_SUPPORTED;
            }
        }

        {
            VkCompositeAlphaFlagBitsKHR preferredCompositeAlpha[] = { VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR };
            m_compositeAlpha = VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;
            for (VkCompositeAlphaFlagBitsKHR compositeAlpha : preferredCompositeAlpha)
            {
                if (surfaceCapabilities.supportedCompositeAlpha & compositeAlpha)
                {
                    m_compositeAlpha = compositeAlpha;
                    break;
                }
            }

            if (m_compositeAlpha == VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR)
                return VK_ERROR_UNKNOWN;
        }

        {
            uint32_t presentModesCount;
            result = vkGetPhysicalDeviceSurfacePresentModesKHR(context->m_physicalDevice, m_surface, &presentModesCount, nullptr);
            VULKAN_ASSERT_SUCCESS(result);
            std::vector<VkPresentModeKHR> presentModes{};
            presentModes.resize(presentModesCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(context->m_physicalDevice, m_surface, &presentModesCount, presentModes.data());

            VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
            for (VkPresentModeKHR supportedMode : presentModes)
            {
                if (supportedMode == ConvertPresentMode(createInfo.presentMode))
                {
                    presentMode = supportedMode;
                }
            }

            if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
                return VK_ERROR_UNKNOWN;

            m_createInfo.presentMode = createInfo.presentMode;
        }

        InitSwapchain();

        return VK_SUCCESS;
    }

    ResultCode ISwapchain::Recreate(ImageSize2D newSize)
    {
        ZoneScoped;

        m_createInfo.imageSize = newSize;

        auto result = InitSwapchain();
        VULKAN_ASSERT_SUCCESS(result);
        return ConvertResult(result);
    }

    ResultCode ISwapchain::Present()
    {
        ZoneScoped;

        vkDeviceWaitIdle(m_context->m_device);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_semaphores.imageRenderComplete;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapchain;
        presentInfo.pImageIndices = &m_currentImageIndex;
        presentInfo.pResults = &m_lastPresentResult;
        auto result = vkQueuePresentKHR(m_context->m_presentQueue, &presentInfo);
        VULKAN_ASSERT_SUCCESS(result);

        result = vkAcquireNextImageKHR(m_context->m_device, m_swapchain, UINT64_MAX, m_semaphores.imageAcquired, VK_NULL_HANDLE, &m_currentImageIndex);
        VULKAN_ASSERT_SUCCESS(result);

        return ResultCode::Success;
    }

    VkResult ISwapchain::InitSwapchain()
    {
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.surface = m_surface;
        createInfo.minImageCount = m_swapchainImagesCount;
        createInfo.imageFormat = m_surfaceFormat.format;
        createInfo.imageColorSpace = m_surfaceFormat.colorSpace;
        createInfo.imageExtent.width = m_createInfo.imageSize.width;
        createInfo.imageExtent.height = m_createInfo.imageSize.height;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = ConvertImageUsageFlags(m_createInfo.imageUsage);
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        createInfo.compositeAlpha = m_compositeAlpha;
        createInfo.presentMode = ConvertPresentMode(m_createInfo.presentMode);
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = m_swapchain;

        auto result = vkCreateSwapchainKHR(m_context->m_device, &createInfo, nullptr, &m_swapchain);
        VULKAN_RETURN_VKERR_CODE(result);

        uint32_t imagesCount;
        result = vkGetSwapchainImagesKHR(m_context->m_device, m_swapchain, &imagesCount, nullptr);
        std::vector<VkImage> images;
        images.resize(imagesCount);
        result = vkGetSwapchainImagesKHR(m_context->m_device, m_swapchain, &imagesCount, images.data());
        VULKAN_RETURN_VKERR_CODE(result);

        for (uint32_t imageIndex = 0; imageIndex < m_swapchainImagesCount; imageIndex++)
        {
            IImage image{};
            image.pool = nullptr;
            image.handle = images[imageIndex];
            image.format = m_surfaceFormat.format;
            image.imageType = VK_IMAGE_TYPE_2D;
            image.swapchain = this;
            m_images[imageIndex] = m_context->m_imageOwner.Insert(image);
        }
        result = vkAcquireNextImageKHR(m_context->m_device, m_swapchain, UINT64_MAX, m_semaphores.imageAcquired, VK_NULL_HANDLE, &m_currentImageIndex);
        VULKAN_ASSERT_SUCCESS(result);

        return result;
    }
} // namespace RHI::Vulkan