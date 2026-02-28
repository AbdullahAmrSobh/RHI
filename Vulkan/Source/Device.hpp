#pragma once

#include <RHI/Device.hpp>
#include <RHI-Vulkan/Loader.hpp>

#include <TL/Ptr.hpp>
#include <TL/Stacktrace.hpp>
#include <TL/Containers/Vector.hpp>

// #include <Volk/volk.h>
#include <vk_mem_alloc.h>

#include "Common.hpp"
#include "Resources.hpp"
#include "Frame.hpp"

namespace RHI::Vulkan
{
    class BindGroupAllocator;
    class ICommandList;

    struct VulkanAPI
    {
#ifdef RHI_DEBUG
        // VK_EXT_debug_utils
        PFN_vkCmdBeginDebugUtilsLabelEXT    vkCmdBeginDebugUtilsLabelEXT;
        PFN_vkCmdEndDebugUtilsLabelEXT      vkCmdEndDebugUtilsLabelEXT;
        PFN_vkCmdInsertDebugUtilsLabelEXT   vkCmdInsertDebugUtilsLabelEXT;
        PFN_vkCreateDebugUtilsMessengerEXT  vkCreateDebugUtilsMessengerEXT;
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
        PFN_vkQueueBeginDebugUtilsLabelEXT  vkQueueBeginDebugUtilsLabelEXT;
        PFN_vkQueueEndDebugUtilsLabelEXT    vkQueueEndDebugUtilsLabelEXT;
        PFN_vkQueueInsertDebugUtilsLabelEXT vkQueueInsertDebugUtilsLabelEXT;
        PFN_vkSetDebugUtilsObjectNameEXT    vkSetDebugUtilsObjectNameEXT;
        PFN_vkSetDebugUtilsObjectTagEXT     vkSetDebugUtilsObjectTagEXT;
        PFN_vkSubmitDebugUtilsMessageEXT    vkSubmitDebugUtilsMessageEXT;
#endif
        PFN_vkCmdBeginConditionalRenderingEXT  vkCmdBeginConditionalRenderingEXT;
        PFN_vkCmdEndConditionalRenderingEXT    vkCmdEndConditionalRenderingEXT;
        PFN_vkCreateRayTracingPipelinesKHR     vkCreateRayTracingPipelinesKHR;
        PFN_vkCmdTraceRaysIndirect2KHR         vkCmdTraceRaysIndirect2KHR;
        PFN_vkCmdPushDescriptorSet2KHR         vkCmdPushDescriptorSet2KHR;
        PFN_vkCmdTraceRaysKHR                  vkCmdTraceRaysKHR;
        PFN_vkCmdDrawMeshTasksEXT              vkCmdDrawMeshTasksEXT;
        PFN_vkCmdDrawMeshTasksIndirectEXT      vkCmdDrawMeshTasksIndirectEXT;
        PFN_vkCmdDrawMeshTasksIndirectCountEXT vkCmdDrawMeshTasksIndirectCountEXT;
    };

    class IQueue
    {
    public:
        VkResult Init(IDevice* device, const char* debugName, uint32_t familyIndex, uint32_t queueIndex);

        void Shutdown();

        void waitTimelineQueueIdle() const;

        bool waitTimeline(uint64_t timelineValue, uint64_t duration = UINT64_MAX);

        void waitTimelineSemaphore(VkSemaphore semaphore, uint64_t value, VkPipelineStageFlags2 stageMask);

        void waitBinarySemaphore(VkSemaphore semaphore, VkPipelineStageFlags2 stageMask) { waitTimelineSemaphore(semaphore, 0, stageMask); }

        void signalTimelineSemaphore(VkSemaphore semaphore, uint64_t value, VkPipelineStageFlags2 stageMask);

        void signalBinarySemaphore(VkSemaphore semaphore, VkPipelineStageFlags2 stageMask) { signalTimelineSemaphore(semaphore, 0, stageMask); }

        void beginDebugUtilsLabel(const char* name);

        void endDebugUtilsLabel();

        uint64_t submit(TL::Span<ICommandList* const> commandLists, VkPipelineStageFlags2 signalStage);

        IDevice*             m_device            = nullptr;
        VkQueue              m_queue             = VK_NULL_HANDLE;
        uint32_t             m_familyIndex       = UINT32_MAX;
        VkSemaphore          m_timelineSemaphore = VK_NULL_HANDLE;
        std::atomic_uint64_t m_timelineValue     = {0};

    private:
        TL::Vector<VkSemaphoreSubmitInfo> m_waitTimelineSemaphores;
        TL::Vector<VkSemaphoreSubmitInfo> m_signalSemaphores;
    };

    class IDevice final : public Device
    {
    public:
        friend Device* RHI::CreateVulkanDevice(const ApplicationInfo& appInfo);
        friend void    RHI::DestroyVulkanDevice(Device* device);

        IDevice();
        ~IDevice();

        ResultCode Init(const ApplicationInfo& appInfo);
        void       Shutdown();

