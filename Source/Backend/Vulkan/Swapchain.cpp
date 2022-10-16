#include "RHI/Common.hpp"
#ifdef RHI_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(RHI_LINUX)
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include "Backend/Vulkan/Swapchain.hpp"
#include "Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{

#ifdef RHI_LINUX
    Expected<Unique<ISurface>> Instance::CreateSurface(const struct X11SurfaceDesc& desc)
    {
        return Unexpected(EResultCode::Fail);
    }
    
    
    VkResult Surface::Init(const X11SurfaceDesc& desc)
    {
        return VK_ERROR_UNKNOWN;
    }
#elif defined(RHI_WINDOWS)
    Expected<Unique<ISurface>> Instance::CreateSurface(const Win32SurfaceDesc& desc)
    {
        Unique<Surface> surface = CreateUnique<Surface>(*this);
        VkResult        result  = surface->Init(desc);

        if (RHI_SUCCESS(result))
            return std::move(surface);

        return Unexpected(ConvertResult(result));
    }

    VkResult Surface::Init(const Win32SurfaceDesc& desc)
    {
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext     = nullptr;
        createInfo.flags     = 0;
        createInfo.hinstance = desc.instance;
        createInfo.hwnd      = desc.hwnd;

        static PFN_vkCreateWin32SurfaceKHR createSurface =
            (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(m_pInstance->GetHandle(), "vkCreateWin32SurfaceKHR");
        return createSurface(m_pInstance->GetHandle(), &createInfo, nullptr, &m_handle);
    }

#endif

    Expected<Unique<ISwapchain>> Device::CreateSwapChain(const SwapchainDesc& desc)
    {
        Unique<Swapchain> swapchain = CreateUnique<Swapchain>(*this);
        VkResult          result    = swapchain->Init(desc);

        if (RHI_SUCCESS(result))
            return std::move(swapchain);
        
        return Unexpected(ConvertResult(result));
    }

    Surface::~Surface()
    {
        vkDestroySurfaceKHR(m_pInstance->GetHandle(), m_handle, nullptr);
    }

    VkBool32 Surface::QueueSupportPresent(const class Queue& queue) const
    {
        VkBool32 support = VK_FALSE;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(static_cast<const PhysicalDevice&>(m_pDevice->GetPhysicalDevice()).GetHandle(),
                                                               queue.GetFamilyIndex(), m_handle, &support);
        if (!RHI_SUCCESS(result))
        {
            return VK_FALSE;
        }
        return support;
    }

    std::vector<VkSurfaceFormatKHR> Surface::GetSupportedFormats()
    {
        uint32_t count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(count);
        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, formats.data());
        assert(RHI_SUCCESS(result));
        return formats;
    }

    std::vector<VkPresentModeKHR> Surface::GetSupportedPresentModes()
    {
        uint32_t count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, nullptr);
        std::vector<VkPresentModeKHR> modes(count);
        VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, modes.data());
        assert(RHI_SUCCESS(result));
        return modes;
    }

    VkSurfaceCapabilities2KHR Surface::GetCapabilities2()
    {
        VkSurfaceCapabilities2KHR       capabilities;
        VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo;
        surfaceInfo.sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
        surfaceInfo.pNext   = nullptr;
        surfaceInfo.surface = m_handle;
        VkResult result     = vkGetPhysicalDeviceSurfaceCapabilities2KHR(m_pDevice->GetPhysicalDevice().GetHandle(), &surfaceInfo, &capabilities);
        assert(RHI_SUCCESS(result));
        return capabilities;
    }

    VkSurfaceFormatKHR Surface::SelectFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        for (const auto& availableFormat : formats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }
        return formats[0];
    }

    VkExtent2D Surface::ClampExtent(VkExtent2D actualExtent, VkExtent2D currentExtent, VkExtent2D minImageExtent, VkExtent2D maxImageExtent)
    {
        if (actualExtent.width != UINT32_MAX)
        {
            return currentExtent;
        }
        else
        {
            return {std::clamp(actualExtent.width, minImageExtent.width, maxImageExtent.width),
                    std::clamp(actualExtent.height, minImageExtent.height, maxImageExtent.height)};
        }
    }

    VkPresentModeKHR Surface::SelectPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
    {
        return std::find_if(presentModes.begin(), presentModes.end(), [](VkPresentModeKHR mode) { return mode == VK_PRESENT_MODE_MAILBOX_KHR; }) ==
                       presentModes.end()
                   ? VK_PRESENT_MODE_MAILBOX_KHR
                   : VK_PRESENT_MODE_FIFO_KHR;
    }

    Swapchain::~Swapchain()
    {
        for (auto image : m_backBuffers)
        {
            delete image;
        }

        vkDestroySwapchainKHR(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult Swapchain::Init(const SwapchainDesc& desc)
    {
        Surface& surface = *static_cast<Surface*>(m_pSurface);

        VkSurfaceCapabilities2KHR       surfaceCaps             = surface.GetCapabilities2();
        std::vector<VkPresentModeKHR>   availablePresentModes   = surface.GetSupportedPresentModes();
        std::vector<VkSurfaceFormatKHR> availableSurfaceFormats = surface.GetSupportedFormats();

        VkSurfaceFormatKHR selectedFormat = Surface::SelectFormat(availableSurfaceFormats);

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext                 = nullptr;
        createInfo.flags                 = 0;
        createInfo.surface               = surface.GetHandle();
        createInfo.minImageCount         = desc.backImagesCount;
        createInfo.imageFormat           = selectedFormat.format;
        createInfo.imageColorSpace       = selectedFormat.colorSpace;
        createInfo.imageExtent           = ConvertExtent(desc.extent);
        createInfo.imageArrayLayers      = 1;
        createInfo.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
        createInfo.preTransform          = surfaceCaps.surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode           = Surface::SelectPresentMode(availablePresentModes);
        createInfo.clipped               = VK_FALSE;
        createInfo.oldSwapchain          = VK_NULL_HANDLE;

        RHI_RETURN_ON_FAIL(vkCreateSwapchainKHR(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle));

        return InitBackbuffers();
    }

    EResultCode Swapchain::SwapBuffers()
    {
        VkAcquireNextImageInfoKHR acquireInfo;
        acquireInfo.sType      = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
        acquireInfo.pNext      = nullptr;
        acquireInfo.swapchain  = m_handle;
        acquireInfo.timeout    = UINT32_MAX;
        acquireInfo.semaphore  = m_imageAcquiredSemaphore.GetHandle();
        acquireInfo.fence      = VK_NULL_HANDLE;
        acquireInfo.deviceMask = 0;

        VkResult result = vkAcquireNextImage2KHR(m_pDevice->GetHandle(), &acquireInfo, &m_currentImageIndex);
        return ConvertResult(result);
    }

    EResultCode Swapchain::Resize(Extent2D newExtent)
    {
        return EResultCode::Fail;
    }

    EResultCode Swapchain::SetFullscreenExeclusive(bool enable)
    {
        return EResultCode::Fail;
    }

    VkResult Swapchain::InitBackbuffers()
    {
        uint32_t             backBuffersCount;
        std::vector<VkImage> backImagesHandles;
        VkResult             result = vkGetSwapchainImagesKHR(m_pDevice->GetHandle(), m_handle, &backBuffersCount, nullptr);

        if (!RHI_SUCCESS(result))
            return result;

        backImagesHandles.resize(backBuffersCount);

        vkGetSwapchainImagesKHR(m_pDevice->GetHandle(), m_handle, &backBuffersCount, backImagesHandles.data());

        if (!RHI_SUCCESS(result))
            return result;
        
        m_backBuffers.reserve(backBuffersCount);

        for (VkImage imageHandle : backImagesHandles)
        {
            m_backBuffers.push_back(static_cast<IImage*>(new Image(*m_pDevice, imageHandle)));
        }
        
        return result;
    }

} // namespace Vulkan
} // namespace RHI