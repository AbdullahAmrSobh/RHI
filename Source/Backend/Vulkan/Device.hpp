#include <vector>
#include "RHI/Common.hpp"
#include "RHI/Device.hpp"
#include "RHI/ShaderResourceBindings.hpp"
#include "RHI/Swapchain.hpp"
#include "Backend/Vulkan/Instance.hpp"
#include "Backend/Vulkan/PipelineState.hpp"
#include "Backend/Vulkan/Swapchain.hpp"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace RHI
{
namespace Vulkan
{
    class Instance;

    class PhysicalDevice final : public IPhysicalDevice
    {
    public:
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

        bool SupportPresen() const;

        VkResult Submit(const std::vector<PresentRequest>& presentRequsts);

        VkResult Present(const std::vector<SubmitRequest>& submitRequests, Fence* pSignalFence = nullptr);

    private:
        VkQueue m_queue;
    };

    class Device final : public IDevice
    {
    public:
        ~Device();

        VkResult Init(Instance& instance, const PhysicalDevice& physicalDevice);

        virtual void WaitIdle() const override;

        virtual Expected<Unique<ISwapchain>> CreateSwapChain(const SwapchainDesc& desc) override;

        virtual Expected<Unique<IShaderProgram>> CreateShaderProgram(const ShaderProgramDesc& desc) override;

        virtual Expected<Unique<IShaderResourceGroupAllocator>> CreateShaderResourceGroupAllocator() override;

        virtual Expected<Unique<IPipelineState>> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) override;

        virtual Expected<Unique<IFence>> CreateFence() override;

        virtual Expected<Unique<ISampler>> CreateSampler(const SamplerDesc& desc) override;

        virtual Expected<Unique<IImage>> CreateImage(const AllocationDesc& allocationDesc, const ImageDesc& desc) override;

        virtual Expected<Unique<IImageView>> CreateImageView(const IImage& image, const ImageViewDesc& desc) override;

        virtual Expected<Unique<IBuffer>> CreateBuffer(const AllocationDesc& allocationDesc, const BufferDesc& desc) override;

        virtual Expected<Unique<IBufferView>> CreateBufferView(const IBuffer& buffer, const BufferViewDesc& desc) override;

        virtual Expected<Unique<FrameGraph>> CreateFrameGraph() override;

    private:
        PipelineLayout FindPipelineLayout(size_t hash);

    private:
        VkDevice m_device;
    };
} // namespace Vulkan
} // namespace RHI