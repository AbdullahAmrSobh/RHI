#pragma once

#include <RHI/Device.hpp>
#include <RHI-Vulkan/Loader.hpp>

#include <TL/Ptr.hpp>
#include <TL/Stacktrace.hpp>
#include <TL/Containers/Vector.hpp>
#include <TL/Utils.hpp>

#define VK_USE_PLATFORM_WIN32_KHR
#include <volk.h>
#include <vk_mem_alloc.h>

#include "Common.hpp"
#include "Resources.hpp"
#include "CommandList.hpp"

namespace RHI::Vulkan
{
    class IQueue final : public Queue
    {
    public:
        VkResult Init(IDevice* device, const char* debugName, uint32_t familyIndex, uint32_t queueIndex);
        void     Shutdown();

        void BeginAnnotation(const char* name, uint32_t bgra) override;
        void EndAnnotation() override;
        void InsertAnnotation(const char* name, uint32_t bgra) override;
        void Submit(const QueueSubmitInfo& submitInfo) override;
        void WaitIdle() override;
        void WaitFence(Fence* fence, uint64_t value) override;

        IDevice*             m_device;
        VkQueue              m_queue;
        uint32_t             m_familyIndex;
        QueueType            m_queueType;
        std::atomic_uint64_t m_lastSubmitValue;
    };

    class IDevice final : public RHI::Device
    {
    public:
        friend Device* RHI::CreateVulkanDevice(const ApplicationInfo& appInfo);
        friend void    RHI::DestroyVulkanDevice(Device* device);

        IDevice();
        ~IDevice();

        ResultCode Init(const ApplicationInfo& appInfo);
        void       Shutdown();

        void SetDebugName(VkObjectType type, uint64_t handle, const char* name) const;

        template<typename T>
        void SetDebugName(T handle, const char* name) const;

        template<typename T, typename... FMT_ARGS>
        void SetDebugName(T handle, const char* fmt, FMT_ARGS... args) const;

        void WaitIdle();

