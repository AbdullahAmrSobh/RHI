#pragma once

#include <Allocator.hpp>
#include <RHI/Context.hpp>
#include <RHI/FrameGraph.hpp>
#include <vk_mem_alloc.h>

namespace Vulkan
{
    struct Image;
    struct Buffer;
    struct ImageView;
    struct BufferView;
    struct BindGroupLayout;
    struct BindGroup;
    struct PipelineLayout;
    struct GraphicsPipeline;
    struct ComputePipeline;
    struct Sampler;

    class ResourceManager;
    class CommandListAllocator;

    class Context final : public RHI::Context
    {
    public:
        Context() = default;
        ~Context();

        VkResult Init(const RHI::ApplicationInfo& appInfo, std::unique_ptr<RHI::DebugCallbacks> debugCallbacks);

        std::unique_ptr<RHI::Swapchain>          CreateSwapchain(const RHI::SwapchainCreateInfo& createInfo) override;
        std::unique_ptr<RHI::FrameScheduler>     CreateFrameScheduler() override;
        std::unique_ptr<RHI::ShaderModule>       CreateShaderModule(const RHI::ShaderModuleCreateInfo& createInfo) override;
        RHI::Handle<RHI::BindGroupLayout>        CreateBindGroupLayout(const RHI::BindGroupLayoutCreateInfo& createInfo) override;
        void                                     FreeBindGroupLayout(RHI::Handle<RHI::BindGroupLayout> layout) override;
        RHI::Handle<RHI::PipelineLayout>         CreatePipelineLayout(const RHI::PipelineLayoutCreateInfo& createInfo) override;
        void                                     FreePipelineLayout(RHI::Handle<RHI::PipelineLayout> layout) override;
        std::unique_ptr<RHI::BindGroupAllocator> CreateBindGroupAllocator() override;
        std::unique_ptr<RHI::ResourcePool>       CreateResourcePool(const RHI::ResourcePoolCreateInfo& createInfo) override;
        RHI::Handle<RHI::GraphicsPipeline>       CreateGraphicsPipeline(const RHI::GraphicsPipelineCreateInfo& createInfo) override;
        void                                     Free(RHI::Handle<RHI::GraphicsPipeline> pso) override;
        RHI::Handle<RHI::ComputePipeline>        CreateComputePipeline(const RHI::ComputePipelineCreateInfo& createInfo) override;
        void                                     Free(RHI::Handle<RHI::ComputePipeline> pso) override;
        RHI::Handle<RHI::Sampler>                CreateSampler(const RHI::SamplerCreateInfo& createInfo) override;
        void                                     Free(RHI::Handle<RHI::Sampler> sampler) override;
        RHI::Handle<RHI::ImageView>              CreateImageView(RHI::Handle<RHI::Image> handle, const RHI::ImageAttachmentUseInfo& useInfo) override;
        void                                     Free(RHI::Handle<RHI::ImageView> view) override;
        RHI::Handle<RHI::BufferView>             CreateBufferView(RHI::Handle<RHI::Buffer> handle, const RHI::BufferAttachmentUseInfo& useInfo) override;
        void                                     Free(RHI::Handle<RHI::BufferView> view) override;
        RHI::DeviceMemoryPtr                     MapResource(RHI::Handle<RHI::Image> image) override;
        void                                     Unmap(RHI::Handle<RHI::Image> image) override;
        RHI::DeviceMemoryPtr                     MapResource(RHI::Handle<RHI::Buffer> buffer) override;
        void                                     Unmap(RHI::Handle<RHI::Buffer> buffer) override;

        uint32_t GetQueueFamilyIndex(RHI::QueueType queueType) const;

        VkSemaphore CreateSemaphore();
        void        FreeSemaphore(VkSemaphore semaphore);

        VkFence CreateFence();
        void    FreeFence(VkFence fence);

        std::vector<VkLayerProperties> GetAvailableInstanceLayerExtensions() const;

        std::vector<VkExtensionProperties> GetAvailableInstanceExtensions() const;

        std::vector<VkLayerProperties> GetAvailableDeviceLayerExtensions(VkPhysicalDevice physicalDevice) const;

        std::vector<VkExtensionProperties> GetAvailableDeviceExtensions(VkPhysicalDevice physicalDevice) const;

        std::vector<VkPhysicalDevice> GetAvailablePhysicalDevices() const;

        std::vector<VkQueueFamilyProperties> GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice) const;

        VkInstance       m_instance       = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice         m_device         = VK_NULL_HANDLE;
        VmaAllocator     m_allocator      = VK_NULL_HANDLE;

        uint32_t m_graphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t m_computeQueueFamilyIndex  = UINT32_MAX;
        uint32_t m_transferQueueFamilyIndex = UINT32_MAX;

        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_computeQueue  = VK_NULL_HANDLE;
        VkQueue m_transferQueue = VK_NULL_HANDLE;

        PFN_vkCmdDebugMarkerBeginEXT  m_vkCmdDebugMarkerBeginEXT  = nullptr;
        PFN_vkCmdDebugMarkerInsertEXT m_vkCmdDebugMarkerInsertEXT = nullptr;
        PFN_vkCmdDebugMarkerEndEXT    m_vkCmdDebugMarkerEndEXT    = nullptr;

        RHI::HandlePool<Image>            m_imageOwner            = RHI::HandlePool<Image>();
        RHI::HandlePool<Buffer>           m_bufferOwner           = RHI::HandlePool<Buffer>();
        RHI::HandlePool<ImageView>        m_imageViewOwner        = RHI::HandlePool<ImageView>();
        RHI::HandlePool<BufferView>       m_bufferViewOwner       = RHI::HandlePool<BufferView>();
        RHI::HandlePool<BindGroupLayout>  m_bindGroupLayoutsOwner = RHI::HandlePool<BindGroupLayout>();
        RHI::HandlePool<BindGroup>        m_bindGroupOwner        = RHI::HandlePool<BindGroup>();
        RHI::HandlePool<PipelineLayout>   m_pipelineLayoutOwner   = RHI::HandlePool<PipelineLayout>();
        RHI::HandlePool<GraphicsPipeline> m_graphicsPipelineOwner = RHI::HandlePool<GraphicsPipeline>();
        RHI::HandlePool<ComputePipeline>  m_computePipelineOwner  = RHI::HandlePool<ComputePipeline>();
        RHI::HandlePool<Sampler>          m_samplerOwner          = RHI::HandlePool<Sampler>();
    };

    inline uint32_t Context::GetQueueFamilyIndex(RHI::QueueType queueType) const
    {
        switch (queueType)
        {
        case RHI::QueueType::Graphics: return m_graphicsQueueFamilyIndex;
        case RHI::QueueType::Compute:  return m_computeQueueFamilyIndex;
        case RHI::QueueType::Transfer: return m_transferQueueFamilyIndex;
        default:                       RHI_UNREACHABLE(); return UINT32_MAX;
        }
    }

} // namespace Vulkan