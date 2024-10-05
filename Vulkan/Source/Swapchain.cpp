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

        for (uint32_t i = 0; i < m_imageCount; i++)
        {
            if (m_image[i] != NullHandle) context->m_imageOwner.Release(m_image[i]);
            if (m_waitSemaphore[i] != NullHandle) context->DestroySemaphore(m_waitSemaphore[i]);
            if (m_signalSemaphore[i] != NullHandle) context->DestroySemaphore(m_signalSemaphore[i]);
        }

        vkDestroySwapchainKHR(context->m_device, m_swapchain, nullptr);
        vkDestroySurfaceKHR(context->m_instance, m_surface, nullptr);
    }

    VkResult ISwapchain::Init(const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        m_name = createInfo.name ? createInfo.name : "";
        m_imageSize = createInfo.imageSize;
        m_imageUsage = createInfo.imageUsage;
        m_imageFormat = createInfo.imageFormat;
        m_presentMode = createInfo.presentMode;
        m_imageCount = createInfo.minImageCount;

        for (uint32_t i = 0; i < MaxImageCount; i++)
        {
            m_signalSemaphore[i] = m_context->CreateSemaphore({ std::format("swapchain-signal-semaphore {}", i).c_str(), false });
            m_waitSemaphore[i] = m_context->CreateSemaphore({ std::format("swapchain-wait-semaphore {}", i).c_str(), false });
        }

        InitSurface(createInfo);
        InitSwapchain();

        return VK_SUCCESS;
    }

    ResultCode ISwapchain::AcquireNextImage()
    {
        ZoneScoped;

        auto context = (IContext*)m_context;
        auto signalSemaphore = context->m_semaphoreOwner.Get(GetWaitSemaphore())->handle;
        Validate(vkAcquireNextImageKHR(context->m_device, m_swapchain, UINT64_MAX, signalSemaphore, VK_NULL_HANDLE, &m_imageIndex));
        return ResultCode::Success;
    }

    ResultCode ISwapchain::Recreate(ImageSize2D newSize)
    {
        ZoneScoped;

        m_imageSize = newSize;

        auto result = InitSwapchain();
        return ConvertResult(result);
    }

    ResultCode ISwapchain::Present()
    {
        ZoneScoped;

        auto context = (IContext*)m_context;
        auto waitSemaphore = context->m_semaphoreOwner.Get(GetSignalSemaphore());

        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &waitSemaphore->handle,
            .swapchainCount = 1,
            .pSwapchains = &m_swapchain,
            .pImageIndices = &m_imageIndex,
            .pResults = &m_lastPresentResult,
        };
        Validate(vkQueuePresentKHR(context->m_queue[(uint32_t)QueueType::Graphics].GetHandle(), &presentInfo));

        RotateSemaphores();

        Validate(AcquireNextImage());

        return ResultCode::Success;
    }

    VkResult ISwapchain::InitSwapchain()
    {
        auto context = (IContext*)m_context;

        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        Validate(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->m_physicalDevice, m_surface, &surfaceCapabilities));

        if (m_imageCount < MinImageCount || m_imageCount > MaxImageCount)
        {
            TL_LOG_INFO("Failed to create the swapchain, invalid SwapchainCreateInfo::minImageCount.");
            return VK_ERROR_UNKNOWN;
        }
        else if (m_imageCount < surfaceCapabilities.minImageCount || m_imageCount > surfaceCapabilities.maxImageCount)
        {
            TL_LOG_INFO("Failed to create the swapchain, invalid SwapchainCreateInfo::minImageCount for the given window");
            return VK_ERROR_UNKNOWN;
        }
        else if (
            m_imageSize.width < surfaceCapabilities.minImageExtent.width ||
            m_imageSize.height < surfaceCapabilities.minImageExtent.height ||
            m_imageSize.width > surfaceCapabilities.maxImageExtent.width ||
            m_imageSize.height > surfaceCapabilities.maxImageExtent.height)
        {
            TL_LOG_WARNNING("Swapchain requested size will be clamped to fit into window's supported size range");
        }

        m_imageSize.width = std::clamp(m_imageSize.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        m_imageSize.height = std::clamp(m_imageSize.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

        uint32_t formatsCount;
        Validate(vkGetPhysicalDeviceSurfaceFormatsKHR(context->m_physicalDevice, m_surface, &formatsCount, nullptr));
        TL::Vector<VkSurfaceFormatKHR> formats{};
        formats.resize(formatsCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(context->m_physicalDevice, m_surface, &formatsCount, formats.data());

        bool formatFound = false;
        VkSurfaceFormatKHR selectedFormat = {};
        for (auto surfaceFormat : formats)
        {
            if (surfaceFormat.format == ConvertFormat(m_imageFormat))
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
            if (supportedMode == ConvertPresentMode(m_presentMode))
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

        auto [width, height] = m_imageSize;

        auto oldSwapchain = m_swapchain;
        VkSwapchainCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = m_surface,
            .minImageCount = m_imageCount,
            .imageFormat = selectedFormat.format,
            .imageColorSpace = selectedFormat.colorSpace,
            .imageExtent = {width, height},
            .imageArrayLayers = 1,
            .imageUsage = ConvertImageUsageFlags(m_imageUsage),
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = selectedCompositeAlpha,
            .presentMode = ConvertPresentMode(m_presentMode),
            .clipped = VK_TRUE,
            .oldSwapchain = m_swapchain,
        };
        Validate(vkCreateSwapchainKHR(context->m_device, &createInfo, nullptr, &m_swapchain));
        context->SetDebugName(m_swapchain, m_name.c_str());

        if (oldSwapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(context->m_device, oldSwapchain, nullptr);
        }

        Validate(vkGetSwapchainImagesKHR(context->m_device, m_swapchain, &m_imageCount, nullptr));
        TL_ASSERT(m_imageCount <= MaxImageCount, "Swapchain returned larger count than supported.");
        VkImage images[MaxImageCount];
        Validate(vkGetSwapchainImagesKHR(context->m_device, m_swapchain, &m_imageCount, images));

        for (uint32_t imageIndex = 0; imageIndex < m_imageCount; imageIndex++)
        {
            IImage image{};
            Validate(image.Init(context, images[imageIndex], createInfo));
            m_image[imageIndex] = context->m_imageOwner.Emplace(std::move(image));
        }

        Validate(AcquireNextImage());

        return VK_SUCCESS;
    }
} // namespace RHI::Vulkan