#pragma once
#include "RHI/Device.hpp"

#include "RHI/Backend/Vulkan/Common.hpp"
#include "RHI/Backend/Vulkan/vma/vk_mem_alloc.h"

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

        inline VkPhysicalDevice GetHandle()
        {
            return m_physicalDevice;
        }

        VkPhysicalDeviceProperties GetProperties() const;

        VkPhysicalDeviceFeatures GetFeatures() const;

        std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties() const;

        std::vector<VkLayerProperties> GetAvailableLayers() const;

        std::vector<VkExtensionProperties> GetAvailableExtensions() const;

        VkPhysicalDeviceMemoryProperties GetMemoryProperties() const;

        VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(VkSurfaceKHR _surface) const;

        std::vector<VkPresentModeKHR> GetPresentModes(VkSurfaceKHR _surface) const;

        std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(VkSurfaceKHR _surface) const;

    private:
        VkPhysicalDevice m_physicalDevice;
    };

    class Device final : public RHI::IDevice
    {
    public:
        using Base = RHI::IDevice;

        Device() = default;
        ~Device();

        VkResult Init(VkInstance _instance, VkPhysicalDevice _physicalDevice);

        inline VkDevice GetHandle() const
        {
            return m_device;
        }

        inline VmaAllocator GetAllocator() const
        {
            return m_allocator;
        }

        inline PhysicalDevice GetPhysicalDevice() const
        {
            return m_physicalDevice;
        }

        inline Queue& GetGraphicsQueue()
        {
            return *m_pGraphicsQueue;
        }

        virtual DeviceAddress MapResourceMemory(const MapableResource& resource, size_t offset, size_t range) override;
        virtual void          UnmapResourceMemory(const MapableResource& resource) override;

    private:
        VkInstance     m_instance;
        VkDevice       m_device;
        PhysicalDevice m_physicalDevice;
        VmaAllocator   m_allocator;

        Queue* m_pGraphicsQueue = nullptr;
        // Queue* m_pPresentQueue  = nullptr;
        // Queue* m_pComputeQueue  = nullptr;
        // Queue* m_pTransferQueue = nullptr;
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

        void Init(Device& _pDevice)
        {
            m_pDevice = &_pDevice;
        }

        inline T GetHandle()
        {
            return m_handle;
        }

        inline T GetHandle() const
        {
            return m_handle;
        }

    protected:
        Device* m_pDevice;
        T       m_handle;
    };

    template <>
    class DeviceObject<void>
    {
    public:
        inline DeviceObject(Device& _device)
            : m_pDevice(&_device)
        {
        }
        virtual ~DeviceObject() = default;

        void Init(Device& _pDevice)
        {
            m_pDevice = &_pDevice;
        }

    protected:
        Device* m_pDevice;
    };

} // namespace Vulkan
} // namespace RHI
