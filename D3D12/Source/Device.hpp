#pragma once

#include <RHI/Device.hpp>

#include <TL/Ptr.hpp>

#include <D3D12MemAlloc.h>
#include <RHI-D3D12/Loader.hpp>
#include <d3d12.h>
#include <dxgi1_6.h>

#include "CommandList.hpp"
#include "Common.hpp"
#include "Resources.hpp"

namespace RHI::D3D12
{
    class IQueue final : public Queue
    {
    public:
        ResultCode Init(IDevice* device, const char* debugName, QueueType queueType);
        void       Shutdown();

        void BeginAnnotation(const char* name, uint32_t bgra) override;
        void EndAnnotation() override;
        void InsertAnnotation(const char* name, uint32_t bgra) override;
        void Submit(const QueueSubmitInfo& submitInfo) override;
        void WaitIdle() override;
        void WaitFence(Fence* fence, uint64_t value) override;

        IDevice*            m_device     = nullptr;
        ID3D12CommandQueue* m_queue      = nullptr;
        ID3D12Fence*        m_fence      = nullptr;
        HANDLE              m_fenceEvent = nullptr;
        uint64_t            m_fenceValue = 0;
        QueueType           m_queueType  = QueueType::Graphics;
    };

    class IDevice final : public RHI::Device
    {
    public:
        friend Device* RHI::CreateD3D12Device();
        friend void    RHI::DestroyD3D12Device(Device* device);

        IDevice();
        ~IDevice();

        ResultCode Init();
        void       Shutdown();

        void SetDebugName(ID3D12Object* object, const char* name) const;

        // Interface implementation
        uint64_t               GarbageCollect(uint64_t graphicsTimeline) override;
        uint64_t               GetNativeHandle(NativeHandleType type, uint64_t handle) override;
        Queue*                 GetQueue(QueueType queueType) override;
        ShaderModule*          CreateShaderModule(const ShaderModuleCreateInfo& createInfo) override;
        void                   DestroyShaderModule(ShaderModule* shaderModule) override;
        BindGroupLayout*       CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void                   DestroyBindGroupLayout(BindGroupLayout* handle) override;
        BindGroup*             CreateBindGroup(const BindGroupCreateInfo& createInfo) override;
        void                   DestroyBindGroup(BindGroup* handle) override;
        void                   UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo) override;
        PipelineLayout*        CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) override;
        void                   DestroyPipelineLayout(PipelineLayout* handle) override;
        GraphicsPipeline*      CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void                   DestroyGraphicsPipeline(GraphicsPipeline* handle) override;
        ComputePipeline*       CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
        void                   DestroyComputePipeline(ComputePipeline* handle) override;
        RayTracingPipeline*    CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo) override;
        void                   DestroyRayTracingPipeline(RayTracingPipeline* handle) override;
        void                   GetShaderBindingTableEntry(RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle) override;
        Buffer*                CreateBuffer(const BufferCreateInfo& createInfo) override;
        void                   DestroyBuffer(Buffer* handle) override;
        uint64_t               GetBufferDeviceAddress(Buffer* buffer) override;
        DeviceMemoryPtr        MapBuffer(Buffer* buffer, uint64_t offset, uint64_t sizeBytes) override;
        void                   UnmapBuffer(Buffer* buffer) override;
        Image*                 CreateImage(const ImageCreateInfo& createInfo) override;
        Image*                 CreateImageView(const ImageViewCreateInfo& createInfo) override;
        void                   DestroyImage(Image* handle) override;
        Sampler*               CreateSampler(const SamplerCreateInfo& createInfo) override;
        void                   DestroySampler(Sampler* handle) override;
        AccelerationStructure* CreateAccelerationStructure(const AccelerationStructureCreateInfo& createInfo) override;
        void                   DestroyAccelerationStructure(AccelerationStructure* handle) override;
        uint64_t               GetAccelerationStructureDeviceAddress(AccelerationStructure* handle) override;
        AccelerationStructureSizesInfo GetAccelerationStructureSizesInfo(AccelerationStructure* handle) override;
        Micromap*              CreateMicromap(const MicromapCreateInfo& createInfo) override;
        void                   DestroyMicromap(Micromap* handle) override;
        CommandPool*           CreateCommandPool(const CommandPoolCreateInfo& createInfo) override;
        void                   DestroyCommandPool(CommandPool* handle) override;
        Fence*                 CreateFence(const FenceCreateInfo& createInfo) override;
        void                   DestroyFence(Fence* handle) override;
        uint64_t               GetFenceValue(Fence* handle) override;
        QueryPool*             CreateQueryPool(const QueryPoolCreateInfo& createInfo) override;
        void                   DestroyQueryPool(QueryPool* handle) override;
        Swapchain*             CreateSwapchain(const SwapchainCreateInfo& createInfo) override;
        void                   DestroySwapchain(Swapchain* swapchain) override;

    public:
        IDXGIFactory6*      m_dxgiFactory = nullptr;
        IDXGIAdapter*       m_dxgiAdapter = nullptr;
        ID3D12Device*       m_device      = nullptr;
        D3D12MA::Allocator* m_allocator   = nullptr;

        IQueue m_queue[(uint32_t)QueueType::Count] = {};

        // Descriptor heaps
        ID3D12DescriptorHeap* m_rtvHeap       = nullptr;
        ID3D12DescriptorHeap* m_dsvHeap       = nullptr;
        ID3D12DescriptorHeap* m_srvCbvUavHeap = nullptr;
        ID3D12DescriptorHeap* m_samplerHeap   = nullptr;

        uint32_t m_rtvDescriptorSize       = 0;
        uint32_t m_dsvDescriptorSize       = 0;
        uint32_t m_srvCbvUavDescriptorSize = 0;
        uint32_t m_samplerDescriptorSize   = 0;

        uint32_t m_rtvHeapOffset       = 0;
        uint32_t m_dsvHeapOffset       = 0;
        uint32_t m_srvCbvUavHeapOffset = 0;
        uint32_t m_samplerHeapOffset   = 0;
    };
} // namespace RHI::D3D12
