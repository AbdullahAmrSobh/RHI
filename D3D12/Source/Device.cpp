#include "Device.hpp"

#include <TL/Assert.hpp>

namespace RHI
{
    Device* CreateD3D12Device()
    {
        auto device = new RHI::D3D12::IDevice();
        auto result = device->Init();
        TL_ASSERT(IsSuccess(result));
        return device;
    }

    void DestroyD3D12Device(Device* _device)
    {
        auto device = (RHI::D3D12::IDevice*)_device;
        device->Shutdown();
        delete device;
    }
} // namespace RHI

namespace RHI::D3D12
{
    ///////////////////////////////////////////////////////////
    // IQueue
    ///////////////////////////////////////////////////////////

    ResultCode IQueue::Init(IDevice* device, const char* debugName, QueueType queueType)
    {
        (void)device;
        (void)debugName;
        (void)queueType;
        return ResultCode::Success;
    }

    void IQueue::Shutdown()
    {
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
        (void)submitInfo;
    }

    void IQueue::WaitIdle()
    {
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
        m_backend = BackendType::DirectX12_2;
    }

    IDevice::~IDevice() = default;

    ResultCode IDevice::Init()
    {
        return ResultCode::Success;
    }

    void IDevice::Shutdown()
    {
    }

    void IDevice::SetDebugName(ID3D12Object* object, const char* name) const
    {
        (void)object;
        (void)name;
    }

    uint64_t IDevice::GarbageCollect(uint64_t graphicsTimeline)
    {
        (void)graphicsTimeline;
        return 0;
    }

    uint64_t IDevice::GetNativeHandle(NativeHandleType type, uint64_t handle)
    {
        (void)type;
        (void)handle;
        return 0;
    }

    Queue* IDevice::GetQueue(QueueType queueType)
    {
        (void)queueType;
        return nullptr;
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
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyBuffer(Buffer* handle)
    {
        (void)handle;
    }

    uint64_t IDevice::GetBufferDeviceAddress(Buffer* buffer)
    {
        (void)buffer;
        return 0;
    }

    DeviceMemoryPtr IDevice::MapBuffer(Buffer* buffer, uint64_t offset, uint64_t sizeBytes)
    {
        (void)buffer;
        (void)offset;
        (void)sizeBytes;
        return nullptr;
    }

    void IDevice::UnmapBuffer(Buffer* buffer)
    {
        (void)buffer;
    }

    Image* IDevice::CreateImage(const ImageCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    Image* IDevice::CreateImageView(const ImageViewCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyImage(Image* handle)
    {
        (void)handle;
    }

    Sampler* IDevice::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroySampler(Sampler* handle)
    {
        (void)handle;
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
        (void)createInfo;
        return nullptr;
    }

    void IDevice::DestroyFence(Fence* handle)
    {
        (void)handle;
    }

    uint64_t IDevice::GetFenceValue(Fence* handle)
    {
        (void)handle;
        return 0;
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
} // namespace RHI::D3D12
