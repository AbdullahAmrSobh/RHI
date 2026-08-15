#include "device.h"

#include <TL/Allocator/Allocator.hpp>
#include <TL/Containers/InlineVector.hpp>

#include <cstdint>
#include <cstdlib>

namespace RHI::WebGPU
{
    inline static WGPUStringView MakeStringView(const char* value)
    {
        return WGPUStringView{
            .data = value,
            .length = WGPU_STRLEN,
        };
    }

    struct AdapterRequest
    {
        WGPUAdapter adapter = nullptr;
    };

    struct DeviceRequest
    {
        WGPUDevice device = nullptr;
    };

    inline static void OnAdapterRequested(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void*)
    {
        auto* request = (AdapterRequest*)userdata1;
        request->adapter = adapter;
    }

    inline static void OnDeviceRequested(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* userdata1, void*)
    {
        auto* request = (DeviceRequest*)userdata1;
        request->device = device;
    }

    inline static void OnDeviceLost(WGPUDevice const*, WGPUDeviceLostReason, WGPUStringView message, void*, void*)
    {
    }

    inline static void OnUncapturedError(WGPUDevice const*, WGPUErrorType, WGPUStringView message, void*, void*)
    {
    }

    inline static void WaitForFuture(WGPUInstance instance, WGPUFuture future)
    {
        WGPUFutureWaitInfo waitInfo{
            .future = future,
            .completed = WGPU_FALSE,
        };
        wgpuInstanceWaitAny(instance, 1, &waitInfo, UINT64_MAX);
    }

    template<typename Resource, typename CreateInfo>
    inline static Resource* CreateResource(IDevice* device, const char* name, const CreateInfo& createInfo)
    {
        auto* resource = TL::constructFrom<Resource>(TL::Context::getDefaultAllocator(), TL::StringView(name));
        resource->Init(device, createInfo);
        return resource;
    }

    template<typename Resource>
    inline static void DestroyResource(IDevice* device, Resource* resource)
    {
        resource->Shutdown(device);
        TL::destructFrom(TL::Context::getDefaultAllocator(), resource);
    }

    void IQueue::Init(IDevice* device, const char* debugName)
    {
        m_queue = wgpuDeviceGetQueue(device->m_device);
        wgpuQueueSetLabel(m_queue, MakeStringView(debugName));
    }

    void IQueue::Shutdown()
    {
        wgpuQueueRelease(m_queue);
        m_queue = nullptr;
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
        TL::InlineVector<WGPUCommandBuffer, MaxSubmittedCommandLists> commandBuffers;
        for (CommandList* commandList : submitInfo.commandLists)
        {
            auto* webGPUCommandList = (ICommandList*)commandList;
            commandBuffers.push_back(webGPUCommandList->commandBuffer);
        }
        wgpuQueueSubmit(self->m_queue, commandBuffers.size(), commandBuffers.data());

        for (Swapchain* swapchain : submitInfo.presentSwapchains)
        {
            auto* webGPUSwapchain = (ISwapchain*)swapchain;
#if !defined(__EMSCRIPTEN__)
            wgpuSurfacePresent(webGPUSwapchain->surface);
#endif
        }

        for (auto signalFence : submitInfo.signalFences)
        {
            auto* fence = (IFence*)signalFence.fence;
            fence->value.store(signalFence.value, std::memory_order_release);
        }
    }

    void queueWaitIdle(IQueue* self)
    {
    }

    void queueWaitFence(IQueue* self, Fence* fence, uint64_t value)
    {
    }

