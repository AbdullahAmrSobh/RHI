#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/Common.hpp"
#include "RHI/Backend/Vulkan/Buffer.hpp"
#include "RHI/Backend/Vulkan/Image.hpp"
#include "RHI/Backend/Vulkan/Queue.hpp"

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

    VkSurfaceCapabilitiesKHR PhysicalDevice::GetSurfaceCapabilities(VkSurfaceKHR _surface) const
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, _surface, &capabilities);
        return capabilities;
    }

    std::vector<VkPresentModeKHR> PhysicalDevice::GetPresentModes(VkSurfaceKHR _surface) const
    {
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, _surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, _surface, &presentModeCount, presentModes.data());
        return presentModes;
    }

    std::vector<VkSurfaceFormatKHR> PhysicalDevice::GetSurfaceFormats(VkSurfaceKHR _surface) const
    {
        uint32_t surfaceFormatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, _surface, &surfaceFormatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, _surface, &surfaceFormatCount, surfaceFormats.data());
        return surfaceFormats;
    }

    Device::~Device()
    {
        vmaDestroyAllocator(m_allocator);
        vkDestroyDevice(m_device, nullptr);
    }

    VkResult Device::Init(VkInstance _instance, VkPhysicalDevice _physicalDevice)
    {
        m_instance       = _instance;
        m_physicalDevice = PhysicalDevice(_instance, _physicalDevice);
        

		QueueDesc graphicsQueueDesc; // this queue is assumed to be present capable.
        
        // Create the device
        {
            float                   queuePriority   = 1.0f;
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.pNext                   = nullptr;
            queueCreateInfo.flags                   = 0;
            queueCreateInfo.queueCount              = 1;
            queueCreateInfo.pQueuePriorities        = &queuePriority;

            uint32_t queueFamilyIndex = 0;
            

            for (const auto& qfp : m_physicalDevice.GetQueueFamilyProperties())
            {
                if (qfp.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    graphicsQueueDesc.queueFamilyIndex = queueFamilyIndex;
                    break;
                }
                queueFamilyIndex++;
            }

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};
            
            std::vector<const char*> enabledLayers     = {"VK_LAYER_LUNARG_standard_validation"};
            std::vector<const char*> enabledExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
            VkPhysicalDeviceFeatures enabledFeatures   = {};

            VkDeviceCreateInfo createInfo      = {};
            createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.pNext                   = nullptr;
            createInfo.flags                   = 0;
            createInfo.queueCreateInfoCount    = 1;
            createInfo.pQueueCreateInfos       = &queueCreateInfo;
            createInfo.enabledLayerCount       = enabledLayers.size();
            createInfo.ppEnabledLayerNames     = enabledLayers.data();
            createInfo.enabledExtensionCount   = enabledExtensions.size();
            createInfo.ppEnabledExtensionNames = enabledExtensions.data();
            createInfo.pEnabledFeatures        = &enabledFeatures;

            VkResult result = vkCreateDevice(m_physicalDevice.GetHandle(), &createInfo, nullptr, &m_device);
            if (result != VK_SUCCESS)
                return result;
        }

        // Create the allocator
        {
            VmaAllocatorCreateInfo createInfo = {};
            createInfo.flags                  = 0;
            createInfo.physicalDevice         = m_physicalDevice.GetHandle();
            createInfo.device                 = m_device;
            createInfo.instance               = m_instance;
            // createInfo.vulkanApiVersion = VK_VERSION_1_2;

            VkResult result = vmaCreateAllocator(&createInfo, &m_allocator);
            return result;
        }

		m_pGraphicsQueue = new Queue(*this, graphicsQueueDesc);
		
	}
    
    DeviceAddress Device::MapResourceMemory(const MapableResource& resource, size_t offset, size_t range)
    {
        VmaAllocation allocation = VK_NULL_HANDLE;
        if (resource.type == MapableResource::Type::Buffer)
            allocation = static_cast<Buffer*>(resource.pBuffer)->m_allocation;
        else
            allocation = static_cast<Image*>(resource.pImage)->m_allocation;

        DeviceAddress address;
        VkResult      result = vmaMapMemory(m_allocator, allocation, &address);

        if (result != VK_SUCCESS)
            return nullptr;

        return address;
    }

    void Device::UnmapResourceMemory(const MapableResource& resource)
    {
        VmaAllocation allocation;

        if (resource.type == MapableResource::Type::Buffer)
            allocation = static_cast<Buffer*>(resource.pBuffer)->m_allocation;
        else
            allocation = static_cast<Image*>(resource.pImage)->m_allocation;

        vmaUnmapMemory(m_allocator, allocation);
    }

} // namespace Vulkan
} // namespace RHI
