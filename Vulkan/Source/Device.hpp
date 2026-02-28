#pragma once

#include <RHI/Device.hpp>
#include <RHI-Vulkan/Loader.hpp>

#include <TL/Ptr.hpp>
#include <TL/Stacktrace.hpp>
#include <TL/Containers/Vector.hpp>

// #include <Volk/volk.h>
#include <vk_mem_alloc.h>

#include "Common.hpp"
#include "Queue.hpp"
#include "Resources.hpp"
#include "Frame.hpp"

namespace RHI::Vulkan
{
    class BindGroupAllocator;

    struct VulkanAPI
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
        PFN_vkCmdBeginConditionalRenderingEXT  m_vkCmdBeginConditionalRenderingEXT;
        PFN_vkCmdEndConditionalRenderingEXT    m_vkCmdEndConditionalRenderingEXT;
        PFN_vkCreateRayTracingPipelinesKHR     m_vkCreateRayTracingPipelinesKHR;
        PFN_vkCmdTraceRaysIndirect2KHR         m_vkCmdTraceRaysIndirect2KHR;
        PFN_vkCmdPushDescriptorSet2KHR         m_vkCmdPushDescriptorSet2KHR;
        PFN_vkCmdTraceRaysKHR                  m_vkCmdTraceRaysKHR;
        PFN_vkCmdDrawMeshTasksEXT              m_vkCmdDrawMeshTasksEXT;
        PFN_vkCmdDrawMeshTasksIndirectEXT      m_vkCmdDrawMeshTasksIndirectEXT;
        PFN_vkCmdDrawMeshTasksIndirectCountEXT m_vkCmdDrawMeshTasksIndirectCountEXT;
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

        void WaitIdle();

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
        RayTracingPipeline* CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo) override;
        void                DestroyRayTracingPipeline(RayTracingPipeline* handle) override;
        void                DestroyGraphicsPipeline(GraphicsPipeline* handle) override;
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
        TL::Ptr<class DeleteQueue>        m_destroyQueue            = nullptr;
        TL::Ptr<class BindGroupAllocator> m_bindGroupAllocator      = nullptr;
        // Frames in flight
        uint32_t                          m_currentFrameIndex       = 0;
        TL::Vector<TL::Ptr<class IFrame>> m_framesInFlight          = {};

    private:
        TL::Map<QueryPool*, TL::Stacktrace>          m_liveQueryPools;
        TL::Map<Swapchain*, TL::Stacktrace>          m_liveSwapchains;
        TL::Map<ShaderModule*, TL::Stacktrace>       m_liveShaderModules;
        TL::Map<Image*, TL::Stacktrace>              m_liveImages;
        TL::Map<Buffer*, TL::Stacktrace>             m_liveBuffers;
        TL::Map<BindGroupLayout*, TL::Stacktrace>    m_liveBindGroupLayouts;
        TL::Map<BindGroup*, TL::Stacktrace>          m_liveBindGroups;
        TL::Map<PipelineLayout*, TL::Stacktrace>     m_livePipelineLayouts;
        TL::Map<GraphicsPipeline*, TL::Stacktrace>   m_liveGraphicsPipelines;
        TL::Map<RayTracingPipeline*, TL::Stacktrace> m_liveRayTracingPipelines;
        TL::Map<ComputePipeline*, TL::Stacktrace>    m_liveComputePipelines;
        TL::Map<Sampler*, TL::Stacktrace>            m_liveSamplers;
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
        if (queue.GetHandle() != VK_NULL_HANDLE)
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

        template<typename ResourceType>
        inline static void destroyVkResource(IDevice* device, ResourceType handle)
        {
            if constexpr (std::is_same_v<VmaAllocation, ResourceType>) vmaFreeMemory(device->m_deviceAllocator, handle);
            else if constexpr (std::is_same_v<VkBuffer, ResourceType>) vkDestroyBuffer(device->m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkBufferView, ResourceType>) vkDestroyBufferView(device->m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkImage, ResourceType>) vkDestroyImage(device->m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkImageView, ResourceType>) vkDestroyImageView(device->m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkSampler, ResourceType>) vkDestroySampler(device->m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkPipeline, ResourceType>) vkDestroyPipeline(device->m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkDescriptorPool, ResourceType>) vkDestroyDescriptorPool(device->m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkQueryPool, ResourceType>) vkDestroyQueryPool(device->m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkSemaphore, ResourceType>) vkDestroySemaphore(device->m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkSwapchainKHR, ResourceType>) vkDestroySwapchainKHR(device->m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkSurfaceKHR, ResourceType>) vkDestroySurfaceKHR(device->m_instance, handle, nullptr);
            else if constexpr (std::is_same_v<VmaBufferAllocation, ResourceType>) vmaDestroyBuffer(device->m_deviceAllocator, handle.first, handle.second);
            else if constexpr (std::is_same_v<VmaImageAllocation, ResourceType>) vmaDestroyImage(device->m_deviceAllocator, handle.first, handle.second);
            else
            {
                static_assert(false, "Invalid ResourceType");
            }
        }

    private:
        // FlushQueue that works on ResourceDeleteQueueEntry<ResourceType>
        template<typename ResourceType>
        void FlushQueue(IDevice* device, TL::Vector<ResourceDeleteQueueEntry<ResourceType>>& queue, uint64_t timeline)
        {
            uint32_t deleteCount = 0;
            for (const auto& entry : queue)
            {
                if (entry.timeline > timeline)
                    break;

                destroyVkResource(device, entry.resource);

                uint64_t key = TL::hashBytes(TL::Block::create(entry.resource));
                TL_ASSERT(m_pending.erase(key));
                deleteCount++;
            }
            queue.erase(queue.begin(), queue.begin() + deleteCount);
        }

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