    void IDevice::Init(const ApplicationInfo& appInfo)
    {
        WGPUInstanceFeatureName instanceFeature = WGPUInstanceFeatureName_TimedWaitAny;
        WGPUInstanceLimits instanceLimits{
            .nextInChain = nullptr,
            .timedWaitAnyMaxCount = 1,
        };
        WGPUInstanceDescriptor instanceDescriptor{
            .nextInChain = nullptr,
            .requiredFeatureCount = 1,
            .requiredFeatures = &instanceFeature,
            .requiredLimits = &instanceLimits,
        };
#if !defined(__EMSCRIPTEN__)
        char const* enabledToggles[]{
            "allow_unsafe_apis",
        };
        WGPUDawnTogglesDescriptor togglesDescriptor{
            .chain = WGPUChainedStruct{
                .next = nullptr,
                .sType = WGPUSType_DawnTogglesDescriptor,
            },
            .enabledToggleCount = 1,
            .enabledToggles = enabledToggles,
            .disabledToggleCount = 0,
            .disabledToggles = nullptr,
        };
        instanceDescriptor.nextInChain = &togglesDescriptor.chain;
#endif
        m_instance = wgpuCreateInstance(&instanceDescriptor);
        WGPURequestAdapterOptions adapterOptions{
            .nextInChain = nullptr,
            .featureLevel = WGPUFeatureLevel_Core,
            .powerPreference = WGPUPowerPreference_Undefined,
            .forceFallbackAdapter = WGPU_FALSE,
            .backendType = WGPUBackendType_Undefined,
            .compatibleSurface = nullptr,
        };

        AdapterRequest adapterRequest{
            .adapter = nullptr,
        };
        WGPURequestAdapterCallbackInfo adapterCallback{
            .nextInChain = nullptr,
            .mode = WGPUCallbackMode_WaitAnyOnly,
            .callback = OnAdapterRequested,
            .userdata1 = &adapterRequest,
            .userdata2 = nullptr,
        };
        WGPUFuture adapterFuture = wgpuInstanceRequestAdapter(m_instance, &adapterOptions, adapterCallback);
        WaitForFuture(m_instance, adapterFuture);
        m_adapter = adapterRequest.adapter;

        WGPUDeviceDescriptor deviceDescriptor{
            .nextInChain = nullptr,
            .label = MakeStringView(appInfo.applicationName),
            .requiredFeatureCount = 0,
            .requiredFeatures = nullptr,
            .requiredLimits = nullptr,
            .defaultQueue = WGPUQueueDescriptor{
                .nextInChain = nullptr,
                .label = MakeStringView("RHI WebGPU Queue"),
            },
            .deviceLostCallbackInfo = WGPUDeviceLostCallbackInfo{
                .nextInChain = nullptr,
                .mode = WGPUCallbackMode_AllowSpontaneous,
                .callback = OnDeviceLost,
                .userdata1 = this,
                .userdata2 = nullptr,
            },
            .uncapturedErrorCallbackInfo = WGPUUncapturedErrorCallbackInfo{
                .nextInChain = nullptr,
                .callback = OnUncapturedError,
                .userdata1 = this,
                .userdata2 = nullptr,
            },
        };

        DeviceRequest deviceRequest{
            .device = nullptr,
        };
        WGPURequestDeviceCallbackInfo deviceCallback{
            .nextInChain = nullptr,
            .mode = WGPUCallbackMode_WaitAnyOnly,
            .callback = OnDeviceRequested,
            .userdata1 = &deviceRequest,
            .userdata2 = nullptr,
        };
        WGPUFuture deviceFuture = wgpuAdapterRequestDevice(m_adapter, &deviceDescriptor, deviceCallback);
        WaitForFuture(m_instance, deviceFuture);
        m_device = deviceRequest.device;

        WGPULimits webLimits{
            .nextInChain = nullptr,
        };
        wgpuDeviceGetLimits(m_device, &webLimits);
        m_limits.minUniformBufferOffsetAlignment = webLimits.minUniformBufferOffsetAlignment;
        m_limits.minStorageBufferOffsetAlignment = webLimits.minStorageBufferOffsetAlignment;
        m_limits.maxMeshWorkGroupInvocations = webLimits.maxComputeInvocationsPerWorkgroup;
        m_limits.maxMeshWorkGroupSize[0] = webLimits.maxComputeWorkgroupSizeX;
        m_limits.maxMeshWorkGroupSize[1] = webLimits.maxComputeWorkgroupSizeY;
        m_limits.maxMeshWorkGroupSize[2] = webLimits.maxComputeWorkgroupSizeZ;

        m_queue.Init(this, "RHI WebGPU Queue");
    }

    void IDevice::Shutdown()
    {
        m_queue.Shutdown();
        wgpuDeviceDestroy(m_device);
        wgpuDeviceRelease(m_device);
        wgpuAdapterRelease(m_adapter);
        wgpuInstanceRelease(m_instance);

        m_device = nullptr;
        m_adapter = nullptr;
        m_instance = nullptr;
    }

    void IDevice::WaitIdle()
    {
    }

    Device* createDevice(const ApplicationInfo& appInfo)
    {
        auto* device = TL::constructFrom<IDevice>(TL::Context::getDefaultAllocator());
        device->Init(appInfo);
        return device;
    }

    void destroyDevice(Device* device)
    {
        auto* webGPUDevice = (IDevice*)device;
        webGPUDevice->Shutdown();
        TL::destructFrom(TL::Context::getDefaultAllocator(), webGPUDevice);
    }

    BackendType deviceGetBackend(IDevice* self)
    {
        return BackendType::WebGPU;
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
        return graphicsTimeline;
    }

