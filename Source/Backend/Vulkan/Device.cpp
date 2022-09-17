#include <vector>

#include "RHI/Common.hpp"
#include "RHI/Device.hpp"

#include "Backend/Vulkan/Instance.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/PipelineState.hpp"
#include "Backend/Vulkan/Resource.hpp"
#include "Backend/Vulkan/Swapchain.hpp"

#include "Backend/Vulkan/Common.hpp"

namespace RHI
{
namespace Vulkan
{

    VkPhysicalDeviceProperties PhysicalDevice::GetProperties() const
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
        return properties;
    }

    VkPhysicalDeviceFeatures PhysicalDevice::GetFeatures() const
    {
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(m_physicalDevice, &features);
        return features;
    }

    std::vector<VkQueueFamilyProperties> PhysicalDevice::GetQueueFamilyProperties() const
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
        return queueFamilyProperties;
    }

    std::vector<VkLayerProperties> PhysicalDevice::GetAvailableLayers() const
    {
        uint32_t layerCount = 0;
        vkEnumerateDeviceLayerProperties(m_physicalDevice, &layerCount, nullptr);
        std::vector<VkLayerProperties> layerProperties(layerCount);
        vkEnumerateDeviceLayerProperties(m_physicalDevice, &layerCount, layerProperties.data());
        return layerProperties;
    }

    std::vector<VkExtensionProperties> PhysicalDevice::GetAvailableExtensions() const
    {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensionProperties(extensionCount);
        vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, extensionProperties.data());
        return extensionProperties;
    }
    
    VkPhysicalDeviceMemoryProperties PhysicalDevice::GetMemoryProperties() const
    {
        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &properties);
        return properties;
    }

    VkSurfaceCapabilitiesKHR PhysicalDevice::GetSurfaceCapabilities(VkSurfaceKHR surface) const
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, surface, &capabilities);
        return capabilities;
    }
    
    std::vector<VkPresentModeKHR> PhysicalDevice::GetPresentModes(VkSurfaceKHR surface) const
    {
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, surface, &presentModeCount, presentModes.data());
        return presentModes;
    }

    std::vector<VkSurfaceFormatKHR> PhysicalDevice::GetSurfaceFormats(VkSurfaceKHR surface) const
    {
        uint32_t surfaceFormatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface, &surfaceFormatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data());
        return surfaceFormats;
    }

    Device::~Device()
    {
        WaitIdle();
        vkDestroyDevice(m_device, nullptr);
    }
    
    VkResult Device::Init(Instance& instance, const PhysicalDevice& physicalDevice)
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        float priority                = 1.0f;
        auto  queueFamiliesProperties = physicalDevice.GetQueueFamilyProperties();
        for (uint32_t queueFamilyIndex = 0; queueFamilyIndex <= queueFamiliesProperties.size(); queueFamilyIndex++)
        {
            auto queueFamilyProperty = queueFamiliesProperties[queueFamilyIndex];

            if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                queueCreateInfos.emplace_back();
                VkDeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos.back();
                queueCreateInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.pNext                    = nullptr;
                queueCreateInfo.flags                    = 0;
                queueCreateInfo.queueFamilyIndex         = queueFamilyIndex;
                queueCreateInfo.queueCount               = 1;
                queueCreateInfo.pQueuePriorities         = &priority;
                break;
            }

            // Check if dedicated compute queue
            if (queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT) {}

            // Find dedicated transfer queue if exist
            if (queueFamilyProperty.queueFlags == VK_QUEUE_TRANSFER_BIT) {}
        }

        std::vector<const char*> enabledLayers     = {"VK_LAYER_LUNARG_standard_validation"};
        std::vector<const char*> enabledExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        
        VkPhysicalDeviceFeatures features = {};
        {
            VkDeviceCreateInfo createInfo      = {};
            createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.pNext                   = nullptr;
            createInfo.flags                   = 0;
            createInfo.queueCreateInfoCount    = CountElements(queueCreateInfos);
            createInfo.pQueueCreateInfos       = queueCreateInfos.data();
            createInfo.enabledLayerCount       = CountElements(enabledLayers);
            createInfo.ppEnabledLayerNames     = enabledLayers.data();
            createInfo.enabledExtensionCount   = CountElements(enabledExtensions);
            createInfo.ppEnabledExtensionNames = enabledExtensions.data();
            createInfo.pEnabledFeatures        = &features;

            VkResult result = vkCreateDevice(physicalDevice.GetHandle(), &createInfo, nullptr, &m_device);
            if (!RHI_SUCCESS(result))
                return result;
        }
        
        VmaAllocatorCreateInfo createInfo = {};
        createInfo.flags                  = 0;
        createInfo.physicalDevice         = m_pPhysicalDevice->GetHandle();
        createInfo.device                 = m_device;
        createInfo.instance               = m_pInstance->GetHandle();
            // createInfo.vulkanApiVersion = VK_VERSION_1_2;
        return vmaCreateAllocator(&createInfo, &m_allocator);
    }
    
    EResultCode Device::WaitIdle() const
    {
        return ConvertResult(vkDeviceWaitIdle(m_device));
    }
    
    Expected<Unique<IFrameGraph>> Device::CreateFrameGraph()
    {
        return Unexpected(EResultCode::Fail);
    }

} // namespace Vulkan
} // namespace RHI