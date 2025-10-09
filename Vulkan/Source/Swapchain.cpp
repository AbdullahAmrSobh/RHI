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
        case SwapchainAlphaMode::None:           return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        case SwapchainAlphaMode::PreMultiplied:  return VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
        case SwapchainAlphaMode::PostMultiplied: return VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
        default:                                 TL_UNREACHABLE(); return VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;
        }
    }

    ISwapchain::ISwapchain()  = default;
    ISwapchain::~ISwapchain() = default;

    ResultCode ISwapchain::Init(IDevice* device, const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        m_device = device;
        if (createInfo.name)
            m_name = createInfo.name;

        VulkanResult result = CreateSurface(*m_device, createInfo, m_surface);
        if (!result)
        {
            Shutdown(device);
            return result;
        }

        m_imageHandle = TL::construct<IImage>();

        for (uint32_t i = 0; i < MaxImageCount; ++i)
        {
            VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            result = vkCreateSemaphore(m_device->m_device, &semaphoreCI, nullptr, &m_acquireSemaphore[i]);
            TL_ASSERT(result, "Failed to create swapchain semaphore");
            if (!m_name.empty())
            {
                m_device->SetDebugName(m_acquireSemaphore[i], "swapchain: {} - acquire_semaphore[{}]", m_name, i);
            }
        }
        return result;
    }

    void ISwapchain::Shutdown(IDevice* device)
    {
        auto frame = (IFrame*)m_device->GetCurrentFrame();

        for (uint32_t i = 0; i < MaxImageCount; ++i)
        {
            if (m_acquireSemaphore[i])
            {
                m_device->m_destroyQueue->Push(frame->GetTimelineValue(), m_acquireSemaphore[i]);
                m_acquireSemaphore[i] = VK_NULL_HANDLE;
            }
        }

        for (uint32_t i = 0; i < MaxImageCount; ++i)
        {
            if (m_imageViews[i])
            {
                m_device->m_destroyQueue->Push(frame->GetTimelineValue(), m_imageViews[i]);
                m_imageViews[i] = VK_NULL_HANDLE;
            }
        }

        if (m_swapchain)
        {
            m_device->m_destroyQueue->Push(frame->GetTimelineValue(), m_swapchain);
            m_swapchain = VK_NULL_HANDLE;
        }

        if (m_surface)
        {
            m_device->m_destroyQueue->Push(frame->GetTimelineValue(), m_surface);
            m_surface = VK_NULL_HANDLE;
        }

        for (uint32_t i = 0; i < MaxImageCount; ++i)
        {
            m_images[i] = VK_NULL_HANDLE;
        }

        TL::free(m_imageHandle);
    }

    uint32_t ISwapchain::GetImagesCount() const
    {
        return m_configuration.imageCount;
    }

    Image* ISwapchain::GetImage() const
    {
        return m_imageHandle;
    }

    SurfaceCapabilities ISwapchain::GetSurfaceCapabilities() const
    {
        ZoneScoped;

        TL_ASSERT(m_surface);

        VulkanResult result;

        VkSurfaceCapabilitiesKHR capabilities;
        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_physicalDevice, m_surface, &capabilities);
        TL_ASSERT(result);

        SurfaceCapabilities output{};

        // Add image size and count limits
        {
            output.minImageSize  = {capabilities.minImageExtent.width, capabilities.minImageExtent.height};
            output.maxImageSize  = {capabilities.maxImageExtent.width, capabilities.maxImageExtent.height};
            output.minImageCount = capabilities.minImageCount;
            output.maxImageCount = capabilities.maxImageCount;
        }

        // Add image usage flags
        {
            if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) output.usages |= ImageUsage::ShaderResource;
            if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT) output.usages |= ImageUsage::StorageResource;
            if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) output.usages |= ImageUsage::Color;
            if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) output.usages |= ImageUsage::CopyDst;
        }

        // Add formats
        {
            uint32_t formatCount{0};
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, nullptr);
            TL_ASSERT(result);
            TL::Vector<VkSurfaceFormatKHR> formats(formatCount);
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, formats.data());
            TL_ASSERT(result);
            for (const auto& surfaceFormat : formats)
            {
                output.formats.push_back(ConvertFormat(surfaceFormat.format));
            }
        }

        // Add composite alpha modes
        {
            auto alpha = capabilities.supportedCompositeAlpha;
            if (alpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) output.alphaModes |= SwapchainAlphaMode::None;
            if (alpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) output.alphaModes |= SwapchainAlphaMode::PreMultiplied;
            if (alpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) output.alphaModes |= SwapchainAlphaMode::PostMultiplied;
        }

        // Add present modes
        {
            uint32_t presentModeCount{0};
            result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->m_physicalDevice, m_surface, &presentModeCount, nullptr);
            TL_ASSERT(result);
            TL::Vector<VkPresentModeKHR> presentModes(presentModeCount);
            result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->m_physicalDevice, m_surface, &presentModeCount, presentModes.data());
            TL_ASSERT(result);
            for (VkPresentModeKHR mode : presentModes)
            {
                switch (mode)
                {
                case VK_PRESENT_MODE_IMMEDIATE_KHR:    output.presentModes |= SwapchainPresentMode::Immediate; break;
                case VK_PRESENT_MODE_MAILBOX_KHR:      output.presentModes |= SwapchainPresentMode::Mailbox; break;
                case VK_PRESENT_MODE_FIFO_KHR:         output.presentModes |= SwapchainPresentMode::Fifo; break;
                case VK_PRESENT_MODE_FIFO_RELAXED_KHR: output.presentModes |= SwapchainPresentMode::FifoRelaxed; break;
                default:                               TL_UNREACHABLE(); break;
                }
            }
        }
        return output;
    }

    ResultCode ISwapchain::Resize(const ImageSize2D& size)
    {
        m_configuration.size = size;
        return Configure(m_configuration);
    }

    ResultCode ISwapchain::Configure(const SwapchainConfigureInfo& configInfo)
    {
        TL_ASSERT(configInfo.imageCount <= MaxImageCount, "Swapchain returned more images than supported.");

        m_configuration = configInfo;

        VulkanResult result;

        // Query surface capabilities
        VkSurfaceCapabilitiesKHR surfaceCaps;
        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_physicalDevice, m_surface, &surfaceCaps);
        if (!result) return ConvertResult(result);

        // Validate image usage flags
        VkImageUsageFlags imageUsage = ConvertImageUsageFlags(configInfo.imageUsage);
        if ((imageUsage & surfaceCaps.supportedUsageFlags) != imageUsage)
        {
            TL_LOG_ERROR("Failed to create swapchain, image usage flags not supported");
            return ResultCode::ErrorInvalidArguments;
        }

        // Clamp extent to supported range
        VkExtent2D extent = ConvertExtent2D(configInfo.size);
        extent.width      = std::clamp(extent.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
        extent.height     = std::clamp(extent.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
        if (extent.width != configInfo.size.width || extent.height != configInfo.size.height)
        {
            TL_LOG_WARNNING(
                "Swapchain size clamped from requested ({}, {}) to supported ({}, {})",
                configInfo.size.width,
                configInfo.size.height,
                extent.width,
                extent.height);
        }

        // Clamp image count to supported range
        uint32_t imageCount = std::clamp(configInfo.imageCount, surfaceCaps.minImageCount, surfaceCaps.maxImageCount);
        if (imageCount != configInfo.imageCount)
        {
            TL_LOG_WARNNING(
                "Swapchain image count clamped from requested ({}) to supported ({}, min: {}, max: {})",
                configInfo.imageCount,
                imageCount,
                surfaceCaps.minImageCount,
                surfaceCaps.maxImageCount);
        }

        // Select surface format
        VkSurfaceFormatKHR surfaceFormat{};
        {
            uint32_t formatCount{0};
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, nullptr);
            TL_ASSERT(result);

            TL::Vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, surfaceFormats.data());
            TL_ASSERT(result);

            VkFormat desiredFormat = ConvertFormat(m_configuration.format);
            bool     found         = false;
            for (const auto& currSurfaceFormat : surfaceFormats)
            {
                if (currSurfaceFormat.format == desiredFormat)
                {
                    surfaceFormat = currSurfaceFormat;
                    found         = true;
                    break;
                }
            }
            TL_ASSERT(found);
        }

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
        TL_ASSERT(result);

        if (m_name.empty() == false)
            m_device->SetDebugName(m_swapchain, m_name.c_str());

        // Destroy old image views and old swapchain if present
        {
            auto frame = (IFrame*)m_device->GetCurrentFrame();
            for (uint32_t i = 0; i < MaxImageCount; i++)
            {
                if (m_imageViews[i] != VK_NULL_HANDLE)
                {
                    m_device->m_destroyQueue->Push(frame->GetTimelineValue(), m_imageViews[i]);
                    m_imageViews[i] = VK_NULL_HANDLE;
                }
            }

            if (createInfo.oldSwapchain)
            {
                m_device->m_destroyQueue->Push(frame->GetTimelineValue(), createInfo.oldSwapchain);
            }
        }

        result = vkGetSwapchainImagesKHR(m_device->m_device, m_swapchain, &m_imageCount, nullptr);
        TL_ASSERT(result, "Failed to get swapchain images count");

        result = vkGetSwapchainImagesKHR(m_device->m_device, m_swapchain, &m_imageCount, m_images);
        TL_ASSERT(result, "Failed to get swapchain images count");

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
            TL_ASSERT(result, "Failed to create swapchain image view");
            if (m_name.empty() == false)
            {
                m_device->SetDebugName(m_images[i], "swapchain: {} - image [{}]", m_name, i);
                m_device->SetDebugName(m_imageViews[i], "swapchain: {} - view [{}]", m_name, i);
            }
        }

        return ConvertResult(result);
    }

    VkResult ISwapchain::AcquireNextImage(VkSemaphore& acquiredSemaphore)
    {
        VulkanResult              result;
        VkAcquireNextImageInfoKHR acquireInfo{
            .sType      = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
            .pNext      = nullptr,
            .swapchain  = m_swapchain,
            .timeout    = UINT64_MAX,
            .semaphore  = m_acquireSemaphore[m_acquireSemaphoreIndex],
            .fence      = VK_NULL_HANDLE,
            .deviceMask = 0x00000001,
        };
        result = vkAcquireNextImage2KHR(m_device->m_device, &acquireInfo, &m_imageIndex);

        auto updateCurrentImage = [this]()
        {
            auto [width, height] = m_configuration.size;
            if (auto image = (IImage*)(m_imageHandle))
            {
                image->handle     = m_images[m_imageIndex];
                image->viewHandle = m_imageViews[m_imageIndex];
                image->format     = m_configuration.format;
                image->size       = {width, height};
            }
        };

        if (result.IsSwapchainSuccess())
        {
            acquiredSemaphore = m_acquireSemaphore[m_acquireSemaphoreIndex];
            updateCurrentImage();
        }
        else if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            TL_LOG_WARNNING("Swapchain is out of date, attempting to reconfigure (result: {})", result.AsString());
            if (Configure(m_configuration) == ResultCode::Success)
            {
                acquireInfo.swapchain = m_swapchain;
                result                = vkAcquireNextImage2KHR(m_device->m_device, &acquireInfo, &m_imageIndex);
                if (result.IsSwapchainSuccess())
                {
                    acquiredSemaphore = m_acquireSemaphore[m_acquireSemaphoreIndex];
                    updateCurrentImage();
                }
                else
                {
                    TL_LOG_ERROR("Failed to acquire next image after swapchain reconfiguration (result: {})", result.AsString());
                }
            }
            else
            {
                TL_LOG_ERROR("Failed to reconfigure swapchain after out-of-date/suboptimal error.");
            }
        }
        else
        {
            TL_LOG_ERROR("Failed to acquire next swapchain image (result: {})", result.AsString());
        }

        if (result)
        {
            m_acquireSemaphoreIndex = (m_acquireSemaphoreIndex + 1) % MaxImageCount;
        }

        return result;
    }

    uint32_t ISwapchain::GetImageIndex() const
    {
        return m_imageIndex;
    }

    VkSwapchainKHR ISwapchain::GetHandle() const
    {
        return m_swapchain;
    }

} // namespace RHI::Vulkan