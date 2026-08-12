#pragma once

#include <RHI/RHI.h>

#include <TL/Allocator/Arena.hpp>
#include <TL/Containers/Vector.hpp>

#include <webgpu/webgpu.h>

#include "CommandList.hpp"
#include "Common.hpp"
#include "Resources.hpp"

namespace RHI::WebGPU
{
    struct IQueue : RHI::Queue
    {
        IDevice* m_device = nullptr;
        WGPUQueue m_queue = nullptr;
        QueueType m_queueType = QueueType::Graphics;
        std::atomic_uint64_t m_lastSubmitValue = 0;

        ResultCode Init(IDevice* device, const char* debugName, QueueType queueType);
        void Shutdown();
    };

    struct IDevice : RHI::Device
    {
        IDevice();
        ~IDevice();

        ResultCode Init(const ApplicationInfo& appInfo);
        void Shutdown();

        void WaitIdle();

        // Internal accessor returning backend queue state directly (facade-free).
        IQueue* getQueue(QueueType queueType) { return &m_queue[(uint32_t)queueType]; }

        BackendType m_backend = BackendType::WebGPU;
        DeviceLimits m_limits = {};
        DeviceFeatures m_features = {};

        // WebGPU instance and core objects
        WGPUInstance m_instance = nullptr; ///< WGPU instance handle.
        WGPUAdapter m_adapter = nullptr;   ///< Physical device selected for use.
        WGPUDevice m_device = nullptr;     ///< Logical device handle.

        IQueue m_queue[(uint32_t)QueueType::Count] = {};

        TL::Arena m_arena;
        TL::IAllocator* m_objectAllocator = nullptr;
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
} // namespace RHI::WebGPU
