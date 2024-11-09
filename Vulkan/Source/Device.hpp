#pragma once

#include <RHI/Device.hpp>
#include <RHI/Queue.hpp>
#include <RHI-Vulkan/Loader.hpp>

#include "Timeline.hpp"
#include "Queue.hpp"
#include "DeleteQueue.hpp"

#include "BindGroup.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "Shader.hpp"
#include "Pipeline.hpp"
#include "Sampler.hpp"

#include <TL/Containers.hpp>

#include "Common.hpp"

namespace RHI::Vulkan
{
    class BindGroupAllocator;
    class ICommandPool;
    class ICommandList;

    class IDevice final : public Device
    {
    public:
        IDevice();
        ~IDevice();

        void SetDebugName(VkObjectType type, uint64_t handle, const char* name) const;

        template<typename T>
        void SetDebugName(T handle, const char* name) const;

        uint32_t GetMemoryTypeIndex(MemoryType memoryType);

        uint64_t    GetTimelineValue() const;
        VkSemaphore GetTimelineSemaphore();
        uint64_t    AdvanceTimelineValue();

    private:
        friend TL::Ptr<Device> RHI::CreateVulkanDevice(const ApplicationInfo& appInfo);

        VkResult Init(const ApplicationInfo& appInfo);
        VkResult InitInstanceAndDevice(const ApplicationInfo& appInfo);

        TL::Ptr<Swapchain>       Impl_CreateSwapchain(const SwapchainCreateInfo& createInfo) override;
        TL::Ptr<ShaderModule>    Impl_CreateShaderModule(TL::Span<const uint32_t> shaderBlob) override;
        TL::Ptr<CommandPool>     Impl_CreateCommandPool(CommandPoolFlags flags) override;
        Handle<BindGroupLayout>  Impl_CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void                     Impl_DestroyBindGroupLayout(Handle<BindGroupLayout> handle) override;
        Handle<BindGroup>        Impl_CreateBindGroup(Handle<BindGroupLayout> handle) override;
        void                     Impl_DestroyBindGroup(Handle<BindGroup> handle) override;
        void                     Impl_UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo) override;
        Handle<PipelineLayout>   Impl_CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) override;
        void                     Impl_DestroyPipelineLayout(Handle<PipelineLayout> handle) override;
        Handle<GraphicsPipeline> Impl_CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void                     Impl_DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle) override;
        Handle<ComputePipeline>  Impl_CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
        void                     Impl_DestroyComputePipeline(Handle<ComputePipeline> handle) override;
        Handle<Sampler>          Impl_CreateSampler(const SamplerCreateInfo& createInfo) override;
        void                     Impl_DestroySampler(Handle<Sampler> handle) override;
        Result<Handle<Image>>    Impl_CreateImage(const ImageCreateInfo& createInfo) override;
        void                     Impl_DestroyImage(Handle<Image> handle) override;
        Result<Handle<Buffer>>   Impl_CreateBuffer(const BufferCreateInfo& createInfo) override;
        void                     Impl_DestroyBuffer(Handle<Buffer> handle) override;
        DeviceMemoryPtr          Impl_MapBuffer(Handle<Buffer> handle) override;
        void                     Impl_UnmapBuffer(Handle<Buffer> handle) override;
        Queue*                   Impl_GetQueue(QueueType queueType) override;
        void                     Impl_CollectResources() override;
        void                     Impl_WaitTimelineValue(uint64_t value) override;

    public:
        VkInstance               m_instance;
        VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
        VkPhysicalDevice         m_physicalDevice;
        VkDevice                 m_device;
        VmaAllocator             m_allocator;

        struct
        {
#ifdef RHI_DEBUG
            // VK_EXT_debug_utils
            PFN_vkCmdBeginDebugUtilsLabelEXT    m_vkCmdBeginDebugUtilsLabelEXT;
            PFN_vkCmdEndDebugUtilsLabelEXT      m_vkCmdEndDebugUtilsLabelEXT;
            PFN_vkCmdInsertDebugUtilsLabelEXT   m_vkCmdInsertDebugUtilsLabelEXT;
            PFN_vkCreateDebugUtilsMessengerEXT  m_vkCreateDebugUtilsMessengerEXT;
            PFN_vkDestroyDebugUtilsMessengerEXT m_vkDestroyDebugUtilsMessengerEXT;
            PFN_vkQueueBeginDebugUtilsLabelEXT  m_vkQueueBeginDebugUtilsLabelEXT;
            PFN_vkQueueEndDebugUtilsLabelEXT    m_vkQueueEndDebugUtilsLabelEXT;
            PFN_vkQueueInsertDebugUtilsLabelEXT m_vkQueueInsertDebugUtilsLabelEXT;
            PFN_vkSetDebugUtilsObjectNameEXT    m_vkSetDebugUtilsObjectNameEXT;
            PFN_vkSetDebugUtilsObjectTagEXT     m_vkSetDebugUtilsObjectTagEXT;
            PFN_vkSubmitDebugUtilsMessageEXT    m_vkSubmitDebugUtilsMessageEXT;
#endif
            PFN_vkCmdBeginConditionalRenderingEXT m_vkCmdBeginConditionalRenderingEXT;
            PFN_vkCmdEndConditionalRenderingEXT   m_vkCmdEndConditionalRenderingEXT;
        } m_pfn;

        // Timeline
        VkSemaphore          m_timelineSemaphore;
        std::atomic_uint64_t m_timelineValue;

        IQueue m_queue[4];

        DeleteQueue m_deleteQueue;

        TL::Ptr<BindGroupAllocator> m_bindGroupAllocator;

        HandlePool<IImage>            m_imageOwner;
        HandlePool<IBuffer>           m_bufferOwner;
        HandlePool<IBindGroupLayout>  m_bindGroupLayoutsOwner;
        HandlePool<IBindGroup>        m_bindGroupOwner;
        HandlePool<IPipelineLayout>   m_pipelineLayoutOwner;
        HandlePool<IGraphicsPipeline> m_graphicsPipelineOwner;
        HandlePool<IComputePipeline>  m_computePipelineOwner;
        HandlePool<ISampler>          m_samplerOwner;
    };

    template<typename T>
    inline void IDevice::SetDebugName(T handle, const char* name) const
    {
        return SetDebugName(GetObjectType<T>(), reinterpret_cast<uint64_t>(handle), name);
    }

} // namespace RHI::Vulkan