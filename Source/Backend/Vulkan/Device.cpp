#include <vector>

#include "RHI/Common.hpp"
#include "RHI/Device.hpp"

#include <vulkan/vulkan.h>

#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/Instance.hpp"
#include "Backend/Vulkan/PipelineState.hpp"
#include "Backend/Vulkan/Resource.hpp"
#include "Backend/Vulkan/Swapchain.hpp"
#include <vulkan/vulkan_core.h>

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
        }

            std::vector<const char*> enabledLayers     = {"VK_LAYER_LUNARG_standard_validation"};
            std::vector<const char*> enabledExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkPhysicalDeviceFeatures features = {};

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
        return result;
    }

    void Device::WaitIdle() const
    {
    }

    Expected<Unique<ISwapchain>> Device::CreateSwapChain(const SwapchainDesc& desc)
    {
        return Unexpected(EResultCode::Fail);
    }

    Expected<Unique<IShaderProgram>> Device::CreateShaderProgram(const ShaderProgramDesc& desc)
    {
        return Unexpected(EResultCode::Fail);
    }

    Expected<Unique<IPipelineState>> Device::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
    {
        return Unexpected(EResultCode::Fail);
    }

    Expected<Unique<IFence>> Device::CreateFence()
    {
        return Unexpected(EResultCode::Fail);
    }

    Expected<Unique<ISampler>> Device::CreateSampler(const SamplerDesc& desc)
    {
        return Unexpected(EResultCode::Fail);
    }

    Expected<Unique<IImage>> Device::CreateImage(const AllocationDesc& allocationDesc, const ImageDesc& desc)
    {
        return Unexpected(EResultCode::Fail);
    }

    Expected<Unique<IImageView>> Device::CreateImageView(const IImage& image, const ImageViewDesc& desc)
    {
        return Unexpected(EResultCode::Fail);
    }

    Expected<Unique<IBuffer>> Device::CreateBuffer(const AllocationDesc& allocationDesc, const BufferDesc& desc)
    {
        return Unexpected(EResultCode::Fail);
    }

    Expected<Unique<IBufferView>> Device::CreateBufferView(const IBuffer& buffer, const BufferViewDesc& desc)
    {
        return Unexpected(EResultCode::Fail);
    }

    Expected<Unique<FrameGraph>> Device::CreateFrameGraph()
    {
        return Unexpected(EResultCode::Fail);
    }


} // namespace Vulkan
} // namespace RHI