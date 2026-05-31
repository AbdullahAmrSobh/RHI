#include "Device.hpp"

#include <TL/Assert.hpp>
#include <TL/Log.hpp>
#include <TL/Memory.hpp>

namespace RHI
{
    Device* CreateWebGPUDevice()
    {
        auto device = TL::construct<RHI::WebGPU::IDevice>();
        auto result = device->Init();
        TL_ASSERT(IsSuccess(result));
        return device;
    }

    void DestroyWebGPUDevice(Device* _device)
    {
        auto device = (RHI::WebGPU::IDevice*)_device;
        device->Shutdown();
        TL::destruct(device);
    }
} // namespace RHI

namespace RHI::WebGPU
{
    // Generic create/destroy mirroring Vulkan/Source/Device.cpp:902.
    template<typename Resource, typename... Args>
    inline Resource* createImpl(IDevice* device, Args... args)
    {
        Resource*  resource = TL::construct<Resource>();
        ResultCode result   = resource->Init(device, args...);
        if (IsSuccess(result))
            return resource;
        resource->Shutdown(device);
        TL::destruct(resource);
        return nullptr;
    }

    template<typename Resource>
    inline void destroyImpl(IDevice* device, Resource* resource)
    {
        resource->Shutdown(device);
        TL::destruct(resource);
    }

    ///////////////////////////////////////////////////////////
    // IQueue
    ///////////////////////////////////////////////////////////

    ResultCode IQueue::Init(IDevice* device, QueueType queueType)
    {
        m_device    = device;
        m_queueType = queueType;
        m_queue     = wgpuDeviceGetQueue(device->m_device);
        return m_queue ? ResultCode::Success : ResultCode::ErrorUnknown;
    }

    void IQueue::Shutdown()
    {
        if (m_queue)
        {
            wgpuQueueRelease(m_queue);
            m_queue = nullptr;
        }
    }

    void IQueue::BeginAnnotation(const char* name, uint32_t bgra)
    {
        (void)name;
        (void)bgra;
    }

    void IQueue::EndAnnotation()
    {
    }

    void IQueue::InsertAnnotation(const char* name, uint32_t bgra)
    {
        (void)name;
        (void)bgra;
    }

    void IQueue::Submit(const QueueSubmitInfo& submitInfo)
    {
        // Fences are no-ops for now; WebGPU's single queue orders submissions implicitly.
        TL::Vector<WGPUCommandBuffer> commandBuffers;
        commandBuffers.reserve(submitInfo.commandLists.size());
        for (auto* commandList : submitInfo.commandLists)
        {
            auto* cl = (ICommandList*)commandList;
            if (cl->m_commandBuffer)
                commandBuffers.push_back(cl->m_commandBuffer);
        }

        if (!commandBuffers.empty())
            wgpuQueueSubmit(m_queue, commandBuffers.size(), commandBuffers.data());

        for (auto* swapchain : submitInfo.presentSwapchains)
            ((ISwapchain*)swapchain)->Present();
    }

    void IQueue::WaitIdle()
    {
        // No-op: no host-side fence/timeline tracking yet.
    }

    void IQueue::WaitFence(Fence* fence, uint64_t value)
    {
        (void)fence;
        (void)value;
    }

    ///////////////////////////////////////////////////////////
    // IDevice
    ///////////////////////////////////////////////////////////

    IDevice::IDevice()
    {
        m_backend = BackendType::WebGPU;
    }

    IDevice::~IDevice() = default;

