#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"

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

        std::vector<VkSurfaceFormatKHR> GetSupportedFormats();
        std::vector<VkPresentModeKHR>   GetSupportedPresentModes();
        VkSurfaceCapabilitiesKHR        GetSupportedCapabilities();
        
        static VkSurfaceFormatKHR SelectFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static VkExtent2D         ClampExtent(VkExtent2D actualExtent, VkExtent2D currentExtent, VkExtent2D minImageExtent, VkExtent2D maxImageExtent);
        static VkPresentModeKHR   SelectPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
    
    private:
        VkInstance m_instanceHandle;
    };

} // namespace Vulkan
} // namespace RHI
