#pragma once

#include <RHI/Device.hpp>

#include <TL/Containers/Map.hpp>
#include <TL/Containers/Vector.hpp>
#include <TL/Ptr.hpp>
#include <TL/Stacktrace.hpp>

#include <D3D12MemAlloc.h>
#include <RHI-D3D12/Loader.hpp>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

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

        IDevice*             m_device    = nullptr;
        ID3D12CommandQueue*  m_queue     = nullptr;
        ID3D12Fence*         m_fence     = nullptr;
        uint64_t             m_fenceValue = 0;
        HANDLE               m_fenceEvent = nullptr;
        QueueType            m_queueType;
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

        // Interface Implementation
        uint64_t            GetNativeHandle(NativeHandleType type, uint64_t handle) override;
        uint64_t            GarbageCollect(uint64_t graphicsTimeline) override;
        Queue*              GetQueue(QueueType queueType) override;
        CommandPool*        CreateCommandPool(const CommandPoolCreateInfo& createInfo) override;
        void                DestroyCommandPool(CommandPool* handle) override;
        Fence*              CreateFence(const FenceCreateInfo& createInfo) override;
        void                DestroyFence(Fence* handle) override;
        uint64_t            GetFenceValue(Fence* handle) override;
        Swapchain*          CreateSwapchain(const SwapchainCreateInfo& createInfo) override;
        void                DestroySwapchain(Swapchain* swapchain) override;
        ShaderModule*       CreateShaderModule(const ShaderModuleCreateInfo& createInfo) override;
        void                DestroyShaderModule(ShaderModule* shaderModule) override;
        BindGroupLayout*    CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void                DestroyBindGroupLayout(BindGroupLayout* handle) override;
        BindGroup*          CreateBindGroup(const BindGroupCreateInfo& createInfo) override;
        void                DestroyBindGroup(BindGroup* handle) override;
        void                UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo) override;
        QueryPool*          CreateQueryPool(const QueryPoolCreateInfo& createInfo) override;
        void                DestroyQueryPool(QueryPool* handle) override;
        Buffer*             CreateBuffer(const BufferCreateInfo& createInfo) override;
        void                DestroyBuffer(Buffer* handle) override;
        uint64_t            GetBufferDeviceAddress(Buffer* buffer) override;
        DeviceMemoryPtr     MapBuffer(Buffer* buffer, uint64_t offset, uint64_t sizeBytes) override;
        void                UnmapBuffer(Buffer* buffer) override;
        Image*              CreateImage(const ImageCreateInfo& createInfo) override;
        Image*              CreateImageView(const ImageViewCreateInfo& createInfo) override;
        void                DestroyImage(Image* handle) override;
        Sampler*            CreateSampler(const SamplerCreateInfo& createInfo) override;
        void                DestroySampler(Sampler* handle) override;
        PipelineLayout*     CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) override;
        void                DestroyPipelineLayout(PipelineLayout* handle) override;
        GraphicsPipeline*   CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void                DestroyGraphicsPipeline(GraphicsPipeline* handle) override;
        RayTracingPipeline* CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo) override;
        void                DestroyRayTracingPipeline(RayTracingPipeline* handle) override;
        ComputePipeline*    CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
        void                DestroyComputePipeline(ComputePipeline* handle) override;

        // Helper to create an IImage wrapping a native D3D12 resource (e.g. swapchain backbuffer)
        IImage* CreateImageFromNative(ID3D12Resource* resource, Format format, ImageSize2D size);

    public:
        IDXGIFactory6*      m_dxgiFactory = nullptr;
        IDXGIAdapter*       m_dxgiAdapter = nullptr;
        ID3D12Device*       m_device      = nullptr;
        D3D12MA::Allocator* m_allocator   = nullptr;

        IQueue m_queue[(uint32_t)QueueType::Count] = {};

        // Descriptor Heaps
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvCbvUavHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_samplerHeap;

        uint32_t m_rtvDescriptorSize       = 0;
        uint32_t m_dsvDescriptorSize       = 0;
        uint32_t m_srvCbvUavDescriptorSize = 0;
        uint32_t m_samplerDescriptorSize   = 0;

        uint32_t m_rtvHeapOffset       = 0;
        uint32_t m_dsvHeapOffset       = 0;
        uint32_t m_srvCbvUavHeapOffset = 0;
        uint32_t m_samplerHeapOffset   = 0;

    private:
        // Track live resources to report leaks
        TL::Map<ICommandPool*, TL::Stacktrace>        m_liveCommandPools;
        TL::Map<IFence*, TL::Stacktrace>              m_liveFences;
        TL::Map<IQueryPool*, TL::Stacktrace>          m_liveQueryPools;
        TL::Map<ISwapchain*, TL::Stacktrace>          m_liveSwapchains;
        TL::Map<IShaderModule*, TL::Stacktrace>       m_liveShaderModules;
        TL::Map<IImage*, TL::Stacktrace>              m_liveImages;
        TL::Map<IBuffer*, TL::Stacktrace>             m_liveBuffers;
        TL::Map<IBindGroupLayout*, TL::Stacktrace>    m_liveBindGroupLayouts;
        TL::Map<IBindGroup*, TL::Stacktrace>          m_liveBindGroups;
        TL::Map<IPipelineLayout*, TL::Stacktrace>     m_livePipelineLayouts;
        TL::Map<IGraphicsPipeline*, TL::Stacktrace>   m_liveGraphicsPipelines;
        TL::Map<IRayTracingPipeline*, TL::Stacktrace> m_liveRayTracingPipelines;
        TL::Map<IComputePipeline*, TL::Stacktrace>    m_liveComputePipelines;
        TL::Map<ISampler*, TL::Stacktrace>            m_liveSamplers;
    };
} // namespace RHI::D3D12
