#include "Swapchain.hpp"
#include "Context.hpp"
#include "Common.hpp"
#include "Image.hpp"

#include <TL/Log.hpp>

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
        : Swapchain(context)
        , m_swapchain(VK_NULL_HANDLE)
        , m_surface(VK_NULL_HANDLE)
        , m_lastPresentResult()
    {
        ZoneScoped;
    }

    ISwapchain::~ISwapchain()
    {
        ZoneScoped;

        auto context = (IContext*)m_context;

        vkDeviceWaitIdle(context->m_device);

        for (auto semaphore : m_imageAcquiredSemaphores)
        {
            if (semaphore) context->DestroySemaphore(semaphore);
        }

        for (auto semaphore : m_imageReleasedSemaphores)
        {
            if (semaphore) context->DestroySemaphore(semaphore);
        }

        for (auto imageHandle : m_images)
        {
            if (imageHandle)
            {
                context->m_imageOwner.Release(imageHandle);
            }
        }

        vkDestroySwapchainKHR(context->m_device, m_swapchain, nullptr);
        vkDestroySurfaceKHR(context->m_instance, m_surface, nullptr);
    }

    VkResult ISwapchain::Init(const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        m_createInfo = createInfo;
        m_name = createInfo.name ? createInfo.name : "";
        m_createInfo.name = m_name.c_str();

        auto context = (IContext*)m_context;
        for (uint32_t i = 0; i < MaxImageCount; i++)
        {
            m_imageAcquiredSemaphores[i] = context->CreateSemaphore();
            m_imageReleasedSemaphores[i] = context->CreateSemaphore();
        }

        InitSurface(createInfo);
        InitSwapchain();

        return VK_SUCCESS;
    }

    ResultCode ISwapchain::Recreate(ImageSize2D newSize)
    {
        ZoneScoped;

        m_createInfo.imageSize = newSize;

        auto result = InitSwapchain();
        return ConvertResult(result);
    }

    ResultCode ISwapchain::Present()
    {
        ZoneScoped;

        auto context = (IContext*)m_context;
        auto waitSemaphore = GetImageSignaledSemaphore();

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &waitSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapchain;
        presentInfo.pImageIndices = &m_imageIndex;
        presentInfo.pResults = &m_lastPresentResult;
        Validate(vkQueuePresentKHR(context->m_queue[(uint32_t)QueueType::Graphics].GetHandle(), &presentInfo));

        m_currentFrameInFlight = (m_currentFrameInFlight + 1) % m_imageCount;
        Validate(vkAcquireNextImageKHR(context->m_device, m_swapchain, UINT64_MAX, GetImageAcquiredSemaphore(), VK_NULL_HANDLE, &m_imageIndex));
        return ResultCode::Success;
    }

    VkResult ISwapchain::InitSwapchain()
    {
        auto context = (IContext*)m_context;

        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        Validate(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->m_physicalDevice, m_surface, &surfaceCapabilities));

        if (m_createInfo.minImageCount < MinImageCount || m_createInfo.minImageCount > MaxImageCount)
        {
            TL_LOG_INFO("Failed to create the swapchain, invalid SwapchainCreateInfo::minImageCount.");
            return VK_ERROR_UNKNOWN;
        }
        else if (m_createInfo.minImageCount < surfaceCapabilities.minImageCount || m_createInfo.minImageCount > surfaceCapabilities.maxImageCount)
        {
            TL_LOG_INFO("Failed to create the swapchain, invalid SwapchainCreateInfo::minImageCount for the given window");
            return VK_ERROR_UNKNOWN;
        }
        else if (m_createInfo.imageSize.width < surfaceCapabilities.minImageExtent.width ||
                 m_createInfo.imageSize.height < surfaceCapabilities.minImageExtent.height ||
                 m_createInfo.imageSize.width > surfaceCapabilities.maxImageExtent.width ||
                 m_createInfo.imageSize.height > surfaceCapabilities.maxImageExtent.height)
        {
            TL_LOG_WARNNING("Swapchain requested size will be clamped to fit into window's supported size range");
        }

        m_createInfo.imageSize.width = std::clamp(m_createInfo.imageSize.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        m_createInfo.imageSize.height = std::clamp(m_createInfo.imageSize.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

        uint32_t formatsCount;
        Validate(vkGetPhysicalDeviceSurfaceFormatsKHR(context->m_physicalDevice, m_surface, &formatsCount, nullptr));
        TL::Vector<VkSurfaceFormatKHR> formats{};
        formats.resize(formatsCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(context->m_physicalDevice, m_surface, &formatsCount, formats.data());

        bool formatFound = false;
        VkSurfaceFormatKHR selectedFormat = {};
        for (auto surfaceFormat : formats)
        {
            if (surfaceFormat.format == ConvertFormat(m_createInfo.imageFormat))
            {
                selectedFormat = surfaceFormat;
                formatFound = true;
                break;
            }
        }

        if (formatFound == false)
        {
            TL_LOG_INFO("Failed to (re)create the swapchain with the required format");
            return VK_ERROR_FORMAT_NOT_SUPPORTED;
        }

        // @todo: Revist this
        VkCompositeAlphaFlagBitsKHR preferredCompositeAlpha[] = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
        };

        VkCompositeAlphaFlagBitsKHR selectedCompositeAlpha = VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;
        for (VkCompositeAlphaFlagBitsKHR compositeAlpha : preferredCompositeAlpha)
        {
            if (surfaceCapabilities.supportedCompositeAlpha & compositeAlpha)
            {
                selectedCompositeAlpha = compositeAlpha;
                break;
            }
        }

        uint32_t presentModesCount;
        Validate(vkGetPhysicalDeviceSurfacePresentModesKHR(context->m_physicalDevice, m_surface, &presentModesCount, nullptr));
        TL::Vector<VkPresentModeKHR> presentModes{};
        presentModes.resize(presentModesCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(context->m_physicalDevice, m_surface, &presentModesCount, presentModes.data());

        VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
        for (VkPresentModeKHR supportedMode : presentModes)
        {
            if (supportedMode == ConvertPresentMode(m_createInfo.presentMode))
            {
                presentMode = supportedMode;
            }
        }

        if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
        {
            // @todo: revist this message
            TL_LOG_WARNNING("Failed to create swapchain with the requested present mode. Will use a fallback present mode");
            presentMode = presentModes.front();
        }

        auto oldSwapchain = m_swapchain;
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.surface = m_surface;
        createInfo.minImageCount = m_createInfo.minImageCount;
        createInfo.imageFormat = selectedFormat.format;
        createInfo.imageColorSpace = selectedFormat.colorSpace;
        createInfo.imageExtent.width = m_createInfo.imageSize.width;
        createInfo.imageExtent.height = m_createInfo.imageSize.height;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = ConvertImageUsageFlags(m_createInfo.imageUsage);
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        createInfo.compositeAlpha = selectedCompositeAlpha;
        createInfo.presentMode = ConvertPresentMode(m_createInfo.presentMode);
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = m_swapchain;
        Validate(vkCreateSwapchainKHR(context->m_device, &createInfo, nullptr, &m_swapchain));
        context->SetDebugName(m_swapchain, m_name.c_str());

        if (oldSwapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(context->m_device, oldSwapchain, nullptr);
        }

        Validate(vkGetSwapchainImagesKHR(context->m_device, m_swapchain, &m_imageCount, nullptr));
        TL::Vector<VkImage> images;
        images.resize(m_imageCount);
        Validate(vkGetSwapchainImagesKHR(context->m_device, m_swapchain, &m_imageCount, images.data()));


        m_currentFrameInFlight = 0;
        Validate(vkAcquireNextImageKHR(context->m_device, m_swapchain, UINT64_MAX, GetImageAcquiredSemaphore(), VK_NULL_HANDLE, &m_imageIndex));

        for (uint32_t imageIndex = 0; imageIndex < m_imageCount; imageIndex++)
        {
            IImage image{};
            Validate(image.Init(context, images[imageIndex], createInfo));
            m_images[imageIndex] = context->m_imageOwner.Emplace(std::move(image));
        }
        return VK_SUCCESS;
    }
} // namespace RHI::Vulkan