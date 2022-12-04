#pragma once
#include "Backend/Vulkan/CommandQueue.hpp"
#include "Backend/Vulkan/Instance.hpp"
#include "RHI/Device.hpp"

namespace RHI
{
namespace Vulkan
{

class PhysicalDevice final : public IPhysicalDevice
{
public:
    PhysicalDevice(VkPhysicalDevice physicalDevice)
        : m_physicalDevice(physicalDevice)
    {
    }

    VkPhysicalDevice GetHandle() const
    {
        return m_physicalDevice;
    }

    VkPhysicalDeviceProperties GetProperties() const;

    VkPhysicalDeviceFeatures GetFeatures() const;

    std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties() const;

    std::vector<VkLayerProperties> GetAvailableLayers() const;

    std::vector<VkExtensionProperties> GetAvailableExtensions() const;

    VkPhysicalDeviceMemoryProperties GetMemoryProperties() const;

    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(
        VkSurfaceKHR _surface) const;

    std::vector<VkPresentModeKHR> GetPresentModes(VkSurfaceKHR _surface) const;

    std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(
        VkSurfaceKHR _surface) const;

private:
    VkPhysicalDevice m_physicalDevice;
};

class Device final : public IDevice
{
public:
    Device(const Instance& instance, const PhysicalDevice& physicaldDevice)
        : m_pInstance(&instance)
        , m_pPhysicalDevice(&physicaldDevice)
    {
    }

    ~Device();

    VkResult Init(const PhysicalDevice& physicalDevice);

    const PhysicalDevice& GetPhysicalDevice() const
    {
        return *m_pPhysicalDevice;
    }

    VkPhysicalDevice GetPhysicalDeviceHandle() const
    {
        return m_pPhysicalDevice->GetHandle();
    }

    VkDevice GetHandle() const
    {
        return m_device;
    }

    VmaAllocator GetAllocator() const
    {
        return m_allocator;
    }

    EResultCode WaitIdle() const override;

    Expected<Unique<ISwapchain>> CreateSwapChain(
        const SwapchainDesc& desc) const override;

    Expected<Unique<IShaderProgram>> CreateShaderProgram(
        const ShaderProgramDesc& desc) const override;

    Expected<Unique<IShaderResourceGroupAllocator>>
    CreateShaderResourceGroupAllocator() const override;

    Expected<Unique<IPipelineState>> CreateGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc) const override;

    Expected<Unique<IFence>> CreateFence() const override;

    Expected<Unique<ISampler>> CreateSampler(
        const SamplerDesc& desc) const override;

    Expected<Unique<IImage>> CreateImage(const AllocationDesc& allocationDesc,
                                         const ImageDesc& desc) const override;

    Expected<Unique<IImageView>> CreateImageView(
        const IImage&        image,
        const ImageViewDesc& desc) const override;

    Expected<Unique<IBuffer>> CreateBuffer(
        const AllocationDesc& allocationDesc,
        const BufferDesc&     desc) const override;

    Expected<Unique<IBufferView>> CreateBufferView(
        const IBuffer&        buffer,
        const BufferViewDesc& desc) const override;

private:
    const Instance* m_pInstance;

    const PhysicalDevice* m_pPhysicalDevice;

    VkDevice m_device;

    VmaAllocator m_allocator;

    Unique<CommandQueue> m_graphicsQueue;

public:
    const CommandQueue& GetGraphicsQueue() const
    {
        return *m_graphicsQueue;
    }

    CommandQueue& GetGraphicsQueue()
    {
        return *m_graphicsQueue;
    }
};
}  // namespace Vulkan
}  // namespace RHI