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

    Expected<std::unique_ptr<ISwapchain>> CreateSwapChain(const SwapchainDesc& desc) override;

    Expected<std::unique_ptr<IShaderProgram>> CreateShaderProgram(const ShaderProgramDesc& desc) override;

    Expected<std::unique_ptr<IShaderResourceGroupAllocator>> CreateShaderResourceGroupAllocator() override;

    Expected<std::unique_ptr<IPipelineState>> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) override;

    Expected<std::unique_ptr<IFence>> CreateFence() override;

    Expected<std::unique_ptr<ISampler>> CreateSampler(const SamplerDesc& desc) override;

    Expected<std::unique_ptr<IImage>> CreateImage(const AllocationDesc& allocationDesc, const ImageDesc& desc) override;

    Expected<std::unique_ptr<IImageView>> CreateImageView(const IImage& image, const ImageViewDesc& desc) override;

    Expected<std::unique_ptr<IBuffer>> CreateBuffer(const AllocationDesc& allocationDesc, const BufferDesc& desc) override;

    Expected<std::unique_ptr<IBufferView>> CreateBufferView(const IBuffer& buffer, const BufferViewDesc& desc) override;

    Expected<std::unique_ptr<IFrameScheduler>> CreateFrameScheduler() override;

    //////////////////////////////////////
    // Cached objects creator

    std::shared_ptr<RenderPassLayout> CreateRenderpassLayout(std::span<const UsedImageAttachment* const> attachments);
    std::shared_ptr<Framebuffer>      CreateCachedFramebuffer(std::span<UsedImageAttachment* const> attachments);

private:
    const Instance* m_pInstance;

    VkDevice m_handle;

    VmaAllocator m_allocator;

    std::unique_ptr<CommandQueue> m_graphicsQueue;

    ObjectCache<RenderPassLayout> m_renderpassLayoutCache;
    ObjectCache<Framebuffer> m_framebufferCache;

    std::unique_ptr<IBuffer> m_stageBuffer;

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