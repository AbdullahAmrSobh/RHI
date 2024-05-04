#pragma once

#include <RHI/Context.hpp>
#include <RHI/FrameScheduler.hpp>
#include <RHI/Common/Containers.h>

#include "Resources.hpp"

#include <vk_mem_alloc.h>

#include <functional>


namespace RHI::Vulkan
{
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
        RHI_NODISCARD Ptr<Swapchain>            CreateSwapchain(const SwapchainCreateInfo& createInfo)               override;
        RHI_NODISCARD Ptr<ShaderModule>         CreateShaderModule(TL::Span<const uint8_t> shaderBlob)               override;
        RHI_NODISCARD Ptr<Fence>                CreateFence()                                                        override;
        RHI_NODISCARD Ptr<CommandPool>          CreateCommandPool()                                                  override;
        RHI_NODISCARD Ptr<ResourcePool>         CreateResourcePool(const ResourcePoolCreateInfo& createInfo)         override;
        RHI_NODISCARD Handle<BindGroupLayout>   CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)   override;
                      void                      DestroyBindGroupLayout(Handle<BindGroupLayout> handle)               override;
        RHI_NODISCARD Handle<BindGroup>         CreateBindGroup(Handle<BindGroupLayout> handle)                      override;
                      void                      DestroyBindGroup(Handle<BindGroup> handle)                           override;
                      void                      UpdateBindGroup(Handle<BindGroup> handle, const BindGroupData& data) override;
        RHI_NODISCARD Handle<PipelineLayout>    CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)     override;
                      void                      DestroyPipelineLayout(Handle<PipelineLayout> handle)                 override;
        RHI_NODISCARD Handle<GraphicsPipeline>  CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
                      void                      DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)             override;
        RHI_NODISCARD Handle<ComputePipeline>   CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)   override;
                      void                      DestroyComputePipeline(Handle<ComputePipeline> handle)               override;
        RHI_NODISCARD Handle<Sampler>           CreateSampler(const SamplerCreateInfo& createInfo)                   override;
                      void                      DestroySampler(Handle<Sampler> handle)                               override;
        RHI_NODISCARD Result<Handle<Image>>     CreateImage(const ImageCreateInfo& createInfo)                       override;
                      void                      DestroyImage(Handle<Image> handle)                                   override;
        RHI_NODISCARD Result<Handle<Buffer>>    CreateBuffer(const BufferCreateInfo& createInfo)                     override;
                      void                      DestroyBuffer(Handle<Buffer> handle)                                 override;
        RHI_NODISCARD Handle<ImageView>         CreateImageView(const ImageViewCreateInfo& createInfo)               override;
                      void                      DestroyImageView(Handle<ImageView> handle)                           override;
        RHI_NODISCARD Handle<BufferView>        CreateBufferView(const BufferViewCreateInfo& createInfo)             override;
                      void                      DestroyBufferView(Handle<BufferView> handle)                         override;
        RHI_NODISCARD DeviceMemoryPtr           MapBuffer(Handle<Buffer> handle)                                     override;
                      void                      UnmapBuffer(Handle<Buffer> handle)                                   override;
        // Protected:
                      void                      DestroyResources()                                                   override;
        // clang-format on

        void SetDebugName(VkDebugReportObjectTypeEXT type, uint64_t handle, const char* name);

        VkSemaphore CreateSemaphore(const char* name = nullptr, bool timeline = false, uint64_t initialValue = 0);

        void DestroySemaphore(VkSemaphore semaphore);

        uint32_t GetMemoryTypeIndex(MemoryType memoryType);

        uint32_t GetQueueFamilyIndex(QueueType queueType);

    private:
        static VkBool32 VKAPI_CALL DebugMessengerCallbacks(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                           void* pUserData);

        VkResult InitInstance(const ApplicationInfo& appInfo, bool* debugExtensionEnabled);
        VkResult InitDevice();
        VkResult InitMemoryAllocator();
        VkResult LoadFunctions(bool debugExtensionEnabled);
        VkResult InitFrameScheduler();

    public:
        // clang-format off
        VkInstance       m_instance       = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice         m_device         = VK_NULL_HANDLE;
        VmaAllocator     m_allocator      = VK_NULL_HANDLE;

        VkQueue m_presentQueue  = VK_NULL_HANDLE;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_computeQueue  = VK_NULL_HANDLE;
        VkQueue m_transferQueue = VK_NULL_HANDLE;

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