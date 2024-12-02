#include "Swapchain.hpp"

#include <TL/Log.hpp>

#include <tracy/Tracy.hpp>

#include "Common.hpp"
#include "Device.hpp"
#include "Image.hpp"

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

    ISwapchain::ISwapchain() = default;

    ISwapchain::~ISwapchain()
    {
        Shutdown();
    }

    VkSemaphore ISwapchain::GetImageAcquiredSemaphore() const
    {
        return m_imageAcquiredSemaphore[m_semaphoreIndex];
    }

    VkSemaphore ISwapchain::GetImagePresentSemaphore() const
    {
        return m_imagePresentSemaphore[m_semaphoreIndex];
    }

    ResultCode ISwapchain::Init(IDevice* device, const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        m_device = device;

        m_name        = createInfo.name ? createInfo.name : "";
        m_imageSize   = createInfo.imageSize;
        m_imageUsage  = createInfo.imageUsage;
        m_imageFormat = createInfo.imageFormat;
        m_presentMode = createInfo.presentMode;
        m_imageCount  = createInfo.minImageCount;

        VkResult result;
        for (uint32_t i = 0; i < MaxImageCount; i++)
        {
            VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = nullptr, .flags = 0};

            result = vkCreateSemaphore(m_device->m_device, &semaphoreCI, nullptr, &m_imageAcquiredSemaphore[i]);
            TL_ASSERT(result == VK_SUCCESS);

            result = vkCreateSemaphore(m_device->m_device, &semaphoreCI, nullptr, &m_imagePresentSemaphore[i]);
            TL_ASSERT(result == VK_SUCCESS);

            if (!m_name.empty())
            {
                m_device->SetDebugName(m_imageAcquiredSemaphore[i], std::format("Swapchain-imageAcquiredSemaphore[{}]", i).data());
                m_device->SetDebugName(m_imagePresentSemaphore[i], std::format("Swapchain-imagePresentSemaphore[{}]", i).data());
            }
        }

        InitSurface(createInfo);
        InitSwapchain();

        return ResultCode::Success;
    }

    void ISwapchain::Shutdown()
    {
        ZoneScoped;

        Validate(vkDeviceWaitIdle(m_device->m_device));

        for (uint32_t i = 0; i < MaxImageCount; i++)
        {
            if (m_image[i] != NullHandle)
            {
                auto image = m_device->m_imageOwner.Get(m_image[i]);
                if (image->viewHandle != VK_NULL_HANDLE)
                    vkDestroyImageView(m_device->m_device, image->viewHandle, nullptr);
                m_device->m_imageOwner.Release(m_image[i]);
            }
            if (m_imageAcquiredSemaphore[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(m_device->m_device, m_imageAcquiredSemaphore[i], nullptr);
            if (m_imagePresentSemaphore[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(m_device->m_device, m_imagePresentSemaphore[i], nullptr);
        }

        vkDestroySwapchainKHR(m_device->m_device, m_swapchain, nullptr);
        vkDestroySurfaceKHR(m_device->m_instance, m_surface, nullptr);
    }

    ImageSemaphorePair ISwapchain::AcquireNextImage()
    {
        ZoneScoped;

        auto semaphore = m_imageAcquiredSemaphore[m_semaphoreIndex];

        auto result = vkAcquireNextImageKHR(m_device->m_device, m_swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &m_imageIndex);
        TL_ASSERT(result == VK_SUCCESS);

        return {
            .semaphore = semaphore,
            .image     = m_image[m_imageIndex],
        };
    }

    ResultCode ISwapchain::Recreate(ImageSize2D newSize)
    {
        ZoneScoped;
        m_imageSize = newSize;
        Validate(vkDeviceWaitIdle(m_device->m_device));
        VkResult result = InitSwapchain();
        return ConvertResult(result);
    }

    ResultCode ISwapchain::Present()
    {
        ZoneScoped;

        auto presentQueue = m_device->m_queue[(uint32_t)QueueType::Graphics]->GetHandle();

        VkPresentInfoKHR presentInfo{
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext              = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &m_imagePresentSemaphore[m_semaphoreIndex],
            .swapchainCount     = 1,
            .pSwapchains        = &m_swapchain,
            .pImageIndices      = &m_imageIndex,
            .pResults           = &m_lastPresentResult,
        };
        Validate(vkQueuePresentKHR(presentQueue, &presentInfo));

        m_semaphoreIndex = (m_semaphoreIndex + 1) % m_imageCount;

        AcquireNextImage();

        return ResultCode::Success;
    }

    VkResult ISwapchain::InitSwapchain()
    {
        ZoneScoped;

        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        Validate(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_physicalDevice, m_surface, &surfaceCapabilities));

        if (!ValidateImageCount(surfaceCapabilities) || !ClampImageSize(surfaceCapabilities))
        {
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        VkSurfaceFormatKHR selectedFormat{};
        if (!SelectSurfaceFormat(selectedFormat))
        {
            TL_LOG_INFO("Failed to (re)create the swapchain with the required format.");
            return VK_ERROR_FORMAT_NOT_SUPPORTED;
        }

        VkCompositeAlphaFlagBitsKHR selectedCompositeAlpha = SelectCompositeAlpha(surfaceCapabilities);

        VkPresentModeKHR selectedPresentMode = SelectPresentMode();
        if (selectedPresentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
        {
            TL_LOG_WARNNING("Fallback to default present mode due to unsupported requested present mode.");
            selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR; // Fallback to FIFO
        }
        VkSwapchainCreateInfoKHR createInfo = {
            .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext                 = nullptr,
            .flags                 = 0,
            .surface               = m_surface,
            .minImageCount         = m_imageCount,
            .imageFormat           = selectedFormat.format,
            .imageColorSpace       = selectedFormat.colorSpace,
            .imageExtent           = {m_imageSize.width, m_imageSize.height},
            .imageArrayLayers      = 1,
            .imageUsage            = ConvertImageUsageFlags(m_imageUsage),
            .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = nullptr,
            .preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha        = selectedCompositeAlpha,
            .presentMode           = selectedPresentMode,
            .clipped               = VK_TRUE,
            .oldSwapchain          = m_swapchain,
        };
        Validate(vkCreateSwapchainKHR(m_device->m_device, &createInfo, nullptr, &m_swapchain));
        m_device->SetDebugName(m_swapchain, m_name.c_str());

        if (createInfo.oldSwapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(m_device->m_device, createInfo.oldSwapchain, nullptr);
        }

        {
            VkImage images[MaxImageCount];
            Validate(vkGetSwapchainImagesKHR(m_device->m_device, m_swapchain, &m_imageCount, nullptr));
            TL_ASSERT(m_imageCount <= MaxImageCount, "Swapchain returned more images than supported.");
            Validate(vkGetSwapchainImagesKHR(m_device->m_device, m_swapchain, &m_imageCount, images));
            for (uint32_t i = 0; i < m_imageCount; ++i)
            {
                IImage image{};
                Validate(image.Init(m_device, images[i], createInfo));
                m_image[i] = m_device->m_imageOwner.Emplace(std::move(image));
            }
        }
        AcquireNextImage();

        return VK_SUCCESS;
    }

    bool ISwapchain::ValidateImageCount(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
    {
        if (m_imageCount < MinImageCount || m_imageCount > MaxImageCount ||
            m_imageCount < surfaceCapabilities.minImageCount || m_imageCount > surfaceCapabilities.maxImageCount)
        {
            TL_LOG_INFO("Invalid SwapchainCreateInfo::minImageCount for the given window.");
            return false;
        }
        return true;
    }

    bool ISwapchain::ClampImageSize(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
    {
        if (m_imageSize.width < surfaceCapabilities.minImageExtent.width ||
            m_imageSize.height < surfaceCapabilities.minImageExtent.height ||
            m_imageSize.width > surfaceCapabilities.maxImageExtent.width ||
            m_imageSize.height > surfaceCapabilities.maxImageExtent.height)
        {
            TL_LOG_WARNNING("Swapchain size will be clamped to fit supported range.");
        }

        m_imageSize.width  = std::clamp(m_imageSize.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        m_imageSize.height = std::clamp(m_imageSize.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

        return true;
    }

    bool ISwapchain::SelectSurfaceFormat(VkSurfaceFormatKHR& selectedFormat)
    {
        uint32_t formatCount = 0;
        Validate(vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, nullptr));
        TL::Vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, formats.data());

        for (const auto& format : formats)
        {
            if (format.format == ConvertFormat(m_imageFormat))
            {
                selectedFormat = format;
                return true;
            }
        }

        return false;
    }

    VkCompositeAlphaFlagBitsKHR ISwapchain::SelectCompositeAlpha(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
    {
        static const VkCompositeAlphaFlagBitsKHR preferredCompositeAlpha[] = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        };

        for (VkCompositeAlphaFlagBitsKHR compositeAlpha : preferredCompositeAlpha)
        {
            if (surfaceCapabilities.supportedCompositeAlpha & compositeAlpha)
            {
                return compositeAlpha;
            }
        }

        return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Default fallback
    }

    VkPresentModeKHR ISwapchain::SelectPresentMode()
    {
        uint32_t presentModeCount = 0;
        Validate(vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->m_physicalDevice, m_surface, &presentModeCount, nullptr));
        TL::Vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->m_physicalDevice, m_surface, &presentModeCount, presentModes.data());

        for (VkPresentModeKHR mode : presentModes)
        {
            if (mode == ConvertPresentMode(m_presentMode))
            {
                return mode;
            }
        }

        return VK_PRESENT_MODE_MAX_ENUM_KHR;
    }
} // namespace RHI::Vulkan