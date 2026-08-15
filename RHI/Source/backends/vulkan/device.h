#pragma once

#include <TL/Log.hpp>
#include <TL/Utils.hpp>
#include <TL/Fmt.hpp>
#include <TL/Containers/Vector.hpp>
#include <TL/Containers/StringView.hpp>

#include <cstddef>
#include <mutex>

// #define VK_USE_PLATFORM_WIN32_KHR
#include <volk.h>
#include <vk_mem_alloc.h>

#include "common.h"
#include "resources.h"
#include "command-list.h"

namespace RHI::Vulkan
{

    enum class DeferredResourceType : uint8_t
    {
        Allocation,
        Buffer,
        BufferView,
        Image,
        ImageView,
        Sampler,
        Pipeline,
        DescriptorPool,
        QueryPool,
        Swapchain,
        Surface,
        Semaphore,
        AccelerationStructure,
        Micromap,
    };

    struct DeferredResource
    {
        uint64_t timeline;
        uint64_t handle;
        DeferredResourceType type;
    };

    struct DeleteQueue
    {
        TL::Vector<DeferredResource> m_entries;

        void shutdown(IDevice* device);

        void Push(uint64_t timeline, VmaAllocation h) { Push(DeferredResourceType::Allocation, timeline, h); }

        void Push(uint64_t timeline, VkBuffer h) { Push(DeferredResourceType::Buffer, timeline, h); }

        void Push(uint64_t timeline, VkBufferView h) { Push(DeferredResourceType::BufferView, timeline, h); }

        void Push(uint64_t timeline, VkImage h) { Push(DeferredResourceType::Image, timeline, h); }

        void Push(uint64_t timeline, VkImageView h) { Push(DeferredResourceType::ImageView, timeline, h); }

        void Push(uint64_t timeline, VkSampler h) { Push(DeferredResourceType::Sampler, timeline, h); }

        void Push(uint64_t timeline, VkPipeline h) { Push(DeferredResourceType::Pipeline, timeline, h); }

        void Push(uint64_t timeline, VkDescriptorPool h) { Push(DeferredResourceType::DescriptorPool, timeline, h); }

        void Push(uint64_t timeline, VkQueryPool h) { Push(DeferredResourceType::QueryPool, timeline, h); }

        void Push(uint64_t timeline, VkSwapchainKHR h) { Push(DeferredResourceType::Swapchain, timeline, h); }

        void Push(uint64_t timeline, VkSurfaceKHR h) { Push(DeferredResourceType::Surface, timeline, h); }

        void Push(uint64_t timeline, VkSemaphore h) { Push(DeferredResourceType::Semaphore, timeline, h); }

        void Push(uint64_t timeline, VkAccelerationStructureKHR h) { Push(DeferredResourceType::AccelerationStructure, timeline, h); }

        void Push(uint64_t timeline, VkMicromapEXT h) { Push(DeferredResourceType::Micromap, timeline, h); }

        template<typename ResourceType>
        void Push(DeferredResourceType type, uint64_t timeline, ResourceType resource)
        {
            uint64_t handle = std::bit_cast<uint64_t>(resource);
#if TL_DEBUG
            for (const DeferredResource& entry : m_entries)
            {
                TL_ASSERT(entry.type != type || entry.handle != handle, "Object was already requested for deletion");
            }
#endif
            m_entries.push_back({.timeline = timeline, .handle = handle, .type = type});
        }

        void Flush(IDevice* device, uint64_t timeline);
    };

    struct IQueue : RHI::Queue
    {
        IDevice* m_device;
        VkQueue m_queue;
        VkSemaphore m_submissionTimeline = VK_NULL_HANDLE;
        void* m_tracyContext = nullptr;
        std::mutex m_tracyCollectMutex;
        uint32_t m_familyIndex;
        QueueType m_queueType;
        std::atomic_uint64_t m_lastSubmitValue{0};

        VkResult Init(IDevice* device, const char* debugName, uint32_t familyIndex, uint32_t queueIndex, VkQueueFlags queueFlags, uint32_t timestampValidBits);
        void Shutdown();
    };

    struct IDevice : RHI::Device
    {
        BackendType m_backend;
        DeviceLimits m_limits;
        DeviceFeatures m_features;

        VkInstance               m_instance                          = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugUtilsMessenger               = VK_NULL_HANDLE;
        VkPhysicalDevice         m_physicalDevice                    = VK_NULL_HANDLE;
        VkDevice                 m_device                            = VK_NULL_HANDLE;
        VmaAllocator             m_deviceAllocator                   = VK_NULL_HANDLE;
        IQueue                   m_queue[(uint32_t)QueueType::Count] = {};
        BindGroupAllocator       m_bindGroupAllocator;
        DeleteQueue              m_destroyQueue;

        ResultCode Init(const ApplicationInfo& appInfo);
        void Shutdown();

        void SetDebugName(VkObjectType type, uint64_t handle, TL::StringView name) const;

        void WaitIdle();

        // Internal accessor returning backend queue state directly (facade-free).
        IQueue* getQueue(QueueType queueType) { return &m_queue[(uint32_t)queueType]; }
    };

    // Queue interface functions