        // Interface Implementation
        uint64_t               GetNativeHandle(NativeHandleType type, uint64_t handle) override;
        uint64_t               GarbageCollect(uint64_t graphicsTimeline) override;
        Queue*                 GetQueue(QueueType queueType) override;
        CommandPool*           CreateCommandPool(const CommandPoolCreateInfo& createInfo) override;
        void                   DestroyCommandPool(CommandPool* handle) override;
        Fence*                 CreateFence(const FenceCreateInfo& createInfo) override;
        void                   DestroyFence(Fence* handle) override;
        uint64_t               GetFenceValue(Fence* handle) override;
        Swapchain*             CreateSwapchain(const SwapchainCreateInfo& createInfo) override;
        void                   DestroySwapchain(Swapchain* swapchain) override;
        ShaderModule*          CreateShaderModule(const ShaderModuleCreateInfo& createInfo) override;
        void                   DestroyShaderModule(ShaderModule* shaderModule) override;
        BindGroupLayout*       CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void                   DestroyBindGroupLayout(BindGroupLayout* handle) override;
        BindGroup*             CreateBindGroup(const BindGroupCreateInfo& createInfo) override;
        void                   DestroyBindGroup(BindGroup* handle) override;
        void                   UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo) override;
        QueryPool*             CreateQueryPool(const QueryPoolCreateInfo& createInfo) override;
        void                   DestroyQueryPool(QueryPool* handle) override;
        Buffer*                CreateBuffer(const BufferCreateInfo& createInfo) override;
        void                   DestroyBuffer(Buffer* handle) override;
        uint64_t               GetBufferDeviceAddress(Buffer* buffer) override;
        DeviceMemoryPtr        MapBuffer(Buffer* buffer, uint64_t offset, uint64_t sizeBytes) override;
        void                   UnmapBuffer(Buffer* buffer) override;
        Image*                 CreateImage(const ImageCreateInfo& createInfo) override;
        Image*                 CreateImageView(const ImageViewCreateInfo& createInfo) override;
        void                   DestroyImage(Image* handle) override;
        AccelerationStructure* CreateAccelerationStructure(const AccelerationStructureCreateInfo& createInfo) override;
        void                   DestroyAccelerationStructure(AccelerationStructure* handle) override;
        Micromap*              CreateMicromap(const MicromapCreateInfo& createInfo) override;
        void                   DestroyMicromap(Micromap* handle) override;
        Sampler*               CreateSampler(const SamplerCreateInfo& createInfo) override;
        void                   DestroySampler(Sampler* handle) override;
        PipelineLayout*        CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) override;
        void                   DestroyPipelineLayout(PipelineLayout* handle) override;
        GraphicsPipeline*      CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void                   DestroyGraphicsPipeline(GraphicsPipeline* handle) override;
        RayTracingPipeline*    CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo) override;
        void                   DestroyRayTracingPipeline(RayTracingPipeline* handle) override;
        ComputePipeline*       CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
        void                   DestroyComputePipeline(ComputePipeline* handle) override;

    public:
        // Vulkan instance and core objects
        VkInstance                 m_instance                          = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT   m_debugUtilsMessenger               = VK_NULL_HANDLE;
        VkPhysicalDevice           m_physicalDevice                    = VK_NULL_HANDLE;
        VkDevice                   m_device                            = VK_NULL_HANDLE;
        VmaAllocator               m_deviceAllocator                   = VK_NULL_HANDLE;
        IQueue                     m_queue[(uint32_t)QueueType::Count] = {};
        BindGroupAllocator         m_bindGroupAllocator;
        TL::Ptr<class DeleteQueue> m_destroyQueue = nullptr;
        TL::Arena                  m_arena;
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

    using VmaImageAllocation  = std::pair<VkImage, VmaAllocation>;
    using VmaBufferAllocation = std::pair<VkBuffer, VmaAllocation>;

    // Entry for any resource type
    template<typename Resource>
    struct ResourceDeleteQueueEntry
    {
        uint64_t timeline;
        Resource resource;
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

        // Returns a unique uint64_t per ResourceType, collision-free across types.
        // Uses a static-local-variable address as a zero-cost type identity.
        template<typename ResourceType>
        static uint64_t typeKey()
        {
            static char s_tag;
            return reinterpret_cast<uint64_t>(&s_tag);
        }

        // Generic push implementation for single-handle resources
        template<typename ResourceType>
        void PushImpl(TL::Vector<ResourceDeleteQueueEntry<ResourceType>>& queue, uint64_t timeline, ResourceType h)
        {
            static_assert(sizeof(ResourceType) <= sizeof(uint64_t), "ResourceType must fit in a uint64_t key");
            uint64_t handleVal = 0;
            memcpy(&handleVal, &h, sizeof(h));
            uint64_t key = TL::HashCombine(typeKey<ResourceType>(), handleVal);

            if (auto it = m_pending.find(key); it != m_pending.end())
            {
                auto st = TL::ReportStacktrace(it->second);
                TL::LogError("Object was already requested for deletion at {}", st);
                TL_UNREACHABLE();
            }
            else
            {
                m_pending.emplace(std::make_pair(key, TL::CaptureStacktrace()));
            }

            queue.emplace_back(ResourceDeleteQueueEntry<ResourceType>{timeline, h});
        }

    private:
        TL::Vector<ResourceDeleteQueueEntry<VmaAllocation>>    m_allocation;
        TL::Vector<ResourceDeleteQueueEntry<VkBuffer>>         m_buffer;
        TL::Vector<ResourceDeleteQueueEntry<VkBufferView>>     m_bufferView;
        TL::Vector<ResourceDeleteQueueEntry<VkImage>>          m_image;
        TL::Vector<ResourceDeleteQueueEntry<VkImageView>>      m_imageView;
        TL::Vector<ResourceDeleteQueueEntry<VkSampler>>        m_sampler;
        TL::Vector<ResourceDeleteQueueEntry<VkPipeline>>       m_pipeline;
        TL::Vector<ResourceDeleteQueueEntry<VkDescriptorPool>> m_descriptorPool;
        TL::Vector<ResourceDeleteQueueEntry<VkQueryPool>>      m_queryPool;
        TL::Vector<ResourceDeleteQueueEntry<VkSwapchainKHR>>   m_swapchain;
        TL::Vector<ResourceDeleteQueueEntry<VkSurfaceKHR>>     m_surface;
        TL::Vector<ResourceDeleteQueueEntry<VkSemaphore>>      m_semaphore;
        TL::Map<uint64_t, TL::Stacktrace>                      m_pending;
    };

} // namespace RHI::Vulkan