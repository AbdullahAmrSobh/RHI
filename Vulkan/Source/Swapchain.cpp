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
        : Swapchain(context)
        , m_swapchain(VK_NULL_HANDLE)
        , m_surface(VK_NULL_HANDLE)
        , m_lastPresentResult()
        , m_compositeAlpha()
        , m_surfaceFormat()
    {
        ZoneScoped;
    }

    ISwapchain::~ISwapchain()
    {
        ZoneScoped;

        auto context = (IContext*)m_context;
        vkDestroySwapchainKHR(context->m_device, m_swapchain, nullptr);
        vkDestroySurfaceKHR(context->m_instance, m_surface, nullptr);
    }

    VkResult ISwapchain::Init(const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        auto context = (IContext*)m_context;

        m_createInfo = createInfo;
        m_name = createInfo.name ? createInfo.name : "";
        m_createInfo.name = m_name.c_str();

        InitSurface(createInfo);

        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        Validate(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->m_physicalDevice, m_surface, &surfaceCapabilities));

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
            Validate(vkGetPhysicalDeviceSurfaceFormatsKHR(context->m_physicalDevice, m_surface, &formatsCount, nullptr));
            TL::Vector<VkSurfaceFormatKHR> formats{};
            formats.resize(formatsCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(context->m_physicalDevice, m_surface, &formatsCount, formats.data());

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
            Validate(vkGetPhysicalDeviceSurfacePresentModesKHR(context->m_physicalDevice, m_surface, &presentModesCount, nullptr));
            TL::Vector<VkPresentModeKHR> presentModes{};
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
        return ConvertResult(result);
    }

    ResultCode ISwapchain::Present()
    {
        ZoneScoped;

        auto context = (IContext*)m_context;

        auto nextImageIndex = (m_currentImageIndex + 1) % m_swapchainImagesCount;
        auto currentImage = context->m_imageOwner.Get(m_images[m_currentImageIndex]);
        auto nextImage = context->m_imageOwner.Get(m_images[nextImageIndex]);

        vkDeviceWaitIdle(context->m_device);

        TL::Vector<VkSemaphore> waitSemaphores;
        for (auto semaphore : currentImage->finalState.semaphores)
        {
            waitSemaphores.push_back(semaphore.semaphore);
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.size();
        presentInfo.pWaitSemaphores = waitSemaphores.data();
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapchain;
        presentInfo.pImageIndices = &m_currentImageIndex;
        presentInfo.pResults = &m_lastPresentResult;
        Validate(vkQueuePresentKHR(context->m_queue[QueueType::Graphics].GetHandle(), &presentInfo));

        vkDeviceWaitIdle(context->m_device);

        auto signalSemaphore = nextImage->initialState.semaphores.front().semaphore;
        Validate(vkAcquireNextImageKHR(context->m_device, m_swapchain, UINT64_MAX, signalSemaphore, VK_NULL_HANDLE, &m_currentImageIndex));

        vkDeviceWaitIdle(context->m_device);

        return ResultCode::Success;
    }

    VkResult ISwapchain::InitSwapchain()
    {
        auto context = (IContext*)m_context;

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
        Validate(vkCreateSwapchainKHR(context->m_device, &createInfo, nullptr, &m_swapchain));
        context->SetDebugName(m_swapchain, m_name.c_str());

        uint32_t imagesCount;
        Validate(vkGetSwapchainImagesKHR(context->m_device, m_swapchain, &imagesCount, nullptr));
        TL::Vector<VkImage> images;
        images.resize(imagesCount);
        Validate(vkGetSwapchainImagesKHR(context->m_device, m_swapchain, &imagesCount, images.data()));

        for (uint32_t imageIndex = 0; imageIndex < m_swapchainImagesCount; imageIndex++)
        {
            IImage image{};

            VkSemaphoreSubmitInfo semaphoreInfo{};

            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            semaphoreInfo.semaphore = context->CreateSemaphore();
            image.initialState.semaphores.push_back(semaphoreInfo);
            image.initialState.pipelineStage = PIPELINE_IMAGE_BARRIER_UNDEFINED;

            semaphoreInfo.semaphore = context->CreateSemaphore();
            image.finalState.semaphores.push_back(semaphoreInfo);
            image.finalState.pipelineStage = PIPELINE_IMAGE_BARRIER_PRESENT_SRC;

            Validate(image.Init(context, images[imageIndex], createInfo));
            m_images[imageIndex] = context->m_imageOwner.Emplace(std::move(image));
        }
        return VK_SUCCESS;
    }
} // namespace RHI::Vulkan