    void queueBeginAnnotation(IQueue* self, const char* name, uint32_t bgra);
    void queueEndAnnotation(IQueue* self);
    void queueInsertAnnotation(IQueue* self, const char* name, uint32_t bgra);
    void queueSubmit(IQueue* self, const QueueSubmitInfo& submitInfo);
    void queueWaitIdle(IQueue* self);
    void queueWaitFence(IQueue* self, Fence* fence, uint64_t value);

    // Device lifetime
    Device* createDevice(const ApplicationInfo& appInfo);
    void destroyDevice(Device* device);

    // Device interface functions
    BackendType deviceGetBackend(IDevice* self);
    DeviceFeatures deviceGetFeatures(IDevice* self);
    DeviceLimits deviceGetLimits(IDevice* self);
    uint64_t deviceGarbageCollect(IDevice* self, uint64_t graphicsTimeline);
    uint64_t deviceGetNativeHandle(IDevice* self, NativeHandleType type, uint64_t handle);
    Queue* deviceGetQueue(IDevice* self, QueueType queueType);
    ShaderModule* createShaderModule(IDevice* self, const ShaderModuleCreateInfo& createInfo);
    void destroyShaderModule(IDevice* self, ShaderModule* shaderModule);
    BindGroupLayout* createBindGroupLayout(IDevice* self, const BindGroupLayoutCreateInfo& createInfo);
    void destroyBindGroupLayout(IDevice* self, BindGroupLayout* handle);
    BindGroup* createBindGroup(IDevice* self, const BindGroupCreateInfo& createInfo);
    void destroyBindGroup(IDevice* self, BindGroup* handle);
    void bindGroupUpdate(IDevice* self, BindGroup* handle, const BindGroupUpdateInfo& updateInfo);
    PipelineLayout* createPipelineLayout(IDevice* self, const PipelineLayoutCreateInfo& createInfo);
    void destroyPipelineLayout(IDevice* self, PipelineLayout* handle);
    GraphicsPipeline* createGraphicsPipeline(IDevice* self, const GraphicsPipelineCreateInfo& createInfo);
    void destroyGraphicsPipeline(IDevice* self, GraphicsPipeline* handle);
    ComputePipeline* createComputePipeline(IDevice* self, const ComputePipelineCreateInfo& createInfo);
    void destroyComputePipeline(IDevice* self, ComputePipeline* handle);
    RayTracingPipeline* createRayTracingPipeline(IDevice* self, const RayTracingPipelineCreateInfo& createInfo);
    void destroyRayTracingPipeline(IDevice* self, RayTracingPipeline* handle);
    void rayTracingPipelineGetShaderBindingTableEntry(IDevice* self, RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle);
    Buffer* createBuffer(IDevice* self, const BufferCreateInfo& createInfo);
    void destroyBuffer(IDevice* self, Buffer* handle);
    uint64_t bufferGetDeviceAddress(IDevice* self, Buffer* buffer);
    DeviceMemoryPtr bufferMap(IDevice* self, Buffer* buffer, uint64_t offset, uint64_t sizeBytes);
    void bufferUnmap(IDevice* self, Buffer* buffer);
    Image* createImage(IDevice* self, const ImageCreateInfo& createInfo);
    Image* createImageView(IDevice* self, const ImageViewCreateInfo& createInfo);
    void destroyImage(IDevice* self, Image* handle);
    Sampler* createSampler(IDevice* self, const SamplerCreateInfo& createInfo);
    void destroySampler(IDevice* self, Sampler* handle);
    AccelerationStructure* createAccelerationStructure(IDevice* self, const AccelerationStructureCreateInfo& createInfo);
    void destroyAccelerationStructure(IDevice* self, AccelerationStructure* handle);
    uint64_t accelerationStructureGetDeviceAddress(IDevice* self, AccelerationStructure* handle);
    AccelerationStructureSizesInfo accelerationStructureGetSizesInfo(IDevice* self, AccelerationStructure* as);
    Micromap* createMicromap(IDevice* self, const MicromapCreateInfo& createInfo);
    void destroyMicromap(IDevice* self, Micromap* handle);
    CommandPool* createCommandPool(IDevice* self, const CommandPoolCreateInfo& createInfo);
    void destroyCommandPool(IDevice* self, CommandPool* handle);
    Fence* createFence(IDevice* self, const FenceCreateInfo& createInfo);
    void destroyFence(IDevice* self, Fence* handle);
    uint64_t fenceGetValue(IDevice* self, Fence* handle);
    QueryPool* createQueryPool(IDevice* self, const QueryPoolCreateInfo& createInfo);
    void destroyQueryPool(IDevice* self, QueryPool* handle);
    Swapchain* createSwapchain(IDevice* self, const SwapchainCreateInfo& createInfo);
    void destroySwapchain(IDevice* self, Swapchain* swapchain);
    uint32_t swapchainGetImagesCount(IDevice* self, Swapchain* swapchain);
    SwapchainAcquireResult swapchainAcquireImage(IDevice* self, Swapchain* swapchain);
    SurfaceCapabilities swapchainGetSurfaceCapabilities(IDevice* self, Swapchain* swapchain);
    ResultCode swapchainResize(IDevice* self, Swapchain* swapchain, const ImageSize2D& size);
    ResultCode swapchainConfigure(IDevice* self, Swapchain* swapchain, const SwapchainConfigureInfo& configInfo);

} // namespace RHI::Vulkan
