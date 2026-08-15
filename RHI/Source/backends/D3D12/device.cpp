#include <backends/D3D12/device.h>

namespace RHI::D3D12
{
    void IQueue::Init(IDevice* device, const char* debugName, QueueType queueType) {}

    void IQueue::Shutdown() {}

    void IDevice::Init(const ApplicationInfo& appInfo) {}

    void IDevice::Shutdown() {}

    void IDevice::WaitIdle() {}

    void queueBeginAnnotation(IQueue* self, const char* name, uint32_t bgra) {}

    void queueEndAnnotation(IQueue* self) {}

    void queueInsertAnnotation(IQueue* self, const char* name, uint32_t bgra) {}

    void queueSubmit(IQueue* self, const QueueSubmitInfo& submitInfo) {}

    void queueWaitIdle(IQueue* self) {}

    void queueWaitFence(IQueue* self, Fence* fence, uint64_t value) {}

    Device* createDevice(const ApplicationInfo& appInfo)
    {
        return nullptr;
    }

    void destroyDevice(Device* device) {}

    BackendType deviceGetBackend(IDevice* self)
    {
        return BackendType::DirectX12_2;
    }

    DeviceFeatures deviceGetFeatures(IDevice* self)
    {
        return {};
    }

    DeviceLimits deviceGetLimits(IDevice* self)
    {
        return {};
    }

    uint64_t deviceGarbageCollect(IDevice* self, uint64_t graphicsTimeline)
    {
        return 0;
    }

    uint64_t deviceGetNativeHandle(IDevice* self, NativeHandleType type, uint64_t handle)
    {
        return 0;
    }

    Queue* deviceGetQueue(IDevice* self, QueueType queueType)
    {
        return nullptr;
    }

    ShaderModule* createShaderModule(IDevice* self, const ShaderModuleCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyShaderModule(IDevice* self, ShaderModule* shaderModule) {}

    BindGroupLayout* createBindGroupLayout(IDevice* self, const BindGroupLayoutCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyBindGroupLayout(IDevice* self, BindGroupLayout* handle) {}

    BindGroup* createBindGroup(IDevice* self, const BindGroupCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyBindGroup(IDevice* self, BindGroup* handle) {}

    void bindGroupUpdate(IDevice* self, BindGroup* handle, const BindGroupUpdateInfo& updateInfo) {}

    PipelineLayout* createPipelineLayout(IDevice* self, const PipelineLayoutCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyPipelineLayout(IDevice* self, PipelineLayout* handle) {}

    GraphicsPipeline* createGraphicsPipeline(IDevice* self, const GraphicsPipelineCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyGraphicsPipeline(IDevice* self, GraphicsPipeline* handle) {}

    ComputePipeline* createComputePipeline(IDevice* self, const ComputePipelineCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyComputePipeline(IDevice* self, ComputePipeline* handle) {}

    RayTracingPipeline* createRayTracingPipeline(IDevice* self, const RayTracingPipelineCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyRayTracingPipeline(IDevice* self, RayTracingPipeline* handle) {}

    void rayTracingPipelineGetShaderBindingTableEntry(IDevice* self, RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle) {}

    Buffer* createBuffer(IDevice* self, const BufferCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyBuffer(IDevice* self, Buffer* handle) {}

    uint64_t bufferGetDeviceAddress(IDevice* self, Buffer* buffer)
    {
        return 0;
    }

    DeviceMemoryPtr bufferMap(IDevice* self, Buffer* buffer, uint64_t offset, uint64_t sizeBytes)
    {
        return nullptr;
    }

    void bufferUnmap(IDevice* self, Buffer* buffer) {}

    Image* createImage(IDevice* self, const ImageCreateInfo& createInfo)
    {
        return nullptr;
    }

    Image* createImageView(IDevice* self, const ImageViewCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyImage(IDevice* self, Image* handle) {}

    Sampler* createSampler(IDevice* self, const SamplerCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroySampler(IDevice* self, Sampler* handle) {}

    AccelerationStructure* createAccelerationStructure(IDevice* self, const AccelerationStructureCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyAccelerationStructure(IDevice* self, AccelerationStructure* handle) {}

    uint64_t accelerationStructureGetDeviceAddress(IDevice* self, AccelerationStructure* handle)
    {
        return 0;
    }

    AccelerationStructureSizesInfo accelerationStructureGetSizesInfo(IDevice* self, AccelerationStructure* accelerationStructure)
    {
        return {};
    }

    Micromap* createMicromap(IDevice* self, const MicromapCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyMicromap(IDevice* self, Micromap* handle) {}

    CommandPool* createCommandPool(IDevice* self, const CommandPoolCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyCommandPool(IDevice* self, CommandPool* handle) {}

    Fence* createFence(IDevice* self, const FenceCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyFence(IDevice* self, Fence* handle) {}

    uint64_t fenceGetValue(IDevice* self, Fence* handle)
    {
        return 0;
    }

    QueryPool* createQueryPool(IDevice* self, const QueryPoolCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroyQueryPool(IDevice* self, QueryPool* handle) {}

    Swapchain* createSwapchain(IDevice* self, const SwapchainCreateInfo& createInfo)
    {
        return nullptr;
    }

    void destroySwapchain(IDevice* self, Swapchain* swapchain) {}

    uint32_t swapchainGetImagesCount(IDevice* self, Swapchain* swapchain)
    {
        return 0;
    }

    SwapchainAcquireResult swapchainAcquireImage(IDevice* self, Swapchain* swapchain)
    {
        return {};
    }

    SurfaceCapabilities swapchainGetSurfaceCapabilities(IDevice* self, Swapchain* swapchain)
    {
        return {};
    }

    ResultCode swapchainResize(IDevice* self, Swapchain* swapchain, const ImageSize2D& size)
    {
        return ResultCode::ErrorUnknown;
    }

    ResultCode swapchainConfigure(IDevice* self, Swapchain* swapchain, const SwapchainConfigureInfo& configInfo)
    {
        return ResultCode::ErrorUnknown;
    }
} // namespace RHI::D3D12
