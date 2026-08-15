#pragma once

#include <RHI/RHI.h>

#include "common.h"

#include <webgpu/webgpu.h>

#include <TL/Containers/InlineVector.hpp>

#include <atomic>

namespace RHI::WebGPU
{
    struct IDevice;

    struct IFence : Fence
    {
        using Fence::Fence;

        void Init(IDevice* device, const FenceCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        std::atomic_uint64_t value = 0;
    };

    struct IBindGroupLayout : BindGroupLayout
    {
        using BindGroupLayout::BindGroupLayout;

        void Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        WGPUBindGroupLayout handle = nullptr;
        TL::InlineVector<ShaderBinding, MaxBindingsPerGroup> bindings;
    };

    struct IBindGroup : BindGroup
    {
        using BindGroup::BindGroup;

        void Init(IDevice* device, const BindGroupCreateInfo& createInfo);
        void Shutdown(IDevice* device);
        void Update(IDevice* device, const BindGroupUpdateInfo& updateInfo);

        WGPUBindGroup handle = nullptr;
        IBindGroupLayout* layout = nullptr;
        TL::InlineVector<WGPUBindGroupEntry, MaxBindingsPerGroup> entries;
        const char* label = nullptr;
    };

    struct IShaderModule : ShaderModule
    {
        using ShaderModule::ShaderModule;

        void Init(IDevice* device, const ShaderModuleCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        WGPUShaderModule handle = nullptr;
    };

    struct IPipelineLayout : PipelineLayout
    {
        using PipelineLayout::PipelineLayout;

        void Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        WGPUPipelineLayout handle = nullptr;
    };

    struct IGraphicsPipeline : GraphicsPipeline
    {
        using GraphicsPipeline::GraphicsPipeline;

        void Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        WGPURenderPipeline handle = nullptr;
        IPipelineLayout* layout = nullptr;
    };

    struct IComputePipeline : ComputePipeline
    {
        using ComputePipeline::ComputePipeline;

        void Init(IDevice* device, const ComputePipelineCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        WGPUComputePipeline handle = nullptr;
        IPipelineLayout* layout = nullptr;
    };

    struct IRayTracingPipeline : RayTracingPipeline
    {
        using RayTracingPipeline::RayTracingPipeline;

        void Init(IDevice* device, const RayTracingPipelineCreateInfo& createInfo);
        void Shutdown(IDevice* device);
        void GetShaderBindingTableEntry(IDevice* device, uint32_t group, size_t size, void* dstHandle);
    };

    struct IQueryPool : QueryPool
    {
        using QueryPool::QueryPool;

        void Init(IDevice* device, const QueryPoolCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        WGPUQuerySet handle = nullptr;
    };

    struct IBuffer : Buffer
    {
        using Buffer::Buffer;

        void Init(IDevice* device, const BufferCreateInfo& createInfo);
        void Shutdown(IDevice* device);
        void* Map(IDevice* device);
        void Unmap(IDevice* device);

        WGPUBuffer handle = nullptr;
        size_t size = 0;
        bool hostMapped = false;
    };

    struct IImage : Image
    {
        using Image::Image;

        void Init(IDevice* device, const ImageCreateInfo& createInfo);
        void Init(IDevice* device, const ImageViewCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        WGPUTexture handle = nullptr;
        WGPUTextureView viewHandle = nullptr;
        bool ownsTexture = false;
        ImageSize3D size = {};
        Format format = Format::Unknown;
        ImageType type = ImageType::None;
        uint32_t arrayCount = 1;
        ImageSubresourceRange subresources = {};
    };

    struct ISampler : Sampler
    {
        using Sampler::Sampler;

        void Init(IDevice* device, const SamplerCreateInfo& createInfo);
        void Shutdown(IDevice* device);

        WGPUSampler handle = nullptr;
    };

    struct IAccelerationStructure : AccelerationStructure
    {
        using AccelerationStructure::AccelerationStructure;

        void Init(IDevice* device, const AccelerationStructureCreateInfo& createInfo);
        void Shutdown(IDevice* device);
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

        uint32_t GetImagesCount() const;
        SwapchainAcquireResult AcquireSwapchainImage();
        SurfaceCapabilities GetSurfaceCapabilities(IDevice* device) const;
        ResultCode ResizeSwapchain(IDevice* device, const ImageSize2D& size);
        ResultCode ConfigureSwapchain(IDevice* device, const SwapchainConfigureInfo& configInfo);

        WGPUSurface surface = nullptr;
        WGPUSurfaceTexture surfaceTexture = {};
        IImage image;
        SwapchainConfigureInfo configuration = {};
    };
} // namespace RHI::WebGPU
