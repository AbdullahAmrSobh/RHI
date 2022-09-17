#include <vector>

#include "RHI/Common.hpp"
#include "RHI/Device.hpp"
#include "RHI/ShaderResourceGroup.hpp"
#include "RHI/Swapchain.hpp"

#include "Backend/Vulkan/Instance.hpp"
#include "Backend/Vulkan/PipelineState.hpp"
#include "Backend/Vulkan/Swapchain.hpp"
#include "Backend/Vulkan/Vma/vk_mem_alloc.hpp"

namespace RHI
{
namespace Vulkan
{
    namespace Internal
    {
        class PipelineLayout;

        class RenderPass final : public DeviceObject<VkRenderPass>
        {
        public:
        };

        class Framebuffer final : public DeviceObject<VkFramebuffer>
        {
        public:
            struct Desc
            {
                RenderPass* pRenderPass;
                uint32_t    attachmentCount;
                ImageView*  pAttachments;
                Extent2D    extent;
            };

            Framebuffer(Device& pDevice);
            ~Framebuffer()
            {
                vkDestroyFramebuffer(m_pDevice->GetHandle(), m_handle, nullptr);
            }
            
            VkResult Init(const Desc& desc)
            {

                std::vector<VkImageView> attachmentsHandles{};
                for (uint32_t i = 0; i < desc.attachmentCount; i++)
                {
                    attachmentsHandles.push_back(desc.pAttachments[i].GetHandle());
                }

                VkFramebufferCreateInfo createInfo{};
                createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                createInfo.pNext           = nullptr;
                createInfo.flags           = 0;
                createInfo.renderPass      = desc.pRenderPass->GetHandle();
                createInfo.attachmentCount = desc.attachmentCount;
                createInfo.pAttachments    = attachmentsHandles.data();
                createInfo.width           = desc.extent.sizeX;
                createInfo.height          = desc.extent.sizeY;
                createInfo.layers          = 0;
                
                return vkCreateFramebuffer(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
            }
        };
        
        class ObjectCache
        {
        public:
            Shared<RenderPass>  GetRenderPass(RenderTargetLayout renderTargetLayout);
            Unique<Framebuffer> GetFramebuffer(Framebuffer::Desc framebufferDesc);
        };
    } // namespace Internal
    
    class RenderPassCache;
    class Instance;
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

        inline Queue(VkQueue queue)
            : m_queue(queue)
        {
        }
        uint32_t GetFamilyIndex() const
        {
            return m_familyIndex;
        }

        uint32_t GetQueueIndex() const
        {
            return m_queueIndex;
        }

        bool SupportPresent() const;

        VkResult Submit(const std::vector<PresentRequest>& presentRequsts);

        VkResult Present(const std::vector<SubmitRequest>& submitRequests, Fence* pSignalFence = nullptr);

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

    private:
        class Instance*    m_pInstance;
        VkDevice           m_device;
        VmaAllocator       m_allocator;
        PhysicalDevice*    m_pPhysicalDevice;
        Unique<Queue>      m_graphicsQueue;
        std::vector<Queue> m_queues;
        Queue*             m_pGraphicsQueue;
        Queue*             m_pComputeQueue;
        Queue*             m_pTransferQueue;

        Unique<Internal::ObjectCache> m_objectCache;
    };
} // namespace Vulkan
} // namespace RHI