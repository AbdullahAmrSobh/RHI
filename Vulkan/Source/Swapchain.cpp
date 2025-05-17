#include "Swapchain.hpp"
#include "Common.hpp"
#include "Device.hpp"
#include "Resources.hpp"
#include "Frame.hpp"

#include <tracy/Tracy.hpp>

#include <TL/Log.hpp>

#include <algorithm>

namespace RHI::Vulkan
{
    inline static VkPresentModeKHR ConvertToPresentMode(SwapchainPresentMode presentMode)
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

    inline static VkCompositeAlphaFlagBitsKHR ConvertToAlphaMode(SwapchainAlphaMode alphaMode)
    {
        switch (alphaMode)
        {
        case SwapchainAlphaMode::None:          return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        case SwapchainAlphaMode::PreMultiplied: return VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
        case SwapchainAlphaMode::PostMultiplied:
            return VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
            // case SwapchainAlphaMode::: return VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        default: TL_UNREACHABLE(); return VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;
        }
    }

    ISwapchain::ISwapchain()  = default;
    ISwapchain::~ISwapchain() = default;

    ResultCode ISwapchain::Init(IDevice* device, const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        m_device = device;
        m_name   = createInfo.name;

        IImage image{}; // default image (initialized on acquire)
        m_image = m_device->m_imageOwner.Emplace(std::move(image));

        VkResult result;

        for (uint32_t i = 0; i < MaxImageCount; i++)
        {
            VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = nullptr, .flags = 0};

            result = vkCreateSemaphore(m_device->m_device, &semaphoreCI, nullptr, &m_imageAcquiredSemaphore[i]);
            TL_ASSERT(result == VK_SUCCESS, "Failed to create swapchain semaphore");
            result = vkCreateSemaphore(m_device->m_device, &semaphoreCI, nullptr, &m_imagePresentSemaphore[i]);
            TL_ASSERT(result == VK_SUCCESS, "Failed to create swapchain semaphore");

            if (m_name.empty() == false)
            {
                m_device->SetDebugName(m_imageAcquiredSemaphore[i], "swapchain: {} - acquire_semaphore[{}]", m_name, i);
                m_device->SetDebugName(m_imagePresentSemaphore[i], "swapchain: {} - present_semaphore[{}]", m_name, i);
            }
        }

