#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/PipelineState.hpp"
#include "Backend/Vulkan/Resource.hpp"
#include "Backend/Vulkan/Swapchain.hpp"

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
        
        std::optional<uint32_t> dedicatedGraphicsQueueFamilyIndex = {};
        std::optional<uint32_t> dedicatedComputeQueueFamilyIndex  = {};
        std::optional<uint32_t> dedicatedTransferQueueFamilyIndex = {};

        float priority                = 1.0f;
        auto  queueFamiliesProperties = physicalDevice.GetQueueFamilyProperties();
        for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamiliesProperties.size(); queueFamilyIndex++)
        {
            VkQueueFamilyProperties queueFamilyProperty = queueFamiliesProperties[queueFamilyIndex];
            
            if (!dedicatedGraphicsQueueFamilyIndex.has_value() && queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                VkDeviceQueueCreateInfo queueCreateInfo;
                queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.pNext            = nullptr;
                queueCreateInfo.flags            = 0;
                queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
                queueCreateInfo.queueCount       = 1;
                queueCreateInfo.pQueuePriorities = &priority;
                queueCreateInfos.push_back(queueCreateInfo);

                dedicatedGraphicsQueueFamilyIndex = queueFamilyIndex;
            }

            if (!dedicatedGraphicsQueueFamilyIndex.has_value() && queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                VkDeviceQueueCreateInfo queueCreateInfo;
                queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.pNext            = nullptr;
                queueCreateInfo.flags            = 0;
                queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
                queueCreateInfo.queueCount       = 1;
                queueCreateInfo.pQueuePriorities = &priority;
                queueCreateInfos.push_back(queueCreateInfo);

                dedicatedGraphicsQueueFamilyIndex = queueFamilyIndex;
            }

            if (!dedicatedGraphicsQueueFamilyIndex.has_value() && queueFamilyProperty.queueFlags == VK_QUEUE_TRANSFER_BIT)
            {
                VkDeviceQueueCreateInfo queueCreateInfo;
                queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.pNext            = nullptr;
                queueCreateInfo.flags            = 0;
                queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
                queueCreateInfo.queueCount       = 1;
                queueCreateInfo.pQueuePriorities = &priority;
                queueCreateInfos.push_back(queueCreateInfo);

                dedicatedGraphicsQueueFamilyIndex = queueFamilyIndex;
            }
        }
        
        std::vector<const char*> enabledLayers;
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
            
            RHI_RETURN_ON_FAIL(vkCreateDevice(physicalDevice.GetHandle(), &createInfo, nullptr, &m_device))
        }

        // Create Queue
        {
            VkQueue queueHandle = VK_NULL_HANDLE;
            
            if (dedicatedGraphicsQueueFamilyIndex.has_value())
            {
                m_queues.push_back(Queue(queueHandle, dedicatedGraphicsQueueFamilyIndex.value(), 0));
                m_pGraphicsQueue = &m_queues.back();

                // Use the graphics queue as a fall back if there is no dedicated compute or transfer queues.
                m_pComputeQueue  = m_pGraphicsQueue;
                m_pTransferQueue = m_pGraphicsQueue;
            }
            else
            {
                return VK_ERROR_UNKNOWN;
            }

            if (dedicatedComputeQueueFamilyIndex.has_value())
            {
                m_queues.push_back(Queue(queueHandle, dedicatedComputeQueueFamilyIndex.value(), 0));
                m_pComputeQueue = &m_queues.back();
            }

            if (dedicatedTransferQueueFamilyIndex.has_value())
            {
                m_queues.push_back(Queue(queueHandle, dedicatedTransferQueueFamilyIndex.value(), 0));
                m_pTransferQueue = &m_queues.back();
            }
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

    VkResult Queue::Present(const PresentRequest& presentRequest) const 
    {
        std::vector<VkSemaphore>    waitSemaphoresHandles;
        std::vector<VkSwapchainKHR> swapchainsHandles;
        std::vector<uint32_t>       imageIndices;
        std::vector<VkResult>       results;

        for (auto swapchain : presentRequest.swapchains)
        {
            imageIndices.push_back(swapchain->GetCurrentBackBufferIndex());
            swapchainsHandles.push_back(swapchain->GetHandle());
            waitSemaphoresHandles.push_back(swapchain->GetBackbufferReadSemaphore().GetHandle());
            results.push_back(VkResult());
        }

        results.resize(presentRequest.swapchains.size());
        imageIndices.reserve(presentRequest.swapchains.size());

        VkPresentInfoKHR presentInfo;
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext              = nullptr;
        presentInfo.waitSemaphoreCount = CountElements(waitSemaphoresHandles);
        presentInfo.pWaitSemaphores    = waitSemaphoresHandles.data();
        presentInfo.swapchainCount     = CountElements(swapchainsHandles);
        presentInfo.pSwapchains        = swapchainsHandles.data();
        presentInfo.pImageIndices      = imageIndices.data();
        presentInfo.pResults           = results.data();

        return vkQueuePresentKHR(m_handle, &presentInfo);
    }
    
    VkResult Queue::Submit(const std::vector<Queue::SubmitRequest>& submitRequests, const Fence* pFence) const 
    {
        std::vector<VkSubmitInfo2> submitInfos;
        submitInfos.reserve(submitRequests.size());

        for (const auto& submitRequest : submitRequests)
        {
            VkSubmitInfo2 submitInfo;
            submitInfo.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
            submitInfo.pNext                    = nullptr;
            submitInfo.flags                    = 0;
            submitInfo.waitSemaphoreInfoCount   = CountElements(submitRequest.waitSemaphores);
            submitInfo.pWaitSemaphoreInfos      = submitRequest.waitSemaphores.data();
            submitInfo.commandBufferInfoCount   = CountElements(submitRequest.commandBuffers);
            submitInfo.pCommandBufferInfos      = submitRequest.commandBuffers.data();
            submitInfo.signalSemaphoreInfoCount = CountElements(submitRequest.signalSemaphores);
            submitInfo.pSignalSemaphoreInfos    = submitRequest.signalSemaphores.data();
            submitInfos.push_back(submitInfo);
        }

        return vkQueueSubmit2(m_handle, CountElements(submitInfos), submitInfos.data(), pFence->GetHandle());
    }

} // namespace Vulkan
} // namespace RHI