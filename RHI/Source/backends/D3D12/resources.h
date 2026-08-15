#pragma once

#include <backends/D3D12/common.h>

#include <atomic>

namespace RHI::D3D12
{
    struct IDevice;

    struct IFence : Fence
    {
        using Fence::Fence;

        void Init(IDevice* device, const FenceCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        ID3D12Fence* m_fence = nullptr;
        std::atomic_uint64_t m_value = 0;
    };

    struct IBindGroupLayout : BindGroupLayout
    {
        using BindGroupLayout::BindGroupLayout;

        void Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct IBindGroup : BindGroup
    {
        using BindGroup::BindGroup;

        void Init(IDevice* device, const BindGroupCreateInfo& createInfo);
        void Shutdown(IDevice* device);
        void Update(IDevice* device, const BindGroupUpdateInfo& updateInfo);

        IBindGroupLayout* m_layout = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE m_resourceCpuHandle = {};
        D3D12_GPU_DESCRIPTOR_HANDLE m_resourceGpuHandle = {};
        D3D12_CPU_DESCRIPTOR_HANDLE m_samplerCpuHandle = {};
        D3D12_GPU_DESCRIPTOR_HANDLE m_samplerGpuHandle = {};
    };

    struct IShaderModule : ShaderModule
    {
        using ShaderModule::ShaderModule;

        void Init(IDevice* device, const ShaderModuleCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct IPipelineLayout : PipelineLayout
    {
        using PipelineLayout::PipelineLayout;

        void Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        ID3D12RootSignature* m_rootSignature = nullptr;
    };

    struct IGraphicsPipeline : GraphicsPipeline
    {
        using GraphicsPipeline::GraphicsPipeline;

        void Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        ID3D12PipelineState* m_pipelineState = nullptr;
        IPipelineLayout* m_layout = nullptr;
    };

    struct IComputePipeline : ComputePipeline
    {
        using ComputePipeline::ComputePipeline;

        void Init(IDevice* device, const ComputePipelineCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        ID3D12PipelineState* m_pipelineState = nullptr;
        IPipelineLayout* m_layout = nullptr;
    };

    struct IRayTracingPipeline : RayTracingPipeline
    {
        using RayTracingPipeline::RayTracingPipeline;

        void Init(IDevice* device, const RayTracingPipelineCreateInfo& createInfo);
        void Shutdown(IDevice* device);
        void GetShaderBindingTableEntry(IDevice* device, uint32_t group, size_t size, void* dstHandle);

        ID3D12StateObject* m_stateObject = nullptr;
        IPipelineLayout* m_layout = nullptr;
    };

    struct IQueryPool : QueryPool
    {
        using QueryPool::QueryPool;

        void Init(IDevice* device, const QueryPoolCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        ID3D12QueryHeap* m_queryHeap = nullptr;
    };

    struct IBuffer : Buffer
    {
        using Buffer::Buffer;

        void Init(IDevice* device, const BufferCreateInfo& createInfo);
        void Shutdown(IDevice* device);
        DeviceMemoryPtr Map(IDevice* device);
        void Unmap(IDevice* device);

        ID3D12Resource* m_resource = nullptr;
        D3D12MA::Allocation* m_allocation = nullptr;
        D3D12_GPU_VIRTUAL_ADDRESS m_address = 0;
    };

    struct IImage : Image
    {
        using Image::Image;

        void Init(IDevice* device, const ImageCreateInfo& createInfo);
        void Init(IDevice* device, const ImageViewCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        ID3D12Resource* m_resource = nullptr;
        D3D12MA::Allocation* m_allocation = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE m_rtv = {};
        D3D12_CPU_DESCRIPTOR_HANDLE m_dsv = {};
        D3D12_CPU_DESCRIPTOR_HANDLE m_srv = {};
        D3D12_CPU_DESCRIPTOR_HANDLE m_uav = {};
        bool m_ownsResource = false;
    };

    struct ISampler : Sampler
    {
        using Sampler::Sampler;

        void Init(IDevice* device, const SamplerCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle = {};
        D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle = {};
    };

    struct IAccelerationStructure : AccelerationStructure
    {
        using AccelerationStructure::AccelerationStructure;

        void Init(IDevice* device, const AccelerationStructureCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        ID3D12Resource* m_resource = nullptr;
        D3D12MA::Allocation* m_allocation = nullptr;
        D3D12_GPU_VIRTUAL_ADDRESS m_address = 0;
        AccelerationStructureSizesInfo m_sizes = {};
    };

    struct IMicromap : Micromap
    {
        using Micromap::Micromap;

        void Init(IDevice* device, const MicromapCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct ISwapchain : Swapchain
    {
        using Swapchain::Swapchain;

        void Init(IDevice* device, const SwapchainCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        IDXGISwapChain4* m_swapchain = nullptr;
        SwapchainConfigureInfo m_configuration = {};
    };
} // namespace RHI::D3D12
