#pragma once
#include "RHI/Device.hpp"
#include "RHI/Buffer.hpp"
#include "RHI/ObjectCache.hpp"

#include "Backend/Vulkan/CommandQueue.hpp"
#include "Backend/Vulkan/Framebuffer.hpp"

namespace RHI
{
class UsedImageAttachment;

namespace Vulkan
{

class Instance;

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

    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(VkSurfaceKHR surface) const;

    std::vector<VkPresentModeKHR> GetPresentModes(VkSurfaceKHR surface) const;

    std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(VkSurfaceKHR surface) const;

private:
    VkPhysicalDevice m_physicalDevice;
};

class Device final : public IDevice
{
public:
    Device(const Instance& instance, const PhysicalDevice& physicaldDevice)
        : IDevice(physicaldDevice)
        , m_pInstance(&instance)
    {
    }

    ~Device();

    VkResult Init(const PhysicalDevice& physicalDevice);

    const PhysicalDevice& GetPhysicalDevice() const
    {
        return static_cast<const PhysicalDevice&>(IDevice::GetPhysicalDevice());
    }

    VkDevice GetHandle() const
    {
        return m_handle;
    }

    VmaAllocator GetAllocator() const
    {
        return m_allocator;
    }

    ResultCode WaitIdle() const override;

    Expected<Unique<ISwapchain>> CreateSwapChain(const SwapchainDesc& desc) override;

    Expected<Unique<IShaderProgram>> CreateShaderProgram(const ShaderProgramDesc& desc) override;

    Expected<Unique<IShaderResourceGroupAllocator>> CreateShaderResourceGroupAllocator() override;

    Expected<Unique<IPipelineState>> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) override;

    Expected<Unique<IFence>> CreateFence() override;

    Expected<Unique<ISampler>> CreateSampler(const SamplerDesc& desc) override;

    Expected<Unique<IImage>> CreateImage(const AllocationDesc& allocationDesc, const ImageDesc& desc) override;

    Expected<Unique<IImageView>> CreateImageView(const IImage& image, const ImageViewDesc& desc) override;

    Expected<Unique<IBuffer>> CreateBuffer(const AllocationDesc& allocationDesc, const BufferDesc& desc) override;

    Expected<Unique<IBufferView>> CreateBufferView(const IBuffer& buffer, const BufferViewDesc& desc) override;

    Expected<Unique<IFrameScheduler>> CreateFrameScheduler() override;

    //////////////////////////////////////
    // Cached objects creator

    Shared<RenderPassLayout> CreateRenderpassLayout(std::span<const UsedImageAttachment* const> attachments);
    Shared<Framebuffer>      CreateCachedFramebuffer(std::span<UsedImageAttachment* const> attachments);

private:
    const Instance* m_pInstance;

    VkDevice m_handle;

    VmaAllocator m_allocator;

    Unique<CommandQueue> m_graphicsQueue;

    ObjectCache<RenderPassLayout> m_renderpassLayoutCache;
    ObjectCache<Framebuffer> m_framebufferCache;

    Unique<IBuffer> m_stageBuffer;

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