    ResultCode IDevice::Init()
    {
        m_backend = BackendType::WebGPU;

        WGPUInstanceDescriptor instanceDesc = {};
        m_instance                          = wgpuCreateInstance(&instanceDesc);
        if (!m_instance)
        {
            TL::LogError("WebGPU: wgpuCreateInstance failed");
            return ResultCode::ErrorUnknown;
        }

        // Request adapter (block on the legacy callback by pumping events).
        struct AdapterRequest
        {
            WGPUAdapter adapter = nullptr;
            bool        done    = false;
        } adapterRequest;

        WGPURequestAdapterOptions adapterOptions = {};
        adapterOptions.powerPreference           = WGPUPowerPreference_HighPerformance;

        wgpuInstanceRequestAdapter(
            m_instance,
            &adapterOptions,
            [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* userdata)
            {
                auto* request = (AdapterRequest*)userdata;
                if (status == WGPURequestAdapterStatus_Success)
                    request->adapter = adapter;
                else
                    TL::LogError("WebGPU: failed to acquire adapter: {}", message ? message : "");
                request->done = true;
            },
            &adapterRequest);

        while (!adapterRequest.done)
            wgpuInstanceProcessEvents(m_instance);

        if (!adapterRequest.adapter)
            return ResultCode::ErrorUnknown;
        m_adapter = adapterRequest.adapter;

        // Request device.
        WGPUDeviceDescriptor deviceDesc            = {};
        deviceDesc.label                           = "RHI WebGPU Device";
        deviceDesc.uncapturedErrorCallbackInfo.callback =
            [](WGPUErrorType type, char const* message, void*)
        {
            TL::LogError("WebGPU uncaptured error ({}): {}", (int)type, message ? message : "");
        };

        struct DeviceRequest
        {
            WGPUDevice device = nullptr;
            bool       done   = false;
        } deviceRequest;

        wgpuAdapterRequestDevice(
            m_adapter,
            &deviceDesc,
            [](WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata)
            {
                auto* request = (DeviceRequest*)userdata;
                if (status == WGPURequestDeviceStatus_Success)
                    request->device = device;
                else
                    TL::LogError("WebGPU: failed to acquire device: {}", message ? message : "");
                request->done = true;
            },
            &deviceRequest);

        while (!deviceRequest.done)
            wgpuInstanceProcessEvents(m_instance);

        if (!deviceRequest.device)
            return ResultCode::ErrorUnknown;
        m_device = deviceRequest.device;

        // Device limits (WebGPU exposes no mesh/ray-tracing limits).
        WGPUSupportedLimits supportedLimits = {};
        if (wgpuAdapterGetLimits(m_adapter, &supportedLimits) == WGPUStatus_Success)
        {
            m_limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
            m_limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
        }

        // Single WebGPU queue shared across all RHI queue types.
        for (uint32_t i = 0; i < (uint32_t)QueueType::Count; i++)
        {
            ResultCode result = m_queue[i].Init(this, (QueueType)i);
            if (IsError(result))
                return result;
        }

        return ResultCode::Success;
    }

    void IDevice::Shutdown()
    {
        for (uint32_t i = 0; i < (uint32_t)QueueType::Count; i++)
            m_queue[i].Shutdown();
        if (m_device)
        {
            wgpuDeviceRelease(m_device);
            m_device = nullptr;
        }
        if (m_adapter)
        {
            wgpuAdapterRelease(m_adapter);
            m_adapter = nullptr;
        }
        if (m_instance)
        {
            wgpuInstanceRelease(m_instance);
            m_instance = nullptr;
        }
    }

    uint64_t IDevice::GarbageCollect(uint64_t graphicsTimeline)
    {
        // No deferred-deletion queue yet; resources are released immediately on Destroy*.
        return graphicsTimeline;
    }

    uint64_t IDevice::GetNativeHandle(NativeHandleType type, uint64_t handle)
    {
        switch (type)
        {
        case NativeHandleType::Device: return (uint64_t)m_device;
        case NativeHandleType::Buffer: return (uint64_t)((IBuffer*)handle)->buffer;
        case NativeHandleType::Image:  return (uint64_t)((IImage*)handle)->texture;
        default:                       return 0;
        }
    }

    Queue* IDevice::GetQueue(QueueType queueType)
    {
        return &m_queue[(uint32_t)queueType];
    }

