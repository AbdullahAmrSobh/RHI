#pragma once
#include "RHI/Backend/Vulkan/Common.hpp"
#include "RHI/Backend/Vulkan/vma/vk_mem_alloc.h"

#include <utility>
#include <vector>

namespace RHI
{
namespace Vulkan
{
    class Queue;
    class PresentQueue;

    class PhysicalDevice
    {
    public:
        PhysicalDevice() = default;
        PhysicalDevice(VkInstance _instance, VkPhysicalDevice _physicalDevice)
            // : m_instance(_instance)
            : m_physicalDevice(_physicalDevice)
        {
        }

        inline VkPhysicalDevice GetHandle() { return m_physicalDevice; }

        inline VkPhysicalDeviceProperties GetProperties() const
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
            return properties;
        }

        inline VkPhysicalDeviceFeatures GetFeatures() const
        {
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(m_physicalDevice, &features);
            return features;
        }

        inline std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties() const
        {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
            return queueFamilyProperties;
        }

        inline std::vector<VkLayerProperties> GetAvailableLayers() const
        {
            uint32_t layerCount = 0;
            vkEnumerateDeviceLayerProperties(m_physicalDevice, &layerCount, nullptr);
            std::vector<VkLayerProperties> layerProperties(layerCount);
            vkEnumerateDeviceLayerProperties(m_physicalDevice, &layerCount, layerProperties.data());
            return layerProperties;
        }

        inline std::vector<VkExtensionProperties> GetAvailableExtensions() const
        {
            uint32_t extensionCount = 0;
            vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> extensionProperties(extensionCount);
            vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, extensionProperties.data());
            return extensionProperties;
        }

        inline VkPhysicalDeviceMemoryProperties GetMemoryProperties() const
        {
            VkPhysicalDeviceMemoryProperties properties;
            vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &properties);
            return properties;
        }

        inline VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(VkSurfaceKHR _surface) const
        {
            VkSurfaceCapabilitiesKHR capabilities;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, _surface, &capabilities);
            return capabilities;
        }

        inline std::vector<VkPresentModeKHR> GetPresentModes(VkSurfaceKHR _surface) const
        {
            uint32_t presentModeCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, _surface, &presentModeCount, nullptr);
            std::vector<VkPresentModeKHR> presentModes(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, _surface, &presentModeCount, presentModes.data());
            return presentModes;
        }

        inline std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(VkSurfaceKHR _surface) const
        {
            uint32_t surfaceFormatCount = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, _surface, &surfaceFormatCount, nullptr);
            std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, _surface, &surfaceFormatCount, surfaceFormats.data());
            return surfaceFormats;
        }

    private:
        VkPhysicalDevice m_physicalDevice;
    };

    class Device
    {
    public:
        Device() = default;
        ~Device();

        VkResult Init(VkInstance _instance, VkPhysicalDevice _physicalDevice);

        inline VkDevice       GetHandle() const { return m_device; }
        inline VmaAllocator   GetAllocator() const { return m_allocator; }
        inline PhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
        inline PresentQueue&  GetPresentQueue() const { return *m_PresentQueue; }
        inline Queue&         GetGraphicsQueue() const { return *m_GraphicsQueue; }
        inline Queue&         GetComputeQueue() const { return *m_ComputeQueue; }
        inline Queue&         GetTransferQueue() const { return *m_TransferQueue; }

    private:
        VkInstance     m_instance;
        VkDevice       m_device;
        PhysicalDevice m_physicalDevice;
        VmaAllocator   m_allocator;

        struct QueueSettings
        {
            uint32_t presentQueueIndex, presentQueueCount;
            uint32_t graphicsQueueIndex, graphicsQueueCount;
            uint32_t computeQueueIndex, computeQueueCount;
            uint32_t transferQueueIndex, transferQueueCount;
        } m_queueSettings;

        Unique<PresentQueue> m_PresentQueue;
        Unique<Queue>        m_GraphicsQueue;
        Unique<Queue>        m_ComputeQueue;
        Unique<Queue>        m_TransferQueue;
    };

    template <typename T>
    class DeviceObject
    {
    public:
        inline DeviceObject(Device& _device, T _handle = VK_NULL_HANDLE)
            : m_pDevice(&_device)
            , m_handle(_handle)
        {
        }
        virtual ~DeviceObject() = default;

        void Init(Device& _pDevice) { m_pDevice = &_pDevice; }

        inline T GetHandle() { return m_handle; }
        inline T GetHandle() const { return m_handle; }

    protected:
        Device* m_pDevice;
        T       m_handle;
    };

} // namespace Vulkan
} // namespace RHI
