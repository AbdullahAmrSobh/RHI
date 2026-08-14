#include "device.h"

namespace RHI::WebGPU
{
    ResultCode IQueue::Init(IDevice* device, const char* debugName, QueueType queueType)
    {
        return {};
    }

    void IQueue::Shutdown()
    {
    }

    void queueBeginAnnotation(IQueue* self, const char* name, uint32_t bgra)
    {
    }

    void queueEndAnnotation(IQueue* self)
    {
    }

    void queueInsertAnnotation(IQueue* self, const char* name, uint32_t bgra)
    {
    }

    void queueSubmit(IQueue* self, const QueueSubmitInfo& submitInfo)
    {
    }

    void queueWaitIdle(IQueue* self)
    {
    }

    void queueWaitFence(IQueue* self, Fence* fence, uint64_t value)
    {
    }

    IDevice::IDevice() = default;
    IDevice::~IDevice() = default;

    ResultCode IDevice::Init(const ApplicationInfo& appInfo)
    {
        return {};
    }

    void IDevice::Shutdown()
    {
    }

    void IDevice::WaitIdle()
    {
    }

    Device* createDevice(const ApplicationInfo& appInfo)
    {
        return {};
    }

    void destroyDevice(Device* _device)
    {
    }

    BackendType deviceGetBackend(IDevice* self)
    {
        return {};
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
        return {};
    }

    uint64_t deviceGetNativeHandle(IDevice* self, NativeHandleType type, uint64_t handle)
    {
        return {};
    }

    Queue* deviceGetQueue(IDevice* self, QueueType queueType)
    {
        return {};
    }

    ShaderModule* createShaderModule(IDevice* self, const ShaderModuleCreateInfo& createInfo)
    {
        return {};
    }

    void destroyShaderModule(IDevice* self, ShaderModule* resource)
    {
    }

    BindGroupLayout* createBindGroupLayout(IDevice* self, const BindGroupLayoutCreateInfo& createInfo)
    {
        return {};
    }

    void destroyBindGroupLayout(IDevice* self, BindGroupLayout* resource)
    {
    }

    BindGroup* createBindGroup(IDevice* self, const BindGroupCreateInfo& createInfo)
    {
        return {};
    }

    void destroyBindGroup(IDevice* self, BindGroup* resource)
    {
    }

    void bindGroupUpdate(IDevice* self, BindGroup* handle, const BindGroupUpdateInfo& updateInfo)
    {
    }

    PipelineLayout* createPipelineLayout(IDevice* self, const PipelineLayoutCreateInfo& createInfo)
    {
        return {};
    }

    void destroyPipelineLayout(IDevice* self, PipelineLayout* resource)
    {
    }

    GraphicsPipeline* createGraphicsPipeline(IDevice* self, const GraphicsPipelineCreateInfo& createInfo)
    {
        return {};
    }

    void destroyGraphicsPipeline(IDevice* self, GraphicsPipeline* resource)
    {
    }

    ComputePipeline* createComputePipeline(IDevice* self, const ComputePipelineCreateInfo& createInfo)
    {
        return {};
    }

    void destroyComputePipeline(IDevice* self, ComputePipeline* resource)
    {
    }

    RayTracingPipeline* createRayTracingPipeline(IDevice* self, const RayTracingPipelineCreateInfo& createInfo)
    {
        return {};
    }

    void destroyRayTracingPipeline(IDevice* self, RayTracingPipeline* handle)
    {
    }

    void rayTracingPipelineGetShaderBindingTableEntry(IDevice* self, RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle)
    {
    }

    Buffer* createBuffer(IDevice* self, const BufferCreateInfo& createInfo)
    {
        return {};
    }

    void destroyBuffer(IDevice* self, Buffer* handle)
    {
    }

    uint64_t bufferGetDeviceAddress(IDevice* self, Buffer* buffer)
    {
        return {};
    }

    DeviceMemoryPtr bufferMap(IDevice* self, Buffer* buffer, uint64_t offset, uint64_t sizeBytes)
    {
        return {};
    }

    void bufferUnmap(IDevice* self, Buffer* buffer)
    {
    }

    Image* createImage(IDevice* self, const ImageCreateInfo& createInfo)
    {
        return {};
    }

    Image* createImageView(IDevice* self, const ImageViewCreateInfo& createInfo)
    {
        return {};
    }

    void destroyImage(IDevice* self, Image* handle)
    {
    }

    Sampler* createSampler(IDevice* self, const SamplerCreateInfo& createInfo)
    {
        return {};
    }

    void destroySampler(IDevice* self, Sampler* handle)
    {
    }

    AccelerationStructure* createAccelerationStructure(IDevice* self, const AccelerationStructureCreateInfo& createInfo)
    {
        return {};
    }

    void destroyAccelerationStructure(IDevice* self, AccelerationStructure* handle)
    {
    }

    uint64_t accelerationStructureGetDeviceAddress(IDevice* self, AccelerationStructure* handle)
    {
        return {};
    }

    AccelerationStructureSizesInfo accelerationStructureGetSizesInfo(IDevice* self, AccelerationStructure* as)
    {
        return {};
    }

    Micromap* createMicromap(IDevice* self, const MicromapCreateInfo& createInfo)
    {
        return {};
    }

    void destroyMicromap(IDevice* self, Micromap* handle)
    {
    }

    CommandPool* createCommandPool(IDevice* self, const CommandPoolCreateInfo& createInfo)
    {
        return {};
    }

    void destroyCommandPool(IDevice* self, CommandPool* handle)
    {
    }

    Fence* createFence(IDevice* self, const FenceCreateInfo& createInfo)
    {
        return {};
    }

    void destroyFence(IDevice* self, Fence* handle)
    {
    }

    uint64_t fenceGetValue(IDevice* self, Fence* handle)
    {
        return {};
    }

    QueryPool* createQueryPool(IDevice* self, const QueryPoolCreateInfo& createInfo)
    {
        return {};
    }

    void destroyQueryPool(IDevice* self, QueryPool* handle)
    {
    }

    Swapchain* createSwapchain(IDevice* self, const SwapchainCreateInfo& createInfo)
    {
        return {};
    }

    void destroySwapchain(IDevice* self, Swapchain* swapchain)
    {
    }

    uint32_t swapchainGetImagesCount(IDevice* self, Swapchain* swapchain)
    {
        return {};
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
        return {};
    }

    ResultCode swapchainConfigure(IDevice* self, Swapchain* swapchain, const SwapchainConfigureInfo& configInfo)
    {
        return {};
    }
} // namespace RHI::WebGPU