        /// @brief Assigns a debug name to a Vulkan object.
        void SetDebugName(VkObjectType type, uint64_t handle, const char* name) const;
        template<typename T>
        void SetDebugName(T handle, const char* name) const;
        template<typename T, typename... FMT_ARGS>
        void SetDebugName(T handle, const char* fmt, FMT_ARGS... args) const;

        IQueue* GetDeviceQueue(QueueType type);

        void waitTimelineIdle();

        // Interface Implementation
        uint64_t            GetNativeHandle(NativeHandleType type, uint64_t handle) override;
        Swapchain*          CreateSwapchain(const SwapchainCreateInfo& createInfo) override;
        void                DestroySwapchain(Swapchain* swapchain) override;
        ShaderModule*       CreateShaderModule(const ShaderModuleCreateInfo& createInfo) override;
        void                DestroyShaderModule(ShaderModule* shaderModule) override;
        BindGroupLayout*    CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void                DestroyBindGroupLayout(BindGroupLayout* handle) override;
        BindGroup*          CreateBindGroup(const BindGroupCreateInfo& createInfo) override;
        void                DestroyBindGroup(BindGroup* handle) override;
        void                UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo) override;
        QueryPool*          CreateQueryPool(const QueryPoolCreateInfo& createInfo) override;
        void                DestroyQueryPool(QueryPool* handle) override;
        Buffer*             CreateBuffer(const BufferCreateInfo& createInfo) override;
        void                DestroyBuffer(Buffer* handle) override;
        Image*              CreateImage(const ImageCreateInfo& createInfo) override;
        Image*              CreateImageView(const ImageViewCreateInfo& createInfo) override;
        void                DestroyImage(Image* handle) override;
        Sampler*            CreateSampler(const SamplerCreateInfo& createInfo) override;
        void                DestroySampler(Sampler* handle) override;
        PipelineLayout*     CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) override;
        void                DestroyPipelineLayout(PipelineLayout* handle) override;
        GraphicsPipeline*   CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void                DestroyGraphicsPipeline(GraphicsPipeline* handle) override;
        RayTracingPipeline* CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo) override;
        void                DestroyRayTracingPipeline(RayTracingPipeline* handle) override;
        ComputePipeline*    CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
        void                DestroyComputePipeline(ComputePipeline* handle) override;
        ResultCode          SetFramesInFlightCount(uint32_t count) override;
        Frame*              GetCurrentFrame() override;

        /// Frame
        TL::IAllocator& GetTempAllocator();

        ///

    public:
        // Vulkan instance and core objects
        VkInstance                        m_instance                = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT          m_debugUtilsMessenger     = VK_NULL_HANDLE;
        VkPhysicalDevice                  m_physicalDevice          = VK_NULL_HANDLE;
        VkDevice                          m_device                  = VK_NULL_HANDLE;
        VmaAllocator                      m_deviceAllocator         = VK_NULL_HANDLE;
        VulkanAPI                         m_pfn                     = {};
        TL::Ptr<Renderdoc>                m_renderdoc               = nullptr;
        // Queue and allocator management
        IQueue                            m_queue[AsyncQueuesCount] = {};
        BindGroupAllocator                m_bindGroupAllocator;
        TL::Ptr<class DeleteQueue>        m_destroyQueue      = nullptr;
        // Frames in flight
        uint32_t                          m_currentFrameIndex = 0;
        TL::Vector<TL::Ptr<class IFrame>> m_framesInFlight    = {};

    private:
        TL::Map<IQueryPool*, TL::Stacktrace>          m_liveQueryPools;
        TL::Map<ISwapchain*, TL::Stacktrace>          m_liveSwapchains;
        TL::Map<IShaderModule*, TL::Stacktrace>       m_liveShaderModules;
        TL::Map<IImage*, TL::Stacktrace>              m_liveImages;
        TL::Map<IBuffer*, TL::Stacktrace>             m_liveBuffers;
        TL::Map<IBindGroupLayout*, TL::Stacktrace>    m_liveBindGroupLayouts;
        TL::Map<IBindGroup*, TL::Stacktrace>          m_liveBindGroups;
        TL::Map<IPipelineLayout*, TL::Stacktrace>     m_livePipelineLayouts;
        TL::Map<IGraphicsPipeline*, TL::Stacktrace>   m_liveGraphicsPipelines;
        TL::Map<IRayTracingPipeline*, TL::Stacktrace> m_liveRayTracingPipelines;
        TL::Map<IComputePipeline*, TL::Stacktrace>    m_liveComputePipelines;
        TL::Map<ISampler*, TL::Stacktrace>            m_liveSamplers;
    };

    template<typename T>
    inline void IDevice::SetDebugName(T handle, const char* name) const
    {
        return SetDebugName(GetObjectType<T>(), reinterpret_cast<uint64_t>(handle), name);
    }

    template<typename T, typename... FMT_ARGS>
    void IDevice::SetDebugName(T handle, const char* fmt, FMT_ARGS... args) const
    {
        auto name = std::vformat(fmt, std::make_format_args(args...));
        SetDebugName(handle, name.c_str());
    }

    inline IQueue* IDevice::GetDeviceQueue(QueueType type)
    {
        auto& queue = m_queue[(uint32_t)type];
        if (queue.m_queue != VK_NULL_HANDLE)
        {
            return &m_queue[(int)QueueType::Graphics];
        }
        return nullptr;
    }

    using VmaImageAllocation  = std::pair<VkImage, VmaAllocation>;
    using VmaBufferAllocation = std::pair<VkBuffer, VmaAllocation>;

    // Entry for any resource type
    template<typename Resource>
    struct ResourceDeleteQueueEntry
    {
        uint64_t       timeline;
        Resource       resource;
        TL::Stacktrace stacktrace;
    };

    class DeleteQueue
    {
    public:
        void shutdown(IDevice* device);

        // clang-format off
        // simple handle pushes
        void Push(uint64_t timeline, VmaAllocation h) { PushImpl(m_allocation, timeline, h); }
        void Push(uint64_t timeline, VkBuffer h) { PushImpl(m_buffer, timeline, h); }
        void Push(uint64_t timeline, VkBufferView h) { PushImpl(m_bufferView, timeline, h); }
        void Push(uint64_t timeline, VkImage h) { PushImpl(m_image, timeline, h); }
        void Push(uint64_t timeline, VkImageView h) { PushImpl(m_imageView, timeline, h); }
        void Push(uint64_t timeline, VkSampler h) { PushImpl(m_sampler, timeline, h); }
        void Push(uint64_t timeline, VkPipeline h) { PushImpl(m_pipeline, timeline, h); }
        void Push(uint64_t timeline, VkDescriptorPool h) { PushImpl(m_descriptorPool, timeline, h); }
        void Push(uint64_t timeline, VkQueryPool h) { PushImpl(m_queryPool, timeline, h); }
        void Push(uint64_t timeline, VkSwapchainKHR h) { PushImpl(m_swapchain, timeline, h); }
        void Push(uint64_t timeline, VkSurfaceKHR h) { PushImpl(m_surface, timeline, h); }
        void Push(uint64_t timeline, VkSemaphore h) { PushImpl(m_semaphore, timeline, h); }
        // void Push(uint64_t timeline, VmaBufferAllocation h) { PushImpl(, timeline, h.first);  PushImpl(m_vmaBuffer, timeline, h.second);}
        // void Push(uint64_t timeline, VmaImageAllocation h) { PushImpl(m_vmaImage, timeline, h.first);  PushImpl(m_vmaImage, timeline, h.second);}
        // clang-format on

        void Flush(IDevice* device, uint64_t timeline);

    private:
        template<typename ResourceType>
        void FlushQueue(IDevice* device, TL::Vector<ResourceDeleteQueueEntry<ResourceType>>& queue, uint64_t timeline);

        // Generic push implementation for single-handle resources
        template<typename ResourceType>
        void PushImpl(TL::Vector<ResourceDeleteQueueEntry<ResourceType>>& queue, uint64_t timeline, ResourceType h)
        {
            uint64_t key = TL::hashBytes(TL::Block::create(h));
            if (auto it = m_pending.find(key); it != m_pending.end())
            {
                auto st = TL::ReportStacktrace(it->second);
                TL_LOG_ERROR("Object was already requested for deletion at {}", st);
                TL_UNREACHABLE();
            }
            else
            {
                m_pending[key] = TL::CaptureStacktrace();
            }

            queue.emplace_back(ResourceDeleteQueueEntry<ResourceType>{timeline, h, TL::CaptureStacktrace()});
        }

    private:
        TL::Vector<ResourceDeleteQueueEntry<VmaAllocation>>       m_allocation;
        TL::Vector<ResourceDeleteQueueEntry<VkBuffer>>            m_buffer;
        TL::Vector<ResourceDeleteQueueEntry<VkBufferView>>        m_bufferView;
        TL::Vector<ResourceDeleteQueueEntry<VkImage>>             m_image;
        TL::Vector<ResourceDeleteQueueEntry<VkImageView>>         m_imageView;
        TL::Vector<ResourceDeleteQueueEntry<VkSampler>>           m_sampler;
        TL::Vector<ResourceDeleteQueueEntry<VkPipeline>>          m_pipeline;
        TL::Vector<ResourceDeleteQueueEntry<VkDescriptorPool>>    m_descriptorPool;
        TL::Vector<ResourceDeleteQueueEntry<VkQueryPool>>         m_queryPool;
        TL::Vector<ResourceDeleteQueueEntry<VkSwapchainKHR>>      m_swapchain;
        TL::Vector<ResourceDeleteQueueEntry<VkSurfaceKHR>>        m_surface;
        TL::Vector<ResourceDeleteQueueEntry<VkSemaphore>>         m_semaphore;
        TL::Vector<ResourceDeleteQueueEntry<VmaBufferAllocation>> m_vmaBuffer;
        TL::Vector<ResourceDeleteQueueEntry<VmaImageAllocation>>  m_vmaImage;
        TL::Map<uint64_t, TL::Stacktrace>                         m_pending;
    };

} // namespace RHI::Vulkan