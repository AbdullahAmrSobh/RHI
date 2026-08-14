#pragma once

#include <RHI/RHI.h>

namespace RHI::WebGPU
{
    struct IDevice;

    struct IFence : Fence
    {
        using Fence::Fence;

        ResultCode Init(IDevice* device, const FenceCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct IBindGroupLayout : BindGroupLayout
    {
        using BindGroupLayout::BindGroupLayout;

        ResultCode Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct IBindGroup : BindGroup
    {
        using BindGroup::BindGroup;

        ResultCode Init(IDevice* device, const BindGroupCreateInfo& createInfo);
        void Shutdown(IDevice* device);
        void Update(IDevice* device, const BindGroupUpdateInfo& updateInfo);
    };

    struct IShaderModule : ShaderModule
    {
        using ShaderModule::ShaderModule;

        ResultCode Init(IDevice* device, const ShaderModuleCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct IPipelineLayout : PipelineLayout
    {
        using PipelineLayout::PipelineLayout;

        ResultCode Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct IGraphicsPipeline : GraphicsPipeline
    {
        using GraphicsPipeline::GraphicsPipeline;

        ResultCode Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct IComputePipeline : ComputePipeline
    {
        using ComputePipeline::ComputePipeline;

        ResultCode Init(IDevice* device, const ComputePipelineCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct IRayTracingPipeline : RayTracingPipeline
    {
        using RayTracingPipeline::RayTracingPipeline;

        ResultCode Init(IDevice* device, const RayTracingPipelineCreateInfo& createInfo);
        void Shutdown(IDevice* device);
        void GetShaderBindingTableEntry(IDevice* device, uint32_t group, size_t size, void* dstHandle);
    };

    struct IQueryPool : QueryPool
    {
        using QueryPool::QueryPool;

        ResultCode Init(IDevice* device, const QueryPoolCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct IBuffer : Buffer
    {
        using Buffer::Buffer;

        ResultCode Init(IDevice* device, const BufferCreateInfo& createInfo);
        void Shutdown(IDevice* device);
        void* Map(IDevice* device);
        void Unmap(IDevice* device);
    };

    struct IImage : Image
    {
        using Image::Image;

        ResultCode Init(IDevice* device, const ImageCreateInfo& createInfo);
        ResultCode Init(IDevice* device, const ImageViewCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct ISampler : Sampler
    {
        using Sampler::Sampler;

        ResultCode Init(IDevice* device, const SamplerCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct IAccelerationStructure : AccelerationStructure
    {
        using AccelerationStructure::AccelerationStructure;

        ResultCode Init(IDevice* device, const AccelerationStructureCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct IMicromap : Micromap
    {
        using Micromap::Micromap;

        ResultCode Init(IDevice* device, const MicromapCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct ISwapchain : Swapchain
    {
        using Swapchain::Swapchain;

        ResultCode Init(IDevice* device, const SwapchainCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        uint32_t GetImagesCount() const;
        SwapchainAcquireResult AcquireSwapchainImage();
        SurfaceCapabilities GetSurfaceCapabilities(IDevice* device) const;
        ResultCode ResizeSwapchain(IDevice* device, const ImageSize2D& size);
        ResultCode ConfigureSwapchain(IDevice* device, const SwapchainConfigureInfo& configInfo);
    };
} // namespace RHI::WebGPU
