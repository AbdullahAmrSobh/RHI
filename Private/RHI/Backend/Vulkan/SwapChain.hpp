#pragma once
#include "RHI/SwapChain.hpp"

#define VK_USE_PLATFORM_WIN32_KHR
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Resources.hpp"

#include "RHI/Backend/Vulkan/Texture.hpp"

#include <array>

namespace RHI
{
namespace Vulkan
{
    class Surface final : public DeviceObject<VkSurfaceKHR>
    {
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
        
		inline std::vector<VkPresentModeKHR>   GetSupportedPresentModes()
		{
			uint32_t count = 0;
			vkGetPhysicalDeviceSurfacePresentModesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, nullptr);
			std::vector<VkPresentModeKHR> modes(count);
			VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &count, modes.data());
			assert(result == VK_SUCCESS);
			return modes;
		}
        
        inline VkSurfaceCapabilitiesKHR        GetSupportedCapabilities()
		{
			VkSurfaceCapabilitiesKHR capabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), m_handle, &capabilities);
			return capabilities;
		}
    };

    class SwapChain final
        : public ISwapChain
        , public DeviceObject<VkSwapchainKHR>
    {
    public:
        SwapChain(Device& _device)
            : DeviceObject(_device)
        {
        }
        ~SwapChain();

        VkResult Init(const SwapChainDesc& desc);

        virtual EResultCode SwapBuffers() override;
        virtual EResultCode Present() override;
    };

} // namespace Vulkan
} // namespace RHI
