#pragma once
#include <unordered_map>
#include "RHI/Common.hpp"
#include "RHI/Device.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Resource.hpp"
#include "RHI/ShaderResourceGroup.hpp"
#include "Backend/Vulkan/Instance.hpp"

namespace RHI
{
namespace Vulkan
{
    class Semaphore;
    class Fence;
    class PipelineLayout;
    class DescriptorSetLayout;
    class Swapchain;

    class PhysicalDevice final : public IPhysicalDevice
    {
    public:
        PhysicalDevice(VkPhysicalDevice physicalDevice)
            : m_physicalDevice(physicalDevice)
        {
        }

        inline VkPhysicalDevice GetHandle() const
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

    class Queue final
    {
    public:
        struct PresentRequest
        {
            std::vector<Semaphore*> waitSemaphores;
            std::vector<Swapchain*> swapchains;
        };

        struct SubmitRequest
        {
            std::vector<VkSemaphoreSubmitInfo>     waitSemaphores;
            std::vector<VkCommandBufferSubmitInfo> commandBuffers;
            std::vector<VkSemaphoreSubmitInfo>     signalSemaphores;
        };

        inline Queue(VkQueue queue, uint32_t familyIndex, uint32_t index)
            : m_handle(queue)
            , m_familyIndex(index)
            , m_queueIndex(index)
        {
        }

        inline uint32_t GetFamilyIndex() const
        {
            return m_familyIndex;
        }

        inline uint32_t GetQueueIndex() const
        {
            return m_queueIndex;
        }

        bool SupportPresent() const;

        VkResult Present(const PresentRequest& presentRequest) const;
        VkResult Submit(const std::vector<SubmitRequest>& submitRequests, const Fence* fence) const;

    private:
        VkQueue  m_handle;
        uint32_t m_familyIndex;
        uint32_t m_queueIndex;
    };

    using DeviceMemoryPtr = void*;
    class DeviceContext
    {
    public:
        virtual ~DeviceContext() = default;

        virtual Expected<DeviceMemoryPtr> Map(IResource& resource);
        virtual void                      Unmap(IResource& resource);
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

        VkResult Init(Instance& instance, const PhysicalDevice& physicalDevice);

        inline const PhysicalDevice& GetPhysicalDevice() const
        {
            return *m_pPhysicalDevice;
        }

        inline VkPhysicalDevice GetPhysicalDeviceHandle() const
        {
            return m_pPhysicalDevice->GetHandle();
        }

        inline VkDevice GetHandle() const
        {
            return m_device;
        }

        inline VmaAllocator GetAllocator() const
        {
            return m_allocator;
        }

        inline const Queue& GetGraphicsQueue() const
        {
            return *m_pGraphicsQueue;
        }

        inline Queue& GetGraphicsQueue()
        {
            return *m_pGraphicsQueue;
        }

        inline const Queue& GetComputeQueue() const
        {
            return *m_pComputeQueue;
        }

        inline Queue& GetComputeQueue()
        {
            return *m_pComputeQueue;
        }

        inline Queue& GetTransferQueue()
        {
            return *m_pTransferQueue;
        }

        inline const Queue& GetTransferQueue() const
        {
            return *m_pTransferQueue;
        }

        virtual EResultCode                                     WaitIdle() const override;
        virtual Expected<Unique<ISwapchain>>                    CreateSwapChain(const SwapchainDesc& desc) override;
        virtual Expected<Unique<IShaderProgram>>                CreateShaderProgram(const ShaderProgramDesc& desc) override;
        virtual Expected<Unique<IShaderResourceGroupAllocator>> CreateShaderResourceGroupAllocator() override;
        virtual Expected<Unique<IPipelineState>>                CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) override;
        virtual Expected<Unique<IFence>>                        CreateFence() override;
        virtual Expected<Unique<ISampler>>                      CreateSampler(const SamplerDesc& desc) override;
        virtual Expected<Unique<IImage>>                        CreateImage(const AllocationDesc& allocationDesc, const ImageDesc& desc) override;
        virtual Expected<Unique<IImageView>>                    CreateImageView(const IImage& image, const ImageViewDesc& desc) override;
        virtual Expected<Unique<IBuffer>>                       CreateBuffer(const AllocationDesc& allocationDesc, const BufferDesc& desc) override;
        virtual Expected<Unique<IBufferView>>                   CreateBufferView(const IBuffer& buffer, const BufferViewDesc& desc) override;
        virtual Expected<Unique<IFrameGraph>>                   CreateFrameGraph() override;

    private:
        EResultCode InitQueues(std::optional<uint32_t> graphicsQueueIndex, std::optional<uint32_t> computeQueueIndex, std::optional<uint32_t> indexQueueIndex);

    private:
        const Instance* m_pInstance;

        const PhysicalDevice* m_pPhysicalDevice;

        VkDevice m_device;

        VmaAllocator m_allocator;

        std::vector<Queue> m_queues;

        // Pointer to dedicated queues.
        Queue* m_pGraphicsQueue;
        Queue* m_pComputeQueue;
        Queue* m_pTransferQueue;
    };
} // namespace Vulkan
} // namespace RHI