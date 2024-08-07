#pragma once

#include <RHI/Context.hpp>
#include <RHI/Definitions.hpp>

#include "Resources.hpp"
#include "Queue.hpp"
#include "DeleteQueue.hpp"

#include <TL/Containers.hpp>

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    class BindGroupAllocator;
    class ICommandPool;
    class ICommandList;

    class IContext final : public Context
    {
    public:
        IContext();
        ~IContext();

        ResultCode Init(const ApplicationInfo& appInfo);

        template<typename T>
        inline void SetDebugName(T handle, const char* name) const
        {
            return SetDebugName(GetObjectType<T>(), reinterpret_cast<uint64_t>(handle), name);
        }

        void SetDebugName(VkObjectType type, uint64_t handle, const char* name) const;

        VkSemaphore CreateSemaphore(const char* name = nullptr, bool timeline = false, uint64_t initialValue = 0);

        void DestroySemaphore(VkSemaphore semaphore);

        uint32_t GetMemoryTypeIndex(MemoryType memoryType);

        uint32_t GetCurrentFrameIndex() const { return 0; }

        ICommandList* GetTransferCommand();

        // clang-format off
       TL::Ptr<Swapchain>           Internal_CreateSwapchain(const SwapchainCreateInfo& createInfo) override;
       TL::Ptr<ShaderModule>        Internal_CreateShaderModule(TL::Span<const uint32_t> shaderBlob) override;
       TL::Ptr<Fence>               Internal_CreateFence() override;
       TL::Ptr<CommandPool>         Internal_CreateCommandPool(CommandPoolFlags flags) override;
        Handle<BindGroupLayout>  Internal_CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void                     Internal_DestroyBindGroupLayout(Handle<BindGroupLayout> handle) override;
        Handle<BindGroup>        Internal_CreateBindGroup(Handle<BindGroupLayout> handle, uint32_t bindlessElementsCount) override;
        void                     Internal_DestroyBindGroup(Handle<BindGroup> handle) override;
        void                     Internal_UpdateBindGroup(Handle<BindGroup> handle, TL::Span<const BindGroupUpdateInfo> bindings) override;
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
        static VkBool32 VKAPI_CALL
        DebugMessengerCallbacks(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

        VkResult InitInstance(const ApplicationInfo& appInfo, bool* debugExtensionEnabled);
        VkResult InitDevice();
        VkResult InitMemoryAllocator();
        VkResult LoadFunctions(bool debugExtensionEnabled);

    public:
        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_device;
        VmaAllocator m_allocator;

        struct
        {
#ifdef RHI_DEBUG
            // VK_EXT_debug_utils
            PFN_vkCmdBeginDebugUtilsLabelEXT m_vkCmdBeginDebugUtilsLabelEXT;
            PFN_vkCmdEndDebugUtilsLabelEXT m_vkCmdEndDebugUtilsLabelEXT;
            PFN_vkCmdInsertDebugUtilsLabelEXT m_vkCmdInsertDebugUtilsLabelEXT;
            PFN_vkCreateDebugUtilsMessengerEXT m_vkCreateDebugUtilsMessengerEXT;
            PFN_vkDestroyDebugUtilsMessengerEXT m_vkDestroyDebugUtilsMessengerEXT;
            PFN_vkQueueBeginDebugUtilsLabelEXT m_vkQueueBeginDebugUtilsLabelEXT;
            PFN_vkQueueEndDebugUtilsLabelEXT m_vkQueueEndDebugUtilsLabelEXT;
            PFN_vkQueueInsertDebugUtilsLabelEXT m_vkQueueInsertDebugUtilsLabelEXT;
            PFN_vkSetDebugUtilsObjectNameEXT m_vkSetDebugUtilsObjectNameEXT;
            PFN_vkSetDebugUtilsObjectTagEXT m_vkSetDebugUtilsObjectTagEXT;
            PFN_vkSubmitDebugUtilsMessageEXT m_vkSubmitDebugUtilsMessageEXT;
#endif
            PFN_vkCmdBeginConditionalRenderingEXT m_vkCmdBeginConditionalRenderingEXT;
            PFN_vkCmdEndConditionalRenderingEXT m_vkCmdEndConditionalRenderingEXT;
        } m_pfn;

        Queue m_queue[QueueType::Count];

        FrameExecuteContext m_frameContext;

       TL::Ptr<BindGroupAllocator> m_bindGroupAllocator;
       TL::Ptr<ICommandPool> m_commandPool;

        HandlePool<IImage> m_imageOwner;
        HandlePool<IBuffer> m_bufferOwner;
        HandlePool<IImageView> m_imageViewOwner;
        HandlePool<IBufferView> m_bufferViewOwner;
        HandlePool<IBindGroupLayout> m_bindGroupLayoutsOwner;
        HandlePool<IBindGroup> m_bindGroupOwner;
        HandlePool<IPipelineLayout> m_pipelineLayoutOwner;
        HandlePool<IGraphicsPipeline> m_graphicsPipelineOwner;
        HandlePool<IComputePipeline> m_computePipelineOwner;
        HandlePool<ISampler> m_samplerOwner;
    };

} // namespace RHI::Vulkan