#pragma once

#include <RHI/Context.hpp>
#include <RHI/QueueType.hpp>
#include <RHI/Common/Containers.h>

#include "Resources.hpp"

#include <vk_mem_alloc.h>

#include "tracy/TracyVulkan.hpp"

#include <functional>

namespace RHI::Vulkan
{
    class BindGroupAllocator;
    class ICommandPool;
    class ICommandList;

    using DestroyResource = std::function<void()>;

    class IContext final : public Context
    {
    public:
        IContext(Ptr<DebugCallbacks> debugCallbacks);
        ~IContext() = default;

        // Initialize the backend
        VkResult Init(const ApplicationInfo& appInfo);

        // Set vulkan object name
        template<typename T>
        inline void SetDebugName(T handle, const char* name) const
        {
            return SetDebugName(GetDebugReportObjectTypeEXT<T>(), reinterpret_cast<uint64_t>(handle), name);
        }

        // Set vulkan object name
        void SetDebugName(VkDebugReportObjectTypeEXT type, uint64_t handle, const char* name) const;

        // Creates a semaphore
        VkSemaphore CreateSemaphore(const char* name = nullptr, bool timeline = false, uint64_t initialValue = 0);

        // Destroys the semaphore
        void DestroySemaphore(VkSemaphore semaphore);

        // Return the memory type index of the memory type
        uint32_t GetMemoryTypeIndex(MemoryType memoryType);

        // Return the queue family index of the queue type
        uint32_t GetQueueFamilyIndex(QueueType queueType);

        // Return handle to vulkan queue with specified queue type
        VkQueue GetQueue(QueueType queueType);

        void QueueSubmit(QueueType queueType,
                         TL::Span<const ICommandList* const> commandLists,
                         TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2> waitSemaphores,
                         TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2> signalSemaphores,
                         IFence* signalFence = nullptr);

        // clang-format off
        using                    Context::DebugLogError;
        using                    Context::DebugLogInfo;
        using                    Context::DebugLogWarn;

        void                     Internal_OnShutdown() override;
        void                     Internal_OnCollectResources() override;
        Ptr<Swapchain>           Internal_CreateSwapchain(const SwapchainCreateInfo& createInfo) override;
        Ptr<ShaderModule>        Internal_CreateShaderModule(TL::Span<const uint8_t> shaderBlob) override;
        Ptr<Fence>               Internal_CreateFence() override;
        Ptr<CommandPool>         Internal_CreateCommandPool() override;
        Ptr<ResourcePool>        Internal_CreateResourcePool(const ResourcePoolCreateInfo& createInfo) override;
        Handle<BindGroupLayout>  Internal_CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void                     Internal_DestroyBindGroupLayout(Handle<BindGroupLayout> handle) override;
        Handle<BindGroup>        Internal_CreateBindGroup(Handle<BindGroupLayout> handle) override;
        void                     Internal_DestroyBindGroup(Handle<BindGroup> handle) override;
        void                     Internal_UpdateBindGroup(Handle<BindGroup> handle, const BindGroupData& data) override;
        Handle<PipelineLayout>   Internal_CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) override;
        void                     Internal_DestroyPipelineLayout(Handle<PipelineLayout> handle) override;
        Handle<GraphicsPipeline> Internal_CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void                     Internal_DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle) override;
        Handle<ComputePipeline>  Internal_CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
        void                     Internal_DestroyComputePipeline(Handle<ComputePipeline> handle) override;
        Handle<Sampler>          Internal_CreateSampler(const SamplerCreateInfo& createInfo) override;
        void                     Internal_DestroySampler(Handle<Sampler> handle) override;
        Result<Handle<Image>>    Internal_CreateImage(const ImageCreateInfo& createInfo) override;
        void                     Internal_DestroyImage(Handle<Image> handle) override;
        Result<Handle<Buffer>>   Internal_CreateBuffer(const BufferCreateInfo& createInfo) override;
        void                     Internal_DestroyBuffer(Handle<Buffer> handle) override;
        Handle<ImageView>        Internal_CreateImageView(const ImageViewCreateInfo& createInfo) override;
        void                     Internal_DestroyImageView(Handle<ImageView> handle) override;
        Handle<BufferView>       Internal_CreateBufferView(const BufferViewCreateInfo& createInfo) override;
        void                     Internal_DestroyBufferView(Handle<BufferView> handle) override;
        void                     Internal_DispatchGraph(RenderGraph& renderGraph, Fence* signalFence) override;
        DeviceMemoryPtr          Internal_MapBuffer(Handle<Buffer> handle) override;
        void                     Internal_UnmapBuffer(Handle<Buffer> handle) override;
        void                     Internal_StageResourceWrite(Handle<Image> image, ImageSubresourceLayers subresources, Handle<Buffer> buffer, size_t bufferOffset) override;
        void                     Internal_StageResourceWrite(Handle<Buffer> buffer, size_t offset, size_t size, Handle<Buffer> srcBuffer, size_t srcOffset) override;
        void                     Internal_StageResourceRead(Handle<Image> image, ImageSubresourceLayers subresources, Handle<Buffer> buffer, size_t bufferOffset, Fence* fence) override;
        void                     Internal_StageResourceRead(Handle<Buffer> buffer, size_t offset, size_t size, Handle<Buffer> srcBuffer, size_t srcOffset, Fence* fence) override;
        // clang-format on

    private:
        static VkBool32 VKAPI_CALL DebugMessengerCallbacks(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                           void* pUserData);

        VkResult InitInstance(const ApplicationInfo& appInfo, bool* debugExtensionEnabled);
        VkResult InitDevice();
        VkResult InitMemoryAllocator();
        VkResult LoadFunctions(bool debugExtensionEnabled);

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

        PFN_vkCmdDebugMarkerBeginEXT                       m_vkCmdDebugMarkerBeginEXT                       = nullptr;
        PFN_vkCmdDebugMarkerInsertEXT                      m_vkCmdDebugMarkerInsertEXT                      = nullptr;
        PFN_vkCmdDebugMarkerEndEXT                         m_vkCmdDebugMarkerEndEXT                         = nullptr;
        PFN_vkCmdBeginConditionalRenderingEXT              m_vkCmdBeginConditionalRenderingEXT              = nullptr;
        PFN_vkCmdEndConditionalRenderingEXT                m_vkCmdEndConditionalRenderingEXT                = nullptr;
        PFN_vkDebugMarkerSetObjectNameEXT                  m_vkDebugMarkerSetObjectNameEXT                  = nullptr;
        Ptr<BindGroupAllocator> m_bindGroupAllocator;
        Ptr<ICommandPool>       m_commandPool;

        TracyVkCtx m_tracyContext;

        TL::Vector<DestroyResource> m_resourceDestroyQueue;

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
    };

} // namespace RHI::Vulkan