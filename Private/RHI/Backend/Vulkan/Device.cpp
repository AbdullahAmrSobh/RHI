#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/Common.hpp"

// #include "RHI/Backend/Vulkan/Queue.hpp"

#include "RHI/Backend/Vulkan/Buffer.hpp"
#include "RHI/Backend/Vulkan/Texture.hpp"

namespace RHI
{
namespace Vulkan
{

    Device::~Device()
    {
        vmaDestroyAllocator(m_allocator);
        vkDestroyDevice(m_device, nullptr);
    }

    VkResult Device::Init(VkInstance _instance, VkPhysicalDevice _physicalDevice)
    {
        m_instance       = _instance;
        m_physicalDevice = PhysicalDevice(_instance, _physicalDevice);

        // Create the device
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            float                   queuePriority   = 1.0f;

            uint32_t index = 0;
            for (const auto& qfb : m_physicalDevice.GetQueueFamilyProperties())
            {

                if (qfb.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {

                    m_queueSettings.presentQueueIndex  = index;
                    m_queueSettings.presentQueueCount  = 1;
                    m_queueSettings.graphicsQueueIndex = index;
                    m_queueSettings.graphicsQueueCount = 1;
                    m_queueSettings.computeQueueIndex  = index;
                    m_queueSettings.computeQueueCount  = 1;
                    m_queueSettings.transferQueueIndex = index;
                    m_queueSettings.transferQueueCount = 1;

                    queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queueCreateInfo.pNext            = nullptr;
                    queueCreateInfo.flags            = 0;
                    queueCreateInfo.queueFamilyIndex = index;
                    queueCreateInfo.queueCount       = 1;
                    queueCreateInfo.pQueuePriorities = &queuePriority;
                    break;
                }

                index++;
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

        // Create queues;
        // m_PresentQueue  = CreateUnique<PresentQueue>(*this, m_queueSettings.presentQueueIndex);
        // m_GraphicsQueue = CreateUnique<Queue>(*this, m_queueSettings.graphicsQueueIndex);
        // m_ComputeQueue  = CreateUnique<Queue>(*this, m_queueSettings.computeQueueIndex);
        // m_TransferQueue = CreateUnique<Queue>(*this, m_queueSettings.transferQueueIndex);
    }

    DeviceAddress Device::MapResourceMemory(const MapableResource& resource, size_t offset, size_t range)
    {
        VmaAllocation allocation = VK_NULL_HANDLE;
        if (resource.type == MapableResource::Type::Buffer)
            allocation = static_cast<Buffer*>(resource.pBuffer)->m_allocation;
        else
            allocation = static_cast<Texture*>(resource.pTexture)->m_allocation;

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
            allocation = static_cast<Texture*>(resource.pTexture)->m_allocation;

        vmaUnmapMemory(m_allocator, allocation);
    }

} // namespace Vulkan
} // namespace RHI