    uint64_t deviceGetNativeHandle(IDevice* self, NativeHandleType type, uint64_t handle)
    {
        switch (type)
        {
        case NativeHandleType::Device: return (uint64_t)(uintptr_t)self->m_device;
        case NativeHandleType::CommandList: return (uint64_t)(uintptr_t)((ICommandList*)(CommandList*)handle)->commandBuffer;
        case NativeHandleType::Buffer: return (uint64_t)(uintptr_t)((IBuffer*)(Buffer*)handle)->handle;
        case NativeHandleType::Image: return (uint64_t)(uintptr_t)((IImage*)(Image*)handle)->handle;
        case NativeHandleType::ImageView: return (uint64_t)(uintptr_t)((IImage*)(Image*)handle)->viewHandle;
        case NativeHandleType::Sampler: return (uint64_t)(uintptr_t)((ISampler*)(Sampler*)handle)->handle;
        case NativeHandleType::ShaderModule: return (uint64_t)(uintptr_t)((IShaderModule*)(ShaderModule*)handle)->handle;
        case NativeHandleType::Pipeline: return (uint64_t)(uintptr_t)((IGraphicsPipeline*)(GraphicsPipeline*)handle)->handle;
        case NativeHandleType::PipelineLayout: return (uint64_t)(uintptr_t)((IPipelineLayout*)(PipelineLayout*)handle)->handle;
        case NativeHandleType::BindGroupLayout: return (uint64_t)(uintptr_t)((IBindGroupLayout*)(BindGroupLayout*)handle)->handle;
        case NativeHandleType::BindGroup: return (uint64_t)(uintptr_t)((IBindGroup*)(BindGroup*)handle)->handle;
        case NativeHandleType::Swapchain: return (uint64_t)(uintptr_t)((ISwapchain*)(Swapchain*)handle)->surface;
        default: std::abort();
        }
    }

    Queue* deviceGetQueue(IDevice* self, QueueType queueType)
    {
        return &self->m_queue;
    }

    ShaderModule* createShaderModule(IDevice* self, const ShaderModuleCreateInfo& createInfo)
    {
        return CreateResource<IShaderModule>(self, createInfo.name, createInfo);
    }

    void destroyShaderModule(IDevice* self, ShaderModule* resource)
    {
        DestroyResource(self, (IShaderModule*)resource);
    }

    BindGroupLayout* createBindGroupLayout(IDevice* self, const BindGroupLayoutCreateInfo& createInfo)
    {
        return CreateResource<IBindGroupLayout>(self, createInfo.name, createInfo);
    }

    void destroyBindGroupLayout(IDevice* self, BindGroupLayout* resource)
    {
        DestroyResource(self, (IBindGroupLayout*)resource);
    }

    BindGroup* createBindGroup(IDevice* self, const BindGroupCreateInfo& createInfo)
    {
        return CreateResource<IBindGroup>(self, createInfo.name, createInfo);
    }

    void destroyBindGroup(IDevice* self, BindGroup* resource)
    {
        DestroyResource(self, (IBindGroup*)resource);
    }

    void bindGroupUpdate(IDevice* self, BindGroup* handle, const BindGroupUpdateInfo& updateInfo)
    {
        ((IBindGroup*)handle)->Update(self, updateInfo);
    }

    PipelineLayout* createPipelineLayout(IDevice* self, const PipelineLayoutCreateInfo& createInfo)
    {
        return CreateResource<IPipelineLayout>(self, createInfo.name, createInfo);
    }

    void destroyPipelineLayout(IDevice* self, PipelineLayout* resource)
    {
        DestroyResource(self, (IPipelineLayout*)resource);
    }

    GraphicsPipeline* createGraphicsPipeline(IDevice* self, const GraphicsPipelineCreateInfo& createInfo)
    {
        return CreateResource<IGraphicsPipeline>(self, createInfo.name, createInfo);
    }

    void destroyGraphicsPipeline(IDevice* self, GraphicsPipeline* resource)
    {
        DestroyResource(self, (IGraphicsPipeline*)resource);
    }

    ComputePipeline* createComputePipeline(IDevice* self, const ComputePipelineCreateInfo& createInfo)
    {
        return CreateResource<IComputePipeline>(self, createInfo.name, createInfo);
    }

    void destroyComputePipeline(IDevice* self, ComputePipeline* resource)
    {
        DestroyResource(self, (IComputePipeline*)resource);
    }

    RayTracingPipeline* createRayTracingPipeline(IDevice* self, const RayTracingPipelineCreateInfo& createInfo)
    {
        return CreateResource<IRayTracingPipeline>(self, createInfo.name, createInfo);
    }

    void destroyRayTracingPipeline(IDevice* self, RayTracingPipeline* handle)
    {
        DestroyResource(self, (IRayTracingPipeline*)handle);
    }

    void rayTracingPipelineGetShaderBindingTableEntry(IDevice* self, RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle)
    {
        ((IRayTracingPipeline*)handle)->GetShaderBindingTableEntry(self, group, size, dstHandle);
    }

