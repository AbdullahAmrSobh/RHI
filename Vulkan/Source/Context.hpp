#pragma once

#include <RHI/Context.hpp>
#include <RHI/FrameScheduler.hpp>

#include <RHI/Common/Containers.h>

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    struct IImage;
    struct IBuffer;
    struct IImageView;
    struct IBufferView;
    struct IBindGroupLayout;
    struct IBindGroup;
    struct IPipelineLayout;
    struct IGraphicsPipeline;
    struct IComputePipeline;
    struct ISampler;

    class BindGroupAllocator;

    using DestroyResource = std::function<void()>;

    class IContext final : public Context
    {
    public:
        IContext(Ptr<DebugCallbacks> debugCallbacks);
        ~IContext();

        VkResult Init(const ApplicationInfo& appInfo);

        using Context::DebugLogError;
        using Context::DebugLogInfo;
        using Context::DebugLogWarn;

        // clang-format off
        Ptr<Swapchain>            CreateSwapchain(const SwapchainCreateInfo& createInfo)               override;
        Ptr<ShaderModule>         CreateShaderModule(TL::Span<const uint8_t> shaderBlob)               override;
        Ptr<Fence>                CreateFence()                                                        override;
        Ptr<CommandListAllocator> CreateCommandListAllocator()                                         override;
        Ptr<ResourcePool>         CreateResourcePool(const ResourcePoolCreateInfo& createInfo)         override;
        Handle<BindGroupLayout>   CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)   override;
        void                      DestroyBindGroupLayout(Handle<BindGroupLayout> handle)               override;
        Handle<BindGroup>         CreateBindGroup(Handle<BindGroupLayout> handle)                      override;
        void                      DestroyBindGroup(Handle<BindGroup> handle)                           override;
        void                      UpdateBindGroup(Handle<BindGroup> handle, const BindGroupData& data) override;
        Handle<PipelineLayout>    CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)     override;
        void                      DestroyPipelineLayout(Handle<PipelineLayout> handle)                 override;
        Handle<GraphicsPipeline>  CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void                      DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)             override;
        Handle<ComputePipeline>   CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)   override;
        void                      DestroyComputePipeline(Handle<ComputePipeline> handle)               override;
        Handle<Sampler>           CreateSampler(const SamplerCreateInfo& createInfo)                   override;
        void                      DestroySampler(Handle<Sampler> handle)                               override;
        Result<Handle<Image>>     CreateImage(const ImageCreateInfo& createInfo)                       override;
        void                      DestroyImage(Handle<Image> handle)                                   override;
        Result<Handle<Buffer>>    CreateBuffer(const BufferCreateInfo& createInfo)                     override;
        void                      DestroyBuffer(Handle<Buffer> handle)                                 override;
        Handle<ImageView>         CreateImageView(const ImageViewCreateInfo& createInfo)               override;
        void                      DestroyImageView(Handle<ImageView> handle)                           override;
        Handle<BufferView>        CreateBufferView(const BufferViewCreateInfo& createInfo)             override;
        void                      DestroyBufferView(Handle<BufferView> handle)                         override;
        DeviceMemoryPtr           MapBuffer(Handle<Buffer> handle)                                     override;
        void                      UnmapBuffer(Handle<Buffer> handle)                                   override;
        // clang-format on

        void SetDebugName(VkDebugReportObjectTypeEXT type, uint64_t handle, const char* name);

        void DestroyResources();

        VkSemaphore CreateSemaphore();

        void FreeSemaphore(VkSemaphore semaphore);

        uint32_t GetMemoryTypeIndex(MemoryType memoryType);

        uint32_t GetQueueFamilyIndex(QueueType queueType);

        uint32_t GetCurrentFrameIndex() const;

    private:
        static VkBool32 VKAPI_CALL DebugMessengerCallbacks(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

        VkResult InitInstance(const ApplicationInfo& appInfo, bool* debugExtensionEnabled);
        VkResult InitDevice();
        VkResult InitMemoryAllocator();
        VkResult LoadFunctions(bool debugExtensionEnabled);
        VkResult InitFrameScheduler();
        VkResult InitStagingBuffer();

    public:
        // clang-format off
        VkInstance       m_instance       = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice         m_device         = VK_NULL_HANDLE;
        VmaAllocator     m_allocator      = VK_NULL_HANDLE;
        VkQueue          m_presentQueue   = VK_NULL_HANDLE;

        uint32_t m_graphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t m_computeQueueFamilyIndex  = UINT32_MAX;
        uint32_t m_transferQueueFamilyIndex = UINT32_MAX;

        PFN_vkCmdDebugMarkerBeginEXT          m_vkCmdDebugMarkerBeginEXT          = nullptr;
        PFN_vkCmdDebugMarkerInsertEXT         m_vkCmdDebugMarkerInsertEXT         = nullptr;
        PFN_vkCmdDebugMarkerEndEXT            m_vkCmdDebugMarkerEndEXT            = nullptr;
        PFN_vkCmdBeginConditionalRenderingEXT m_vkCmdBeginConditionalRenderingEXT = nullptr;
        PFN_vkCmdEndConditionalRenderingEXT   m_vkCmdEndConditionalRenderingEXT   = nullptr;
        PFN_vkDebugMarkerSetObjectNameEXT     m_vkDebugMarkerSetObjectNameEXT     = nullptr;

        HandlePool<IImage>            m_imageOwner             = HandlePool<IImage>();
        HandlePool<IBuffer>           m_bufferOwner            = HandlePool<IBuffer>();
        HandlePool<IImageView>        m_imageViewOwner         = HandlePool<IImageView>();
        HandlePool<IBufferView>       m_bufferViewOwner        = HandlePool<IBufferView>();
        HandlePool<IBindGroupLayout>  m_bindGroupLayoutsOwner  = HandlePool<IBindGroupLayout>();
        HandlePool<IBindGroup>        m_bindGroupOwner         = HandlePool<IBindGroup>();
        HandlePool<IPipelineLayout>   m_pipelineLayoutOwner    = HandlePool<IPipelineLayout>();
        HandlePool<IGraphicsPipeline> m_graphicsPipelineOwner  = HandlePool<IGraphicsPipeline>();
        HandlePool<IComputePipeline>  m_computePipelineOwner   = HandlePool<IComputePipeline>();
        HandlePool<ISampler>          m_samplerOwner           = HandlePool<ISampler>();
        // clang-format on

        TL::Vector<DestroyResource> m_deferDeleteQueue;

        Ptr<BindGroupAllocator> m_bindGroupAllocator;
    };

} // namespace RHI::Vulkan