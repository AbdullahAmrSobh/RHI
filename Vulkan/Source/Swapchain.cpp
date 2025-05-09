#include "Swapchain.hpp"

#include <TL/Log.hpp>

#include <tracy/Tracy.hpp>

#include "Common.hpp"
#include "Device.hpp"
#include "Resources.hpp"

#include <algorithm>

namespace RHI::Vulkan
{

#define CHECK_RESULT(result)          \
    if (result != VK_SUCCESS)         \
    {                                 \
        Shutdown();                   \
        return ConvertResult(result); \
    }

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

        m_name = createInfo.name;

        VkResult result;

        for (uint32_t i = 0; i < MaxImageCount; i++)
        {
            VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = nullptr, .flags = 0};

            result = vkCreateSemaphore(m_device->m_device, &semaphoreCI, nullptr, &m_imageAcquiredSemaphore[i]);
            CHECK_RESULT(result);

            result = vkCreateSemaphore(m_device->m_device, &semaphoreCI, nullptr, &m_imagePresentSemaphore[i]);
            CHECK_RESULT(result);

            if (!m_name.empty())
            {
                m_device->SetDebugName(m_imageAcquiredSemaphore[i], "Swapchain-{}-imageAcquiredSemaphore[{}]", m_name, i);
                m_device->SetDebugName(m_imagePresentSemaphore[i], "Swapchain-{}-imagePresentSemaphore[{}]", m_name, i);
            }
        }

        return ConvertResult(InitSurface(createInfo));
    }

    void ISwapchain::Shutdown()
    {
        ZoneScoped;

        VkResult result;
        result = vkDeviceWaitIdle(m_device->m_device);
        TL_ASSERT(result == VK_SUCCESS);

        for (uint32_t i = 0; i < MaxImageCount; i++)
        {
            if (m_imageAcquiredSemaphore[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(m_device->m_device, m_imageAcquiredSemaphore[i], nullptr);

            if (m_imagePresentSemaphore[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(m_device->m_device, m_imagePresentSemaphore[i], nullptr);

            if (m_imageViews[i] != VK_NULL_HANDLE)
                vkDestroyImageView(m_device->m_device, m_imageViews[i], nullptr);
        }

        if (m_swapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(m_device->m_device, m_swapchain, nullptr);

        if (m_surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(m_device->m_instance, m_surface, nullptr);

        m_device->m_imageOwner.Release(m_image);
    }

    VkSemaphore ISwapchain::GetImageAcquiredSemaphore() const
    {
        return m_imageAcquiredSemaphore[m_currentSemaphoreIndex];
    }

    VkSemaphore ISwapchain::GetImagePresentSemaphore() const
    {
        return m_imagePresentSemaphore[m_currentSemaphoreIndex];
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
        ZoneScoped;

        m_configuration = configInfo;

        VkResult result;

        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_physicalDevice, m_surface, &surfaceCapabilities);
        TL_ASSERT(result == VK_SUCCESS);

        VkSurfaceFormatKHR surfaceFormat{};

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
            {
                surfaceFormat = currSurfaceFormat;
                break;
            }
        }
        if (m_configuration.imageCount < surfaceCapabilities.minImageCount || m_configuration.imageCount > surfaceCapabilities.maxImageCount)
        {
            TL_LOG_WARNNING("Invalid SwapchainCreateInfo::minImageCount ({}) for the given window.", m_configuration.imageCount);
            m_configuration.imageCount = std::clamp(m_configuration.imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
        }
        if (m_configuration.size.width < surfaceCapabilities.minImageExtent.width || m_configuration.size.height < surfaceCapabilities.minImageExtent.height || m_configuration.size.width > surfaceCapabilities.maxImageExtent.width || m_configuration.size.height > surfaceCapabilities.maxImageExtent.height)
        {
            auto clampedWidth  = std::clamp(m_configuration.size.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
            auto clampedHeight = std::clamp(m_configuration.size.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
            TL_LOG_WARNNING("Can't (re)create swapchain with size ({}x{}) for the given window. Clamping to ({}, {}).", m_configuration.size.width, m_configuration.size.height, clampedWidth, clampedHeight);
            m_configuration.size.width  = clampedWidth;
            m_configuration.size.height = clampedHeight;
        }

        VkSwapchainCreateInfoKHR createInfo{
            .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext                 = nullptr,
            .flags                 = 0,
            .surface               = m_surface,
            .minImageCount         = m_configuration.imageCount,
            .imageFormat           = surfaceFormat.format,
            .imageColorSpace       = surfaceFormat.colorSpace,
            .imageExtent           = {m_configuration.size.width, m_configuration.size.height},
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
        CHECK_RESULT(result);

        m_device->SetDebugName(m_swapchain, m_name.c_str());
        if (createInfo.oldSwapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(m_device->m_device, createInfo.oldSwapchain, nullptr);
        }

        result = vkGetSwapchainImagesKHR(m_device->m_device, m_swapchain, &m_imageCount, nullptr);
        CHECK_RESULT(result);
        TL_ASSERT(m_imageCount <= MaxImageCount, "Swapchain returned more images than supported.");
        result = vkGetSwapchainImagesKHR(m_device->m_device, m_swapchain, &m_imageCount, m_images);
        CHECK_RESULT(result);

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
            CHECK_RESULT(result);
            if (m_name.empty() == false)
            {
                m_device->SetDebugName(m_imageViews[i], "Swapchain-{}-ImageView[{}]", m_name, i);
            }
        }

        IImage image{}; // default image (initialized on acquire)
        image.format = configInfo.format;
        m_image      = m_device->m_imageOwner.Emplace(std::move(image));

        {
            auto semaphore = m_imageAcquiredSemaphore[m_currentSemaphoreIndex];
            result         = vkAcquireNextImageKHR(m_device->m_device, m_swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &m_currentImageIndex);
            TL_ASSERT(result == VK_SUCCESS);

            auto image        = m_device->m_imageOwner.Get(m_image);
            image->handle     = m_images[m_currentImageIndex];
            image->viewHandle = m_imageViews[m_currentImageIndex];
        }

        return ConvertResult(result);
    }

    ResultCode ISwapchain::Present()
    {
        ZoneScoped;

        auto& presentQueue = *m_device->GetDeviceQueue(QueueType::Graphics);

        VkPresentInfoKHR presentInfo{
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext              = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &m_imagePresentSemaphore[m_currentSemaphoreIndex],
            .swapchainCount     = 1,
            .pSwapchains        = &m_swapchain,
            .pImageIndices      = &m_currentImageIndex,
            .pResults           = &m_lastPresentResult,
        };
        VkResult result = vkQueuePresentKHR(presentQueue.GetHandle(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            return ResultCode::Success;

        m_currentSemaphoreIndex = (m_currentSemaphoreIndex + 1) % m_imageCount;

        auto semaphore = m_imageAcquiredSemaphore[m_currentSemaphoreIndex];
        result         = vkAcquireNextImageKHR(m_device->m_device, m_swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &m_currentImageIndex);
        TL_ASSERT(result == VK_SUCCESS);

        auto image        = m_device->m_imageOwner.Get(m_image);
        image->handle     = m_images[m_currentImageIndex];
        image->viewHandle = m_imageViews[m_currentImageIndex];

        return ConvertResult(result);
    }
} // namespace RHI::Vulkan