    Buffer* createBuffer(IDevice* self, const BufferCreateInfo& createInfo)
    {
        return CreateResource<IBuffer>(self, createInfo.name, createInfo);
    }

    void destroyBuffer(IDevice* self, Buffer* handle)
    {
        DestroyResource(self, (IBuffer*)handle);
    }

    uint64_t bufferGetDeviceAddress(IDevice* self, Buffer* buffer)
    {
        return 0;
    }

    DeviceMemoryPtr bufferMap(IDevice* self, Buffer* buffer, uint64_t offset, uint64_t sizeBytes)
    {
        auto* webGPUBuffer = (IBuffer*)buffer;
        if (sizeBytes == RemainingSize)
            sizeBytes = webGPUBuffer->size - offset;
        auto* base = (uint8_t*)webGPUBuffer->Map(self);
        return base + offset;
    }

    void bufferUnmap(IDevice* self, Buffer* buffer)
    {
        ((IBuffer*)buffer)->Unmap(self);
    }

    Image* createImage(IDevice* self, const ImageCreateInfo& createInfo)
    {
        return CreateResource<IImage>(self, createInfo.name, createInfo);
    }

    Image* createImageView(IDevice* self, const ImageViewCreateInfo& createInfo)
    {
        return CreateResource<IImage>(self, createInfo.name, createInfo);
    }

    void destroyImage(IDevice* self, Image* handle)
    {
        DestroyResource(self, (IImage*)handle);
    }

    Sampler* createSampler(IDevice* self, const SamplerCreateInfo& createInfo)
    {
        return CreateResource<ISampler>(self, createInfo.name, createInfo);
    }

    void destroySampler(IDevice* self, Sampler* handle)
    {
        DestroyResource(self, (ISampler*)handle);
    }

    AccelerationStructure* createAccelerationStructure(IDevice* self, const AccelerationStructureCreateInfo& createInfo)
    {
        return CreateResource<IAccelerationStructure>(self, createInfo.name, createInfo);
    }

    void destroyAccelerationStructure(IDevice* self, AccelerationStructure* handle)
    {
        DestroyResource(self, (IAccelerationStructure*)handle);
    }

    uint64_t accelerationStructureGetDeviceAddress(IDevice* self, AccelerationStructure* handle)
    {
        return 0;
    }

    AccelerationStructureSizesInfo accelerationStructureGetSizesInfo(IDevice* self, AccelerationStructure* as)
    {
        return {};
    }

    Micromap* createMicromap(IDevice* self, const MicromapCreateInfo& createInfo)
    {
        return CreateResource<IMicromap>(self, createInfo.name, createInfo);
    }

    void destroyMicromap(IDevice* self, Micromap* handle)
    {
        DestroyResource(self, (IMicromap*)handle);
    }

    CommandPool* createCommandPool(IDevice* self, const CommandPoolCreateInfo& createInfo)
    {
        auto* commandPool = TL::constructFrom<ICommandPool>(TL::Context::getDefaultAllocator());
        commandPool->Init(self);
        return commandPool;
    }

    void destroyCommandPool(IDevice* self, CommandPool* handle)
    {
        auto* commandPool = (ICommandPool*)handle;
        commandPool->Shutdown(self);
        TL::destructFrom(TL::Context::getDefaultAllocator(), commandPool);
    }

    Fence* createFence(IDevice* self, const FenceCreateInfo& createInfo)
    {
        return CreateResource<IFence>(self, createInfo.name, createInfo);
    }

    void destroyFence(IDevice* self, Fence* handle)
    {
        DestroyResource(self, (IFence*)handle);
    }

    uint64_t fenceGetValue(IDevice* self, Fence* handle)
    {
        return ((IFence*)handle)->value.load();
    }

    QueryPool* createQueryPool(IDevice* self, const QueryPoolCreateInfo& createInfo)
    {
        return CreateResource<IQueryPool>(self, createInfo.name, createInfo);
    }

    void destroyQueryPool(IDevice* self, QueryPool* handle)
    {
        DestroyResource(self, (IQueryPool*)handle);
    }

    Swapchain* createSwapchain(IDevice* self, const SwapchainCreateInfo& createInfo)
    {
        return CreateResource<ISwapchain>(self, createInfo.name, createInfo);
    }

    void destroySwapchain(IDevice* self, Swapchain* swapchain)
    {
        DestroyResource(self, (ISwapchain*)swapchain);
    }

    uint32_t swapchainGetImagesCount(IDevice* self, Swapchain* swapchain)
    {
        return ((ISwapchain*)swapchain)->GetImagesCount();
    }

    SwapchainAcquireResult swapchainAcquireImage(IDevice* self, Swapchain* swapchain)
    {
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
