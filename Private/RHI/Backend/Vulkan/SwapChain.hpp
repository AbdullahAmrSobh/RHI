#pragma once
#include "RHI/SwapChain.hpp"

#define VK_USE_PLATFORM_WIN32_KHR
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/Semaphore.hpp"
#include "RHI/Backend/Vulkan/Texture.hpp"

#include <array>

namespace RHI
{
namespace Vulkan
{

    class Surface final : public DeviceObject<VkSurfaceKHR>
    {
    private:
        friend class Factory;

        static void InitCacheManager(VkInstance instance, Device* pDevice)
        {
            s_pDevice  = pDevice;
            s_instance = instance;
        }

    public:
        Surface(Device& _device)
            : DeviceObject(_device)
        {
        }
        ~Surface();

        VkResult Init(VkInstance instance, NativeWindowHandle nativeWindowHandle);

        inline std::vector<VkSurfaceFormatKHR> GetSupportedFormats()
        {
            uint32_t count;
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, nullptr);
            std::vector<VkSurfaceFormatKHR> formats(count);
            VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, formats.data());
            assert(result == VK_SUCCESS);
            return formats;
        }

        inline std::vector<VkPresentModeKHR> GetSupportedPresentModes()
        {
            uint32_t count = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, nullptr);
            std::vector<VkPresentModeKHR> modes(count);
            VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, modes.data());
            assert(result == VK_SUCCESS);
            return modes;
        }

        inline VkSurfaceCapabilitiesKHR GetSupportedCapabilities()
        {
            VkSurfaceCapabilitiesKHR capabilities;
            VkResult                 result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &capabilities);
            assert(result == VK_SUCCESS);
            return capabilities;
        }

        static inline VkSurfaceFormatKHR SelectFormat(const std::vector<VkSurfaceFormatKHR>& formats)
        {
            for (const auto& availableFormat : formats)
            {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return availableFormat;
            }
            return formats[0];
        }

        static inline VkExtent2D ClampExtent(VkExtent2D actualExtent, VkExtent2D currentExtent, VkExtent2D minImageExtent, VkExtent2D maxImageExtent)
        {
            if (actualExtent.width != UINT32_MAX)
                return currentExtent;
            else
                return {std::clamp(actualExtent.width, minImageExtent.width, maxImageExtent.width),
                        std::clamp(actualExtent.height, minImageExtent.height, maxImageExtent.height)};
        }

        static inline VkPresentModeKHR SelectPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
        {
            return std::find_if(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) == presentModes.end() ? VK_PRESENT_MODE_MAILBOX_KHR
                                                                                                                             : VK_PRESENT_MODE_FIFO_KHR;
        }

        static inline Surface& FindSurfaceOrCreate(NativeWindowHandle windowHandle)
        {
            auto surface = m_surfaces.find(windowHandle);

            if (m_surfaces.end() != surface)
                return *surface->second.get();

            m_surfaces[windowHandle] = CreateUnique<Surface>(*Surface::s_pDevice);
            m_surfaces[windowHandle]->Init(Surface::s_instance, windowHandle);
            return *m_surfaces[windowHandle].get();
        }

    private:
        static VkInstance                                              s_instance;
        static Device*                                                 s_pDevice;
        static std::unordered_map<NativeWindowHandle, Unique<Surface>> m_surfaces;
    };
    
    class SwapChain final
        : public ISwapChain
        , public DeviceObject<VkSwapchainKHR>
    {
    private:
        friend class Queue;

    public:
        SwapChain(Device& _device)
            : DeviceObject(_device)
        {
        }
        ~SwapChain();

        VkResult Init(const SwapChainDesc& desc);

        virtual EResultCode SwapBuffers() override;

    private:
        void ObtainBackBuffers();

    private:
        Queue* m_pPresentQueue;
    };

} // namespace Vulkan
} // namespace RHI
