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

    inline static bool SelectSurfaceFormat(IDevice& device, VkSurfaceKHR surface, Format format, VkSurfaceFormatKHR& selectedFormat)
    {
        uint32_t formatCount = 0;
        Validate(vkGetPhysicalDeviceSurfaceFormatsKHR(device.m_physicalDevice, surface, &formatCount, nullptr));
        TL::Vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device.m_physicalDevice, surface, &formatCount, formats.data());
        for (const auto& surfaceFormat : formats)
        {
            if (surfaceFormat.format == ConvertFormat(format))
            {
                selectedFormat = surfaceFormat;
                return true;
            }
        }
        return false;
    }

    inline static bool SelectPresentMode(IDevice& device, SwapchainPresentMode presentMode, VkSurfaceKHR surface, VkPresentModeKHR& selectedPresentMode)
    {
        uint32_t presentModeCount = 0;
        Validate(vkGetPhysicalDeviceSurfacePresentModesKHR(device.m_physicalDevice, surface, &presentModeCount, nullptr));
        TL::Vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device.m_physicalDevice, surface, &presentModeCount, presentModes.data());
        for (VkPresentModeKHR mode : presentModes)
        {
            if (mode == ConvertPresentMode(presentMode))
            {
                selectedPresentMode = mode;
                return true;
            }
        }
        return false;
    }

    inline static VkCompositeAlphaFlagBitsKHR SelectCompositeAlpha(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
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

    ISwapchain::ISwapchain()  = default;
    ISwapchain::~ISwapchain() = default;

    ResultCode ISwapchain::Init(IDevice* device, const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        m_device = device;

        m_name       = createInfo.name;
        m_createInfo = createInfo;

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

        result = InitSurface(createInfo);
        Validate(result);
        result = InitSwapchain(m_createInfo.imageSize, m_createInfo.minImageCount);
        Validate(result);

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

    VkSemaphore ISwapchain::GetImageAcquiredSemaphore() const
    {
        return m_imageAcquiredSemaphore[m_semaphoreIndex];
    }

    VkSemaphore ISwapchain::GetImagePresentSemaphore() const
    {
        return m_imagePresentSemaphore[m_semaphoreIndex];
    }

    ImageSemaphorePair ISwapchain::AcquireNextImage()
    {
        ZoneScoped;

        auto imageAcquiredSemaphore = m_imageAcquiredSemaphore[m_semaphoreIndex];
        auto result                 = vkAcquireNextImageKHR(m_device->m_device, m_swapchain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE, &m_imageIndex);
        TL_ASSERT(result == VK_SUCCESS);

        return {
            .semaphore = imageAcquiredSemaphore,
            .image     = m_image[m_imageIndex],
        };
    }

    ResultCode ISwapchain::Recreate(ImageSize2D newSize)
    {
        ZoneScoped;
        Validate(vkDeviceWaitIdle(m_device->m_device));
        VkResult result = InitSwapchain(newSize, m_createInfo.minImageCount);
        return ConvertResult(result);
    }

    ResultCode ISwapchain::Present()
    {
        ZoneScoped;

        auto& presentQueue = m_device->GetDeviceQueue(QueueType::Graphics);

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
        Validate(vkQueuePresentKHR(presentQueue.GetHandle(), &presentInfo));

        m_semaphoreIndex = (m_semaphoreIndex + 1) % m_imageCount;

        AcquireNextImage();

        return ResultCode::Success;
    }

    VkResult ISwapchain::InitSwapchain(ImageSize2D size, uint32_t minImageCount)
    {
        ZoneScoped;

        m_createInfo.imageSize     = size;
        m_createInfo.minImageCount = minImageCount;

        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        Validate(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_physicalDevice, m_surface, &surfaceCapabilities));

        // Clamp the image count to fit the surface supported range.
        m_imageCount = minImageCount;
        if (m_imageCount < MinImageCount || m_imageCount > MaxImageCount ||
            m_imageCount < surfaceCapabilities.minImageCount || m_imageCount > surfaceCapabilities.maxImageCount)
        {
            TL_LOG_INFO("Invalid SwapchainCreateInfo::minImageCount for the given window.");
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        // Clamp the swapchain size to fit the surface supported range.
        if (size.width < surfaceCapabilities.minImageExtent.width ||
            size.height < surfaceCapabilities.minImageExtent.height ||
            size.width > surfaceCapabilities.maxImageExtent.width ||
            size.height > surfaceCapabilities.maxImageExtent.height)
        {
            auto clampedWidth  = std::clamp(size.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
            auto clampedHeight = std::clamp(size.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

            TL_LOG_WARNNING("Can't (re)create swapchain with size ({}x{}) for the given window. Clamping to ({}, {}).",
                            size.width,
                            size.height,
                            clampedWidth,
                            clampedHeight);

            size.width  = clampedWidth;
            size.height = clampedHeight;
        }

        VkSurfaceFormatKHR selectedFormat{};
        if (SelectSurfaceFormat(*m_device, m_surface, m_createInfo.imageFormat, selectedFormat) == false)
        {
            TL_LOG_INFO("Failed to (re)create the swapchain with the required format.");
            return VK_ERROR_FORMAT_NOT_SUPPORTED;
        }

        VkPresentModeKHR selectedPresentMode{};
        if (SelectPresentMode(*m_device, m_createInfo.presentMode, m_surface, selectedPresentMode) == false)
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
            .imageExtent           = {size.width, size.height},
            .imageArrayLayers      = 1,
            .imageUsage            = ConvertImageUsageFlags(m_createInfo.imageUsage),
            .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = nullptr,
            .preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha        = SelectCompositeAlpha(surfaceCapabilities),
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
            createInfo.minImageCount = m_imageCount;
        }
        AcquireNextImage();

        return VK_SUCCESS;
    }
} // namespace RHI::Vulkan