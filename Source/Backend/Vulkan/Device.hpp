#pragma once
#include "RHI/Device.hpp"
#include "Backend/Vulkan/Instance.hpp"

namespace RHI
{
namespace Vulkan
{
    class RenderPassManager;
    class Semaphore;
    class Fence;
    class PipelineLayout;

    class PhysicalDevice final : public IPhysicalDevice
    {
    public:
        inline PhysicalDevice(VkPhysicalDevice physicalDevice)
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
            Semaphore*  pRenderFinishedSemaphores;
            ISwapchain* pSwapchain;
        };

        struct SubmitRequest
        {
            std::vector<VkSemaphore>     waitSemaphores;
            std::vector<VkCommandBuffer> commandBuffers;
            std::vector<VkSemaphore>     signalSemaphores;
            VkSubmitInfo2                submitInfo;
        };

        inline Queue(VkQueue queue, uint32_t familyIndex, uint32_t index)
            : m_queue(queue)
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

        VkResult Submit(const std::vector<SubmitRequest>& submitRequests, Fence* pSignalFence = nullptr);
        VkResult Present(const PresentRequest& presentRequest);

    private:
        VkQueue  m_queue;
        uint32_t m_familyIndex;
        uint32_t m_queueIndex;
    };

    class Device final : public IDevice
    {
    public:
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

        inline Queue& GetGraphicsQueue()
        {
            return *m_pGraphicsQueue;
        }

        inline Queue& GetComputeQueue()
        {
            return *m_pComputeQueue;
        }

        inline Queue& GetTransferQueue()
        {
            return *m_pTransferQueue;
        }

        inline RenderPassManager& GetRenderPassManager() const
        {
            return *m_renderPassManager;
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
        PipelineLayout FindPipelineLayout(size_t hash);

        EResultCode InitQueues(std::optional<uint32_t> graphicsQueueIndex, std::optional<uint32_t> computeQueueIndex, std::optional<uint32_t> indexQueueIndex);

    private:
        Instance*       m_pInstance;
        PhysicalDevice* m_pPhysicalDevice;

        VkDevice     m_device;
        VmaAllocator m_allocator;

        std::vector<Queue> m_queues;

        // Pointer to dedicated graphics queue.
        Queue* m_pGraphicsQueue;
        // pointer to dedicated compute queue.
        Queue* m_pComputeQueue;
        // pointer to dedicated transfer queue.
        Queue* m_pTransferQueue;

        // Cache for render passes.
        Unique<RenderPassManager> m_renderPassManager;
    };
} // namespace Vulkan
} // namespace RHI