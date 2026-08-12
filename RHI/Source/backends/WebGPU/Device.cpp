#include "Device.hpp"
#include "Common.hpp"

#include <TL/Context.hpp>
#include <TL/Log.hpp>

namespace RHI::WebGPU
{
    template<typename Resource, typename... Args>
    inline Resource* createImpl(IDevice* device, const char* debugName, Args... args)
    {
        Resource* resource = TL::constructFrom<Resource>(device->m_objectAllocator, debugName ? TL::StringView(debugName) : TL::StringView{});
        ResultCode result = resource->Init(device, args...);
        if (IsSuccess(result))
        {
            return resource;
        }
        TL_UNREACHABLE();
        return nullptr;
    }

    template<typename Resource>
    inline void destroyImpl(IDevice* device, Resource* resource)
    {
        resource->Shutdown(device);
        TL::destructFrom(device->m_objectAllocator, resource);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IQueue
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ResultCode IQueue::Init(IDevice* device, const char* debugName, QueueType queueType)
    {
        (void)debugName;
        m_device = device;
        m_queueType = queueType;
        // TODO: wgpuDeviceGetQueue (WebGPU exposes a single queue shared across all RHI queue types)
        return ResultCode::Success;
    }

    void IQueue::Shutdown()
    {
        // TODO: wgpuQueueRelease
    }

    void queueBeginAnnotation(IQueue* self, const char* name, uint32_t bgra)
    {
        (void)self;
        (void)name;
        (void)bgra;
        // TODO: WebGPU has no queue-level debug annotation API.
    }

    void queueEndAnnotation(IQueue* self)
    {
        (void)self;
    }

    void queueInsertAnnotation(IQueue* self, const char* name, uint32_t bgra)
    {
        (void)self;
        (void)name;
        (void)bgra;
    }

    void queueSubmit(IQueue* self, const QueueSubmitInfo& submitInfo)
    {
        (void)self;
        (void)submitInfo;
        // TODO: wgpuQueueSubmit, then wgpuSurfacePresent for each presentSwapchains entry.
    }

    void queueWaitIdle(IQueue* self)
    {
        (void)self;
        // TODO: no host-side fence/timeline tracking yet.
    }

    void queueWaitFence(IQueue* self, Fence* fence, uint64_t value)
    {
        (void)self;
        (void)fence;
        (void)value;
    }

    ///

    IDevice::IDevice()
    {
        m_objectAllocator = TL::Context::getDefaultAllocator();
    }

    IDevice::~IDevice() = default;

    ResultCode IDevice::Init(const ApplicationInfo& appInfo)
    {
        (void)appInfo;

        m_backend = BackendType::WebGPU;

        // TODO: wgpuCreateInstance, wgpuInstanceRequestAdapter, wgpuAdapterRequestDevice.

        for (uint32_t i = 0; i < (uint32_t)QueueType::Count; i++)
        {
            ResultCode result = m_queue[i].Init(this, nullptr, (QueueType)i);
            if (IsError(result))
                return result;
        }

        return ResultCode::Success;
    }

    void IDevice::WaitIdle()
    {
        // TODO: no host-side fence/timeline tracking yet.
    }

    void IDevice::Shutdown()
    {
        for (uint32_t i = 0; i < (uint32_t)QueueType::Count; i++)
            m_queue[i].Shutdown();

        // TODO: wgpuDeviceRelease, wgpuAdapterRelease, wgpuInstanceRelease
    }

    Device* createDevice(const ApplicationInfo& appInfo)
    {
        auto device = TL::constructFrom<IDevice>(TL::Context::getDefaultAllocator());
        auto result = device->Init(appInfo);
        TL_ASSERT(IsSuccess(result));
        return device;
    }

    void destroyDevice(Device* _device)
    {
        auto device = (IDevice*)_device;
        device->Shutdown();
        TL::destructFrom(TL::Context::getDefaultAllocator(), device);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// IDevice interface implementation
    //////////////////////////////////////////////////////////////////////////////////////////

    BackendType deviceGetBackend(IDevice* self)
    {
        return self->m_backend;
    }

    DeviceFeatures deviceGetFeatures(IDevice* self)
    {
        return self->m_features;
    }

    DeviceLimits deviceGetLimits(IDevice* self)
    {
        return self->m_limits;
    }

    uint64_t deviceGarbageCollect(IDevice* self, uint64_t graphicsTimeline)
    {
        self->m_arena.reset();
        // TODO: no deferred-deletion queue yet; resources are released immediately on destroy.
        return graphicsTimeline;
    }

    uint64_t deviceGetNativeHandle(IDevice* self, NativeHandleType type, uint64_t handle)
    {
        (void)self;
        switch (type)
        {
        case NativeHandleType::None: return 0;
        case NativeHandleType::Device: return (uint64_t)self->m_device;
        case NativeHandleType::Buffer: return (uint64_t)((IBuffer*)handle)->handle;
        case NativeHandleType::Image: return (uint64_t)((IImage*)handle)->handle;
        case NativeHandleType::ImageView: return (uint64_t)((IImage*)handle)->viewHandle;
        case NativeHandleType::Sampler: return (uint64_t)((ISampler*)handle)->handle;
        case NativeHandleType::ShaderModule: return (uint64_t)((IShaderModule*)handle)->handle;
        case NativeHandleType::Pipeline: return (uint64_t)((IGraphicsPipeline*)handle)->handle;
        case NativeHandleType::PipelineLayout: return (uint64_t)((IPipelineLayout*)handle)->handle;
        case NativeHandleType::BindGroupLayout: return (uint64_t)((IBindGroupLayout*)handle)->handle;
        case NativeHandleType::BindGroup: return (uint64_t)((IBindGroup*)handle)->handle;
        case NativeHandleType::Swapchain: return (uint64_t)((ISwapchain*)handle)->m_surface;
        default: TL_UNREACHABLE_MSG("Unknown NativeHandleType");
        }
        return 0;
    }

    Queue* deviceGetQueue(IDevice* self, QueueType queueType)
    {
        return &self->m_queue[(int)queueType];
    }

    ShaderModule* createShaderModule(IDevice* self, const ShaderModuleCreateInfo& createInfo)
    {
        return createImpl<IShaderModule>(self, createInfo.name, createInfo);
    }

    void destroyShaderModule(IDevice* self, ShaderModule* resource)
    {
        destroyImpl<IShaderModule>(self, (IShaderModule*)resource);
    }

    BindGroupLayout* createBindGroupLayout(IDevice* self, const BindGroupLayoutCreateInfo& createInfo)
    {
        return createImpl<IBindGroupLayout>(self, createInfo.name, createInfo);
    }

    void destroyBindGroupLayout(IDevice* self, BindGroupLayout* resource)
    {
        destroyImpl<IBindGroupLayout>(self, (IBindGroupLayout*)resource);
    }

    BindGroup* createBindGroup(IDevice* self, const BindGroupCreateInfo& createInfo)
    {
        return createImpl<IBindGroup>(self, createInfo.name, createInfo);
    }

    void destroyBindGroup(IDevice* self, BindGroup* resource)
    {
        destroyImpl<IBindGroup>(self, (IBindGroup*)resource);
    }

    void bindGroupUpdate(IDevice* self, BindGroup* handle, const BindGroupUpdateInfo& updateInfo)
    {
        auto bindGroup = (IBindGroup*)(handle);
        bindGroup->Update(self, updateInfo);
    }

    PipelineLayout* createPipelineLayout(IDevice* self, const PipelineLayoutCreateInfo& createInfo)
    {
        return createImpl<IPipelineLayout>(self, createInfo.name, createInfo);
    }

    void destroyPipelineLayout(IDevice* self, PipelineLayout* resource)
    {
        destroyImpl<IPipelineLayout>(self, (IPipelineLayout*)resource);
    }

    GraphicsPipeline* createGraphicsPipeline(IDevice* self, const GraphicsPipelineCreateInfo& createInfo)
    {
        return createImpl<IGraphicsPipeline>(self, createInfo.name, createInfo);
    }

    void destroyGraphicsPipeline(IDevice* self, GraphicsPipeline* resource)
    {
        destroyImpl<IGraphicsPipeline>(self, (IGraphicsPipeline*)resource);
    }

    ComputePipeline* createComputePipeline(IDevice* self, const ComputePipelineCreateInfo& createInfo)
    {
        return createImpl<IComputePipeline>(self, createInfo.name, createInfo);
    }

    void destroyComputePipeline(IDevice* self, ComputePipeline* resource)
    {
        destroyImpl<IComputePipeline>(self, (IComputePipeline*)resource);
    }

    RayTracingPipeline* createRayTracingPipeline(IDevice* self, const RayTracingPipelineCreateInfo& createInfo)
    {
        (void)self;
        (void)createInfo;
        // Unsupported on WebGPU.
        return nullptr;
    }

    void destroyRayTracingPipeline(IDevice* self, RayTracingPipeline* handle)
    {
        (void)self;
        (void)handle;
    }

    void rayTracingPipelineGetShaderBindingTableEntry(IDevice* self, RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle)
    {
        (void)self;
        (void)handle;
        (void)group;
        (void)size;
        (void)dstHandle;
    }

    Buffer* createBuffer(IDevice* self, const BufferCreateInfo& createInfo)
    {
        return createImpl<IBuffer>(self, createInfo.name, createInfo);
    }

    void destroyBuffer(IDevice* self, Buffer* handle)
    {
        destroyImpl<IBuffer>(self, (IBuffer*)handle);
    }

    uint64_t bufferGetDeviceAddress(IDevice* self, Buffer* buffer)
    {
        (void)self;
        (void)buffer;
        // WebGPU does not expose buffer device addresses.
        return 0;
    }

    DeviceMemoryPtr bufferMap(IDevice* self, Buffer* buffer, uint64_t offset, uint64_t sizeBytes)
    {
        (void)sizeBytes;
        auto* ptr = (char*)((IBuffer*)buffer)->Map(self);
        return ptr ? ptr + offset : nullptr;
    }

    void bufferUnmap(IDevice* self, Buffer* buffer)
    {
        ((IBuffer*)buffer)->Unmap(self);
    }

    Image* createImage(IDevice* self, const ImageCreateInfo& createInfo)
    {
        return createImpl<IImage>(self, createInfo.name, createInfo);
    }

    Image* createImageView(IDevice* self, const ImageViewCreateInfo& createInfo)
    {
        return createImpl<IImage>(self, createInfo.name, createInfo);
    }

    void destroyImage(IDevice* self, Image* handle)
    {
        destroyImpl<IImage>(self, (IImage*)handle);
    }

    Sampler* createSampler(IDevice* self, const SamplerCreateInfo& createInfo)
    {
        return createImpl<ISampler>(self, createInfo.name, createInfo);
    }

    void destroySampler(IDevice* self, Sampler* handle)
    {
        destroyImpl<ISampler>(self, (ISampler*)handle);
    }

    AccelerationStructure* createAccelerationStructure(IDevice* self, const AccelerationStructureCreateInfo& createInfo)
    {
        (void)self;
        (void)createInfo;
        // Unsupported on WebGPU.
        return nullptr;
    }

    void destroyAccelerationStructure(IDevice* self, AccelerationStructure* handle)
    {
        (void)self;
        (void)handle;
    }

    uint64_t accelerationStructureGetDeviceAddress(IDevice* self, AccelerationStructure* handle)
    {
        (void)self;
        (void)handle;
        return 0;
    }

    AccelerationStructureSizesInfo accelerationStructureGetSizesInfo(IDevice* self, AccelerationStructure* as)
    {
        (void)self;
        (void)as;
        return {};
    }

    Micromap* createMicromap(IDevice* self, const MicromapCreateInfo& createInfo)
    {
        (void)self;
        (void)createInfo;
        // Unsupported on WebGPU.
        return nullptr;
    }

    void destroyMicromap(IDevice* self, Micromap* handle)
    {
        (void)self;
        (void)handle;
    }

    CommandPool* createCommandPool(IDevice* self, const CommandPoolCreateInfo& createInfo)
    {
        return createImpl<ICommandPool>(self, createInfo.name, createInfo);
    }

    void destroyCommandPool(IDevice* self, CommandPool* handle)
    {
        destroyImpl<ICommandPool>(self, (ICommandPool*)handle);
    }

    Fence* createFence(IDevice* self, const FenceCreateInfo& createInfo)
    {
        return createImpl<IFence>(self, createInfo.name, createInfo);
    }

    void destroyFence(IDevice* self, Fence* handle)
    {
        destroyImpl<IFence>(self, (IFence*)handle);
    }

    uint64_t fenceGetValue(IDevice* self, Fence* handle)
    {
        (void)self;
        // No timeline tracking; report the fence's stored value.
        return ((IFence*)handle)->value;
    }

    QueryPool* createQueryPool(IDevice* self, const QueryPoolCreateInfo& createInfo)
    {
        return createImpl<IQueryPool>(self, createInfo.name, createInfo);
    }

    void destroyQueryPool(IDevice* self, QueryPool* handle)
    {
        destroyImpl<IQueryPool>(self, (IQueryPool*)handle);
    }

    Swapchain* createSwapchain(IDevice* self, const SwapchainCreateInfo& createInfo)
    {
        return createImpl<ISwapchain>(self, createInfo.name, createInfo);
    }

    void destroySwapchain(IDevice* self, Swapchain* swapchain)
    {
        destroyImpl<ISwapchain>(self, (ISwapchain*)swapchain);
    }

    uint32_t swapchainGetImagesCount(IDevice* self, Swapchain* swapchain)
    {
        (void)self;
        return ((ISwapchain*)swapchain)->GetImagesCount();
    }

    SwapchainAcquireResult swapchainAcquireImage(IDevice* self, Swapchain* swapchain)
    {
        (void)self;
        return ((ISwapchain*)swapchain)->AcquireSwapchainImage();
    }

    SurfaceCapabilities swapchainGetSurfaceCapabilities(IDevice* self, Swapchain* swapchain)
    {
        return ((ISwapchain*)swapchain)->GetSurfaceCapabilities(self);
    }

    ResultCode swapchainResize(IDevice* self, Swapchain* swapchain, const ImageSize2D& size)
    {
        return ((ISwapchain*)swapchain)->ResizeSwapchain(self, size);
    }

    ResultCode swapchainConfigure(IDevice* self, Swapchain* swapchain, const SwapchainConfigureInfo& configInfo)
    {
        return ((ISwapchain*)swapchain)->ConfigureSwapchain(self, configInfo);
    }
} // namespace RHI::WebGPU