        return ConvertResult(InitSurface(createInfo));
    }

    void ISwapchain::Shutdown()
    {
        auto frame = (IFrame*)m_device->GetCurrentFrame();

        for (uint32_t i = 0; i < MaxImageCount; i++)
        {
            if (m_imageAcquiredSemaphore[i] != VK_NULL_HANDLE)
                m_device->m_destroyQueue->Push(frame->m_timeline, m_imageAcquiredSemaphore[i]);

            if (m_imagePresentSemaphore[i] != VK_NULL_HANDLE)
                m_device->m_destroyQueue->Push(frame->m_timeline, m_imagePresentSemaphore[i]);
        }

        CleanupOldSwapchain(m_swapchain, m_imageCount);

        if (m_surface != VK_NULL_HANDLE)
            m_device->m_destroyQueue->Push(frame->m_timeline, m_surface);

        m_device->m_imageOwner.Release(m_image);
    }

    VkSemaphore ISwapchain::GetImageAcquiredSemaphore() const
    {
        return m_imageAcquiredSemaphore[m_acquireSemaphoreIndex];
    }

    VkSemaphore ISwapchain::GetImagePresentSemaphore() const
    {
        return m_imagePresentSemaphore[m_presentSemaphoreIndex];
    }

    SurfaceCapabilities ISwapchain::GetSurfaceCapabilities()
    {
        ZoneScoped;

        VkResult                 result;
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_physicalDevice, m_surface, &surfaceCapabilities);
        TL_ASSERT(result == VK_SUCCESS);

        SurfaceCapabilities outCaps{};
        if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) outCaps.usages |= ImageUsage::Color;
        if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) outCaps.usages |= ImageUsage::CopyDst;

        uint32_t formatCount = 0;
        result               = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, nullptr);
        TL_ASSERT(result == VK_SUCCESS);
        TL::Vector<VkSurfaceFormatKHR> formats(formatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, formats.data());
        TL_ASSERT(result == VK_SUCCESS);

        for (const auto& surfaceFormat : formats)
        {
            outCaps.formats.push_back(ConvertFormat(surfaceFormat.format));
        }

        uint32_t presentModeCount = 0;
        result                    = vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->m_physicalDevice, m_surface, &presentModeCount, nullptr);
        TL_ASSERT(result == VK_SUCCESS);
        TL::Vector<VkPresentModeKHR> presentModes(presentModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->m_physicalDevice, m_surface, &presentModeCount, presentModes.data());
        TL_ASSERT(result == VK_SUCCESS);

        for (VkPresentModeKHR mode : presentModes)
        {
            switch (mode)
            {
            case VK_PRESENT_MODE_IMMEDIATE_KHR:    outCaps.presentModes |= SwapchainPresentMode::Immediate; break;
            case VK_PRESENT_MODE_MAILBOX_KHR:      outCaps.presentModes |= SwapchainPresentMode::Mailbox; break;
            case VK_PRESENT_MODE_FIFO_KHR:         outCaps.presentModes |= SwapchainPresentMode::Fifo; break;
            case VK_PRESENT_MODE_FIFO_RELAXED_KHR: outCaps.presentModes |= SwapchainPresentMode::FifoRelaxed; break;
            default:                               TL_UNREACHABLE(); break;
            }
        }

        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) outCaps.alphaModes |= SwapchainAlphaMode::None;
        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) outCaps.alphaModes |= SwapchainAlphaMode::PreMultiplied;
        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) outCaps.alphaModes |= SwapchainAlphaMode::PostMultiplied;
        return outCaps;
    }

    ResultCode ISwapchain::Resize(ImageSize2D size)
    {
        ZoneScoped;
        m_configuration.size = size;
        return Configure(m_configuration);
    }

    ResultCode ISwapchain::Configure(const SwapchainConfigureInfo& configInfo)
    {
        vkDeviceWaitIdle(m_device->m_device);

        TL_ASSERT(configInfo.imageCount <= MaxImageCount, "Swapchain returned more images than supported.");

        m_configuration = configInfo;

        VkResult result;

        VkSurfaceCapabilitiesKHR surfaceCaps;
        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_physicalDevice, m_surface, &surfaceCaps);
        if (result) return ConvertResult(result);

        VkExtent2D extent = ConvertExtent2D(configInfo.size);
        extent.width      = std::clamp(extent.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
        extent.height     = std::clamp(extent.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
        if (extent.width != configInfo.size.width || extent.height != configInfo.size.height)
        {
            TL_LOG_WARNNING("Swapchain size clamped to ({}, {})", extent.width, extent.height);
        }

        uint32_t imageCount = std::clamp(configInfo.imageCount, surfaceCaps.minImageCount, surfaceCaps.maxImageCount);
        if (imageCount != configInfo.imageCount)
        {
            TL_LOG_WARNNING("Swapchain image count clamped to {}", imageCount);
        }

        VkImageUsageFlags imageUsage = ConvertImageUsageFlags(configInfo.imageUsage);
        if ((imageUsage & surfaceCaps.supportedUsageFlags) != imageUsage)
        {
            TL_LOG_ERROR("Failed to create swapchain, image usage flags not supported");
            return ResultCode::ErrorInvalidArguments;
        }

        auto SelectSurfaceFormat = [&]() -> VkSurfaceFormatKHR
        {
            uint32_t formatCount = 0;
            result               = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, nullptr);
            TL_ASSERT(result == VK_SUCCESS);

            TL::Vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, surfaceFormats.data());
            TL_ASSERT(result == VK_SUCCESS);

            for (auto currSurfaceFormat : surfaceFormats)
            {
                auto convertedFormat = ConvertFormat(m_configuration.format);
                if (currSurfaceFormat.format == convertedFormat)
                    return currSurfaceFormat;
            }
            TL_UNREACHABLE();
            return surfaceFormats[0];
        };

        VkSurfaceFormatKHR surfaceFormat = SelectSurfaceFormat();

        VkSwapchainCreateInfoKHR createInfo{
            .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext                 = nullptr,
            .flags                 = 0,
            .surface               = m_surface,
            .minImageCount         = imageCount,
            .imageFormat           = surfaceFormat.format,
            .imageColorSpace       = surfaceFormat.colorSpace,
            .imageExtent           = extent,
            .imageArrayLayers      = 1,
            .imageUsage            = ConvertImageUsageFlags(m_configuration.imageUsage),
            .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = nullptr,
            .preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha        = ConvertToAlphaMode(m_configuration.alphaMode),
            .presentMode           = ConvertToPresentMode(m_configuration.presentMode),
            .clipped               = VK_TRUE,
            .oldSwapchain          = m_swapchain,
        };
        result = vkCreateSwapchainKHR(m_device->m_device, &createInfo, nullptr, &m_swapchain);
        vkDeviceWaitIdle(m_device->m_device);

        if (m_name.empty() == false)
            m_device->SetDebugName(m_swapchain, m_name.c_str());

        CleanupOldSwapchain(createInfo.oldSwapchain, m_imageCount);

        result = vkGetSwapchainImagesKHR(m_device->m_device, m_swapchain, &m_imageCount, nullptr);
        TL_ASSERT(result == VK_SUCCESS, "Failed to get swapchain images count");

        result = vkGetSwapchainImagesKHR(m_device->m_device, m_swapchain, &m_imageCount, m_images);
        TL_ASSERT(result == VK_SUCCESS, "Failed to get swapchain images count");

        for (uint32_t i = 0; i < m_imageCount; i++)
        {
            VkImageViewCreateInfo imageViewCI{
                .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext            = nullptr,
                .flags            = 0,
                .image            = m_images[i],
                .viewType         = VK_IMAGE_VIEW_TYPE_2D,
                .format           = surfaceFormat.format,
                .components       = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS},
            };
            result = vkCreateImageView(m_device->m_device, &imageViewCI, nullptr, &m_imageViews[i]);
            TL_ASSERT(result == VK_SUCCESS, "Failed to create swapchain image view");
            if (m_name.empty() == false)
            {
                m_device->SetDebugName(m_images[i], "swapchain: {} - image [{}]", m_name, i);
                m_device->SetDebugName(m_imageViews[i], "swapchain: {} - view [{}]", m_name, i);
            }
        }

        result = AcquireNextImage();

        return ConvertResult(result);
    }

    ResultCode ISwapchain::Present()
    {
        ZoneScoped;

        VkResult result;

        VkSemaphore waitSemaphore = GetImagePresentSemaphore();

        VkPresentInfoKHR presentInfo{
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext              = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &waitSemaphore,
            .swapchainCount     = 1,
            .pSwapchains        = &m_swapchain,
            .pImageIndices      = &m_imageIndex,
            .pResults           = &m_lastPresentResult,
        };

        auto queue = m_device->GetDeviceQueue(QueueType::Graphics);
        result     = vkQueuePresentKHR(queue->GetHandle(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            TL_LOG_INFO("Swapchain out of date, reconfiguring");
            return Configure(m_configuration);
        }
        else if (result != VK_SUCCESS)
        {
            TL_LOG_ERROR("Failed to present swapchain image");
            return ConvertResult(result);
        }

        {
            // wait for previous swapchain frame in flight
            queue->WaitTimeline(m_timelineValue[m_imageIndex]);
            m_timelineValue[m_imageIndex] = queue->GetTimelineValue();
        }

        m_acquireSemaphoreIndex = m_acquireSemaphoreNextIndex;

        result = AcquireNextImage();
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            TL_LOG_INFO("Swapchain out of date, reconfiguring");
            return Configure(m_configuration);
        }
        else if (result != VK_SUCCESS)
        {
            TL_LOG_ERROR("Failed to acquire next swapchain image");
            return ConvertResult(result);
        }

        m_presentSemaphoreIndex = (m_presentSemaphoreIndex + 1) % MaxImageCount;

        return ConvertResult(result);
    }

    VkResult ISwapchain::AcquireNextImage()
    {
        VkResult                  result;
        VkSemaphore               semaphore = GetImageAcquiredSemaphore();
        VkAcquireNextImageInfoKHR acquireInfo{
            .sType      = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
            .pNext      = nullptr,
            .swapchain  = m_swapchain,
            .timeout    = UINT64_MAX,
            .semaphore  = semaphore,
            .fence      = VK_NULL_HANDLE,
            .deviceMask = 0x00000001,
        };
        result = vkAcquireNextImage2KHR(m_device->m_device, &acquireInfo, &m_imageIndex);
        if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
        {
            auto image        = m_device->m_imageOwner.Get(m_image);
            image->handle     = m_images[m_imageIndex];
            image->viewHandle = m_imageViews[m_imageIndex];
            image->format     = m_configuration.format;
            image->size       = {m_size.width, m_size.height};

            m_acquireSemaphoreNextIndex = (m_acquireSemaphoreIndex + 1) % MaxImageCount;
        }
        return result;
    }

    void ISwapchain::CleanupOldSwapchain(VkSwapchainKHR oldSwapchain, uint32_t oldImageCount)
    {
        auto frame = (IFrame*)m_device->GetCurrentFrame();

        for (uint32_t i = 0; i < oldImageCount; i++)
        {
            m_device->m_destroyQueue->Push(frame->m_timeline, m_imageViews[i]);
            m_imageViews[i] = VK_NULL_HANDLE;
        }

        m_device->m_destroyQueue->Push(frame->m_timeline, oldSwapchain);
    }
} // namespace RHI::Vulkan