    ShaderModule* IDevice::CreateShaderModule(const ShaderModuleCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyShaderModule(ShaderModule* shaderModule)
    {
        (void)shaderModule;
    }

    BindGroupLayout* IDevice::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyBindGroupLayout(BindGroupLayout* handle)
    {
        (void)handle;
    }

    BindGroup* IDevice::CreateBindGroup(const BindGroupCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyBindGroup(BindGroup* handle)
    {
        (void)handle;
    }

    void IDevice::UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo)
    {
        (void)handle;
        (void)updateInfo;
    }

    PipelineLayout* IDevice::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyPipelineLayout(PipelineLayout* handle)
    {
        (void)handle;
    }

    GraphicsPipeline* IDevice::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyGraphicsPipeline(GraphicsPipeline* handle)
    {
        (void)handle;
    }

    ComputePipeline* IDevice::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyComputePipeline(ComputePipeline* handle)
    {
        (void)handle;
    }

    RayTracingPipeline* IDevice::CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyRayTracingPipeline(RayTracingPipeline* handle)
    {
        (void)handle;
    }

    void IDevice::GetShaderBindingTableEntry(RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle)
    {
        (void)handle;
        (void)group;
        (void)size;
        (void)dstHandle;
    }

    Buffer* IDevice::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        return createImpl<IBuffer>(this, createInfo);
    }

    void IDevice::DestroyBuffer(Buffer* handle)
    {
        destroyImpl<IBuffer>(this, (IBuffer*)handle);
    }

    uint64_t IDevice::GetBufferDeviceAddress(Buffer* buffer)
    {
        // WebGPU does not expose buffer device addresses.
        (void)buffer;
        return 0;
    }

    DeviceMemoryPtr IDevice::MapBuffer(Buffer* buffer, uint64_t offset, uint64_t sizeBytes)
    {
        (void)sizeBytes;
        auto* ptr = (char*)((IBuffer*)buffer)->Map(this);
        return ptr ? ptr + offset : nullptr;
    }

    void IDevice::UnmapBuffer(Buffer* buffer)
    {
        ((IBuffer*)buffer)->Unmap(this);
    }

    Image* IDevice::CreateImage(const ImageCreateInfo& createInfo)
    {
        return createImpl<IImage>(this, createInfo);
    }

    Image* IDevice::CreateImageView(const ImageViewCreateInfo& createInfo)
    {
        return createImpl<IImage>(this, createInfo);
    }

    void IDevice::DestroyImage(Image* handle)
    {
        destroyImpl<IImage>(this, (IImage*)handle);
    }

    Sampler* IDevice::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        return createImpl<ISampler>(this, createInfo);
    }

    void IDevice::DestroySampler(Sampler* handle)
    {
        destroyImpl<ISampler>(this, (ISampler*)handle);
    }

    AccelerationStructure* IDevice::CreateAccelerationStructure(const AccelerationStructureCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyAccelerationStructure(AccelerationStructure* handle)
    {
        (void)handle;
    }

    uint64_t IDevice::GetAccelerationStructureDeviceAddress(AccelerationStructure* handle)
    {
        (void)handle;
        return 0;
    }

    AccelerationStructureSizesInfo IDevice::GetAccelerationStructureSizesInfo(AccelerationStructure* handle)
    {
        (void)handle;
        return {};
    }

    Micromap* IDevice::CreateMicromap(const MicromapCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyMicromap(Micromap* handle)
    {
        (void)handle;
    }

    CommandPool* IDevice::CreateCommandPool(const CommandPoolCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyCommandPool(CommandPool* handle)
    {
        (void)handle;
    }

    Fence* IDevice::CreateFence(const FenceCreateInfo& createInfo)
    {
        return createImpl<IFence>(this, createInfo);
    }

    void IDevice::DestroyFence(Fence* handle)
    {
        destroyImpl<IFence>(this, (IFence*)handle);
    }

    uint64_t IDevice::GetFenceValue(Fence* handle)
    {
        // No timeline tracking; report the fence's stored value.
        return ((IFence*)handle)->value;
    }

    QueryPool* IDevice::CreateQueryPool(const QueryPoolCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyQueryPool(QueryPool* handle)
    {
        (void)handle;
    }

    Swapchain* IDevice::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroySwapchain(Swapchain* swapchain)
    {
        (void)swapchain;
    }
} // namespace RHI::WebGPU
