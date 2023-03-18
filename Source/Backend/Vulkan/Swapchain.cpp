#define VK_USE_PLATFORM_WIN32_KHR
#include "RHI/Pch.hpp"

#include "Backend/Vulkan/Common.hpp"

#include "Backend/Vulkan/Swapchain.hpp"

#include "Backend/Vulkan/CommandQueue.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/Image.hpp"
#include "Backend/Vulkan/Instance.hpp"

namespace RHI
{
namespace Vulkan
{

Expected<std::unique_ptr<ISurface>> Instance::CreateSurface(const Win32SurfaceDesc& desc)
{
    std::unique_ptr<Surface> surface = std::make_unique<Surface>(*this);
    VkResult        result  = surface->Init(desc);

    if (Utils::IsSuccess(result))
        return std::move(surface);

    return Unexpected(ConvertResult(result));
}

VkResult Surface::Init(const Win32SurfaceDesc& desc)
{
    VkWin32SurfaceCreateInfoKHR createInfo {};
    createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext     = nullptr;
    createInfo.flags     = 0;
    createInfo.hinstance = (HINSTANCE)desc.instance;
    createInfo.hwnd      = (HWND)desc.hwnd;

    static PFN_vkCreateWin32SurfaceKHR createSurface =
        (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(m_pInstance->GetHandle(), "vkCreateWin32SurfaceKHR");
    return createSurface(m_pInstance->GetHandle(), &createInfo, nullptr, &m_handle);
}

Expected<std::unique_ptr<ISwapchain>> Device::CreateSwapChain(const SwapchainDesc& desc)
{
    std::unique_ptr<Swapchain> swapchain = std::make_unique<Swapchain>(*this);
    VkResult          result    = swapchain->Init(desc);

    if (Utils::IsSuccess(result))
        return std::move(swapchain);

    return Unexpected(ConvertResult(result));
}

Surface::~Surface()
{
    vkDestroySurfaceKHR(m_pInstance->GetHandle(), m_handle, nullptr);
}

VkBool32 Surface::QueryQueueFamilyPresentSupport(const PhysicalDevice& physicalDevice, uint32_t familyIndex) const
{
    VkBool32 support = VK_FALSE;

    VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.GetHandle(), familyIndex, m_handle, &support);

    Utils::AssertSuccess(result);

    return support;
}

std::vector<VkSurfaceFormatKHR> Surface::GetSupportedFormats(const PhysicalDevice& physicalDevice) const
{
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.GetHandle(), m_handle, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(count);
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.GetHandle(), m_handle, &count, formats.data());
    Utils::AssertSuccess(result);
    return formats;
}

std::vector<VkPresentModeKHR> Surface::GetSupportedPresentModes(const PhysicalDevice& physicalDevice) const
{
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.GetHandle(), m_handle, &count, nullptr);
    std::vector<VkPresentModeKHR> modes(count);
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.GetHandle(), m_handle, &count, modes.data());
    Utils::AssertSuccess(result);
    return modes;
}

VkSurfaceCapabilitiesKHR Surface::GetCapabilities(const PhysicalDevice& physicalDevice) const
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkResult                 result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.GetHandle(), m_handle, &capabilities);
    Utils::AssertSuccess(result);
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
    return std::find_if(presentModes.begin(), presentModes.end(), [](VkPresentModeKHR mode) { return mode == VK_PRESENT_MODE_MAILBOX_KHR; })
                   == presentModes.end()
               ? VK_PRESENT_MODE_MAILBOX_KHR
               : VK_PRESENT_MODE_FIFO_KHR;
}

Swapchain::~Swapchain()
{
    for(auto& image : m_images)
    {
        image.release();
    }
    vkDestroySwapchainKHR(m_device->GetHandle(), m_handle, nullptr);
}

VkResult Swapchain::Init(const SwapchainDesc& desc)
{
    m_imageDescription = std::make_unique<ImageDesc>();
    m_imageDescription->arraySize      = 1;
    m_imageDescription->mipLevelsCount = 1;
    m_imageDescription->extent.sizeX   = desc.extent.sizeX;
    m_imageDescription->extent.sizeY   = desc.extent.sizeY;
    m_imageDescription->extent.sizeZ   = 1;
    m_imageDescription->sampleCount    = RHI::SampleCount::Count1;
    m_imageDescription->usage          = RHI::ImageUsageFlagBits::Color;
    m_imageDescription->format         = RHI::Format::R32G32B32_FLOAT;
    m_imageReadySemaphore              = Semaphore::Create(*m_device);

    m_pSurface       = desc.pSurface;
    Surface& surface = *static_cast<Surface*>(m_pSurface);

    VkSurfaceCapabilitiesKHR        surfaceCaps             = surface.GetCapabilities(m_device->GetPhysicalDevice());
    std::vector<VkPresentModeKHR>   availablePresentModes   = surface.GetSupportedPresentModes(m_device->GetPhysicalDevice());
    std::vector<VkSurfaceFormatKHR> availableSurfaceFormats = surface.GetSupportedFormats(m_device->GetPhysicalDevice());

    VkSurfaceFormatKHR selectedFormat = Surface::SelectFormat(availableSurfaceFormats);

    VkSwapchainCreateInfoKHR createInfo {};
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
    createInfo.preTransform          = surfaceCaps.currentTransform;
    createInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode           = Surface::SelectPresentMode(availablePresentModes);
    createInfo.clipped               = VK_FALSE;
    createInfo.oldSwapchain          = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(m_device->GetHandle(), &createInfo, nullptr, &m_handle);

    VK_RETURN_ON_ERROR(result);

    uint32_t             backBuffersCount;
    std::vector<VkImage> backImagesHandles;
    Utils::AssertSuccess(vkGetSwapchainImagesKHR(m_device->GetHandle(), m_handle, &backBuffersCount, nullptr));
    backImagesHandles.resize(backBuffersCount);
    Utils::AssertSuccess(vkGetSwapchainImagesKHR(m_device->GetHandle(), m_handle, &backBuffersCount, backImagesHandles.data()));

    m_images.reserve(backBuffersCount);

    for (VkImage imageHandle : backImagesHandles)
    {
        m_images.emplace_back(static_cast<IImage*>(new Image(*m_device, imageHandle)));
    }

    SwapImages();

    for(uint32_t index = 0; index < backBuffersCount; index++)
    {
        m_framesInFlightFence.emplace_back(std::move(std::make_unique<Fence>(*m_device)));
        Utils::AssertSuccess(m_framesInFlightFence.back()->Init());
    }

    return result;
}

Image& Swapchain::GetCurrentImage()
{
    return static_cast<Image&>(ISwapchain::GetCurrentImage());
}

void Swapchain::SwapImages()
{
    VkResult result = vkAcquireNextImageKHR(m_device->GetHandle(), m_handle, UINT64_MAX, m_imageReadySemaphore->GetHandle(), VK_NULL_HANDLE, &m_currentImageIndex);
    Utils::AssertSuccess(result);
}

}  // namespace Vulkan
}  // namespace RHI