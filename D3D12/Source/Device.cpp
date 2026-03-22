#include "Device.hpp"

#include <TL/Memory.hpp>

#include <RHI-D3D12/Loader.hpp>
#include <tracy/Tracy.hpp>

RHI::Device* RHI::CreateD3D12Device()
{
    ZoneScoped;
    auto device = new RHI::D3D12::IDevice();
    auto result = device->Init();
    TL_ASSERT(IsSuccess(result));
    return device;
}

void RHI::DestroyD3D12Device(RHI::Device* _device)
{
    ZoneScoped;
    auto device = (RHI::D3D12::IDevice*)_device;
    device->Shutdown();
    delete device;
}

namespace RHI::D3D12
{
    IDevice::IDevice()  = default;
    IDevice::~IDevice() = default;

    ResultCode IDevice::Init()
    {
        ZoneScoped;

        m_backend = BackendType::DirectX12_2;

        HRESULT hr;
        hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_dxgiFactory));
        VALIDATE_D3D12_RESULT(hr);

        for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapters(i, &m_dxgiAdapter); ++i)
        {
            hr = D3D12CreateDevice(m_dxgiAdapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&m_device));
            if (SUCCEEDED(hr))
                break;
            m_dxgiAdapter->Release();
            m_dxgiAdapter = nullptr;
        }

        if (!m_device)
            return ResultCode::ErrorUnknown;

        D3D12MA::ALLOCATOR_DESC allocatorDesc{
            .Flags                = {},
            .pDevice              = m_device,
            .PreferredBlockSize   = {},
            .pAllocationCallbacks = {},
            .pAdapter             = m_dxgiAdapter,
        };
        hr = D3D12MA::CreateAllocator(&allocatorDesc, &m_allocator);
        VALIDATE_D3D12_RESULT(hr);

        auto result = m_queue[(uint32_t)QueueType::Graphics].Init(this, "Graphics Queue", QueueType::Graphics);
        if (!IsSuccess(result)) return result;
        result = m_queue[(uint32_t)QueueType::Compute].Init(this, "Compute Queue", QueueType::Compute);
        if (!IsSuccess(result)) return result;
        result = m_queue[(uint32_t)QueueType::Transfer].Init(this, "Transfer Queue", QueueType::Transfer);
        if (!IsSuccess(result)) return result;

        m_rtvDescriptorSize       = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_dsvDescriptorSize       = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        m_srvCbvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_samplerDescriptorSize   = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
        rtvHeapDesc.NumDescriptors = 1024;
        rtvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        hr                         = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
        VALIDATE_D3D12_RESULT(hr);

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
        dsvHeapDesc.NumDescriptors = 1024;
        dsvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        hr                         = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
        VALIDATE_D3D12_RESULT(hr);

        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
        srvHeapDesc.NumDescriptors = 4096;
        srvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        hr                         = m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvCbvUavHeap));
        VALIDATE_D3D12_RESULT(hr);

        D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc{};
        samplerHeapDesc.NumDescriptors = 512;
        samplerHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        samplerHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        hr                             = m_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_samplerHeap));
        VALIDATE_D3D12_RESULT(hr);

        return ResultCode::Success;
    }

    void IDevice::Shutdown()
    {
        ZoneScoped;

        m_queue[(uint32_t)QueueType::Transfer].Shutdown();
        m_queue[(uint32_t)QueueType::Compute].Shutdown();
        m_queue[(uint32_t)QueueType::Graphics].Shutdown();

        m_samplerHeap   = nullptr;
        m_srvCbvUavHeap = nullptr;
        m_dsvHeap       = nullptr;
        m_rtvHeap       = nullptr;

        if (m_allocator)   { m_allocator->Release();   m_allocator   = nullptr; }
        if (m_device)      { m_device->Release();      m_device      = nullptr; }
        if (m_dxgiAdapter) { m_dxgiAdapter->Release(); m_dxgiAdapter = nullptr; }
        if (m_dxgiFactory) { m_dxgiFactory->Release(); m_dxgiFactory = nullptr; }
    }

    void IDevice::SetDebugName(ID3D12Object* object, const char* name) const
    {
        if (!object || !name) return;
        wchar_t wname[256];
        mbstowcs_s(nullptr, wname, name, 255);
        object->SetName(wname);
    }

    /////////////////////////////////////////////////////////////
    // IQueue
    /////////////////////////////////////////////////////////////

    ResultCode IQueue::Init(IDevice* device, const char* debugName, QueueType queueType)
    {
        ZoneScoped;
        m_device    = device;
        m_queueType = queueType;

        D3D12_COMMAND_LIST_TYPE cmdType = D3D12_COMMAND_LIST_TYPE_DIRECT;
        switch (queueType)
        {
        case QueueType::Graphics: cmdType = D3D12_COMMAND_LIST_TYPE_DIRECT;  break;
        case QueueType::Compute:  cmdType = D3D12_COMMAND_LIST_TYPE_COMPUTE; break;
        case QueueType::Transfer: cmdType = D3D12_COMMAND_LIST_TYPE_COPY;    break;
        default:                  break;
        }

        D3D12_COMMAND_QUEUE_DESC queueDesc{
            .Type     = cmdType,
            .Priority = 0,
            .Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE,
            .NodeMask = 0,
        };
        HRESULT hr = device->m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queue));
        if (FAILED(hr)) return ConvertFromHRESULT(hr);

        hr = device->m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
        if (FAILED(hr)) return ConvertFromHRESULT(hr);

        m_fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        if (!m_fenceEvent) return ResultCode::ErrorUnknown;

        device->SetDebugName(m_queue, debugName);
        return ResultCode::Success;
    }

    void IQueue::Shutdown()
    {
        if (m_fenceEvent) { CloseHandle(m_fenceEvent); m_fenceEvent = nullptr; }
        if (m_fence)      { m_fence->Release();        m_fence      = nullptr; }
        if (m_queue)      { m_queue->Release();        m_queue      = nullptr; }
    }

    void IQueue::BeginAnnotation(const char* name, uint32_t bgra) { (void)name; (void)bgra; }
    void IQueue::EndAnnotation() {}
    void IQueue::InsertAnnotation(const char* name, uint32_t bgra) { (void)name; (void)bgra; }

    void IQueue::Submit(const QueueSubmitInfo& submitInfo)
    {
        ZoneScoped;

        for (auto& waitFence : submitInfo.waitFences)
        {
            auto fence = (IFence*)waitFence.fence;
            m_queue->Wait(fence->m_fence, waitFence.value);
        }

        TL::Vector<ID3D12CommandList*> cmdLists;
        cmdLists.reserve(submitInfo.commandLists.size());
        for (auto* cmdList : submitInfo.commandLists)
            cmdLists.push_back(((ICommandList*)cmdList)->m_cmdLst);

        if (!cmdLists.empty())
            m_queue->ExecuteCommandLists((UINT)cmdLists.size(), cmdLists.data());

        for (auto& signalFence : submitInfo.signalFences)
        {
            auto fence = (IFence*)signalFence.fence;
            m_queue->Signal(fence->m_fence, signalFence.value);
        }

        for (auto* swapchain : submitInfo.presentSwapchains)
            ((ISwapchain*)swapchain)->m_swapchain->Present(1, 0);
    }

    void IQueue::WaitIdle()
    {
        ZoneScoped;
        m_fenceValue++;
        m_queue->Signal(m_fence, m_fenceValue);
        if (m_fence->GetCompletedValue() < m_fenceValue)
        {
            m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
            WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
        }
    }

    void IQueue::WaitFence(Fence* fence, uint64_t value)
    {
        ZoneScoped;
        auto* d3dFence = (IFence*)fence;
        if (d3dFence->m_fence->GetCompletedValue() < value)
        {
            d3dFence->m_fence->SetEventOnCompletion(value, m_fenceEvent);
            WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
        }
    }

    /////////////////////////////////////////////////////////////
    // IDevice Interface
    /////////////////////////////////////////////////////////////

    uint64_t IDevice::GetNativeHandle(NativeHandleType type, uint64_t handle)
    {
        switch (type)
        {
        case NativeHandleType::Device: return reinterpret_cast<uint64_t>(m_device);
        case NativeHandleType::Buffer: return reinterpret_cast<uint64_t>(((IBuffer*)handle)->resource);
        case NativeHandleType::Image:  return reinterpret_cast<uint64_t>(((IImage*)handle)->resource);
        default:                       return 0;
        }
    }

    uint64_t IDevice::GarbageCollect(uint64_t graphicsTimeline)
    {
        // TODO: implement deferred deletion queue
        (void)graphicsTimeline;
        return 0;
    }

    Queue* IDevice::GetQueue(QueueType queueType)
    {
        return &m_queue[(uint32_t)queueType];
    }

    template<typename Resource, typename... Args>
    inline Resource* CreateResource(IDevice* device, TL::Map<Resource*, TL::Stacktrace>& liveResources, Args... args)
    {
        auto*      resource = TL::construct<Resource>();
        ResultCode result   = resource->Init(device, args...);
        if (IsSuccess(result))
        {
            liveResources.emplace(resource, TL::CaptureStacktrace());
            return resource;
        }
        TL::destruct(resource);
        return nullptr;
    }

    template<typename Resource>
    inline void DestroyResource(IDevice* device, Resource* resource, TL::Map<Resource*, TL::Stacktrace>& liveResources)
    {
        auto erased = liveResources.erase(resource);
        TL_ASSERT(erased);
        resource->Shutdown(device);
        TL::destruct(resource);
    }

    // clang-format off
    CommandPool*        IDevice::CreateCommandPool(const CommandPoolCreateInfo& createInfo)                { return CreateResource<ICommandPool>(this, m_liveCommandPools, createInfo); }
    void                IDevice::DestroyCommandPool(CommandPool* handle)                                   { DestroyResource(this, (ICommandPool*)handle, m_liveCommandPools); }
    Fence*              IDevice::CreateFence(const FenceCreateInfo& createInfo)                            { return CreateResource<IFence>(this, m_liveFences, createInfo); }
    void                IDevice::DestroyFence(Fence* handle)                                               { DestroyResource(this, (IFence*)handle, m_liveFences); }
    uint64_t            IDevice::GetFenceValue(Fence* handle)                                              { return ((IFence*)handle)->m_fence->GetCompletedValue(); }
    Swapchain*          IDevice::CreateSwapchain(const SwapchainCreateInfo& createInfo)                    { return CreateResource<ISwapchain>(this, m_liveSwapchains, createInfo); }
    void                IDevice::DestroySwapchain(Swapchain* swapchain)                                    { DestroyResource(this, (ISwapchain*)swapchain, m_liveSwapchains); }
    ShaderModule*       IDevice::CreateShaderModule(const ShaderModuleCreateInfo& createInfo)              { return CreateResource<IShaderModule>(this, m_liveShaderModules, createInfo); }
    void                IDevice::DestroyShaderModule(ShaderModule* shaderModule)                           { DestroyResource(this, (IShaderModule*)shaderModule, m_liveShaderModules); }
    BindGroupLayout*    IDevice::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)        { return CreateResource<IBindGroupLayout>(this, m_liveBindGroupLayouts, createInfo); }
    void                IDevice::DestroyBindGroupLayout(BindGroupLayout* handle)                           { DestroyResource(this, (IBindGroupLayout*)handle, m_liveBindGroupLayouts); }
    BindGroup*          IDevice::CreateBindGroup(const BindGroupCreateInfo& createInfo)                    { return CreateResource<IBindGroup>(this, m_liveBindGroups, createInfo); }
    void                IDevice::DestroyBindGroup(BindGroup* handle)                                       { DestroyResource(this, (IBindGroup*)handle, m_liveBindGroups); }
    void                IDevice::UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo) { ((IBindGroup*)handle)->Update(this, updateInfo); }
    QueryPool*          IDevice::CreateQueryPool(const QueryPoolCreateInfo& createInfo)                    { return CreateResource<IQueryPool>(this, m_liveQueryPools, createInfo); }
    void                IDevice::DestroyQueryPool(QueryPool* handle)                                       { DestroyResource(this, (IQueryPool*)handle, m_liveQueryPools); }
    Buffer*             IDevice::CreateBuffer(const BufferCreateInfo& createInfo)                          { return CreateResource<IBuffer>(this, m_liveBuffers, createInfo); }
    void                IDevice::DestroyBuffer(Buffer* handle)                                             { DestroyResource(this, (IBuffer*)handle, m_liveBuffers); }
    Image*              IDevice::CreateImage(const ImageCreateInfo& createInfo)                            { return CreateResource<IImage>(this, m_liveImages, createInfo); }
    Image*              IDevice::CreateImageView(const ImageViewCreateInfo& createInfo)                    { return CreateResource<IImage>(this, m_liveImages, createInfo); }
    void                IDevice::DestroyImage(Image* handle)                                               { DestroyResource(this, (IImage*)handle, m_liveImages); }
    Sampler*            IDevice::CreateSampler(const SamplerCreateInfo& createInfo)                        { return CreateResource<ISampler>(this, m_liveSamplers, createInfo); }
    void                IDevice::DestroySampler(Sampler* handle)                                           { DestroyResource(this, (ISampler*)handle, m_liveSamplers); }
    PipelineLayout*     IDevice::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)          { return CreateResource<IPipelineLayout>(this, m_livePipelineLayouts, createInfo); }
    void                IDevice::DestroyPipelineLayout(PipelineLayout* handle)                             { DestroyResource(this, (IPipelineLayout*)handle, m_livePipelineLayouts); }
    GraphicsPipeline*   IDevice::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)      { return CreateResource<IGraphicsPipeline>(this, m_liveGraphicsPipelines, createInfo); }
    void                IDevice::DestroyGraphicsPipeline(GraphicsPipeline* handle)                         { DestroyResource(this, (IGraphicsPipeline*)handle, m_liveGraphicsPipelines); }
    RayTracingPipeline* IDevice::CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo)  { return CreateResource<IRayTracingPipeline>(this, m_liveRayTracingPipelines, createInfo); }
    void                IDevice::DestroyRayTracingPipeline(RayTracingPipeline* handle)                     { DestroyResource(this, (IRayTracingPipeline*)handle, m_liveRayTracingPipelines); }
    ComputePipeline*    IDevice::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)        { return CreateResource<IComputePipeline>(this, m_liveComputePipelines, createInfo); }
    void                IDevice::DestroyComputePipeline(ComputePipeline* handle)                           { DestroyResource(this, (IComputePipeline*)handle, m_liveComputePipelines); }
    // clang-format on

    uint64_t IDevice::GetBufferDeviceAddress(Buffer* buffer)
    {
        return static_cast<uint64_t>(((IBuffer*)buffer)->resource->GetGPUVirtualAddress());
    }

    DeviceMemoryPtr IDevice::MapBuffer(Buffer* buffer, uint64_t offset, uint64_t sizeBytes)
    {
        void*       ptr = nullptr;
        D3D12_RANGE range{offset, offset + sizeBytes};
        ((IBuffer*)buffer)->resource->Map(0, &range, &ptr);
        return static_cast<uint8_t*>(ptr) + offset;
    }

    void IDevice::UnmapBuffer(Buffer* buffer)
    {
        ((IBuffer*)buffer)->resource->Unmap(0, nullptr);
    }

    IImage* IDevice::CreateImageFromNative(ID3D12Resource* resource, Format format, ImageSize2D size)
    {
        auto* image  = TL::construct<IImage>();
        auto  result = image->InitFromNative(this, resource, format, size);
        if (IsSuccess(result))
        {
            m_liveImages.emplace(image, TL::CaptureStacktrace());
            return image;
        }
        TL::destruct(image);
        return nullptr;
    }

} // namespace RHI::D3D12
