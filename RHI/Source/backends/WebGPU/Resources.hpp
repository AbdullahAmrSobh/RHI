#pragma once

#include <RHI/RHI.h>

#include <TL/Utils.hpp>

#include <webgpu/webgpu.h>

namespace RHI::WebGPU
{
    struct IDevice;
    struct IBindGroup;
    struct IBindGroupLayout;

    WGPUTextureFormat ConvertToTextureFormat(Format format);
    WGPUVertexFormat ConvertToVertexFormat(Format format);
    WGPUFilterMode ConvertToSamplerFilter(SamplerFilter filter);
    uint32_t ConvertToSampleCount(SampleCount count);
    WGPUTextureUsage ConvertToTextureUsage(TL::Flags<ImageUsage> usage);
    WGPUBufferUsage ConvertToBufferUsage(TL::Flags<BufferUsage> bufferUsageFlags);
    WGPUTextureDimension ConvertToTextureDimension(ImageType imageType);
    WGPUTextureViewDimension ConvertToTextureViewDimension(ImageViewType imageViewType, bool asArray);
    WGPUExtent2D ConvertToExtent2D(ImageSize2D size);
    WGPUExtent3D ConvertToExtent3D(ImageSize3D size);
    WGPUOrigin2D ConvertToOffset2D(ImageOffset2D offset);
    WGPUOrigin3D ConvertToOffset3D(ImageOffset3D offset);
    WGPUAddressMode ConvertToAddressMode(SamplerAddressMode addressMode);
    WGPUMipmapFilterMode ConvertToMipmapFilter(SamplerFilter samplerFilter);
    WGPUTextureAspect ConvertToTextureAspect(TL::Flags<ImageAspect> imageAspect, Format format);
    WGPUShaderStage ConvertToShaderStage(TL::Flags<ShaderStage> shaderStageFlags);
    WGPUPrimitiveTopology ConvertToPrimitiveTopology(PipelineTopologyMode topology);
    WGPUIndexFormat ConvertToIndexFormat(IndexType indexType);
    WGPUFrontFace ConvertToFrontFace(PipelineRasterizerStateFrontFace frontFace);
    WGPUCullMode ConvertToCullMode(PipelineRasterizerStateCullMode cullMode);
    WGPUCompareFunction ConvertToCompareFunction(CompareOperator compareOperator);
    WGPUVertexStepMode ConvertToVertexStepMode(PipelineVertexInputRate rate);

    struct IFence : Fence
    {
        IFence(TL::StringView name = {})
            : Fence(name)
        {
        }

        // WebGPU has no fence object; submission progress is tracked via futures / OnSubmittedWorkDone.
        uint64_t value = 0;

        ResultCode Init(IDevice* device, const FenceCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBindGroupLayout : BindGroupLayout
    {
        IBindGroupLayout(TL::StringView name = {})
            : BindGroupLayout(name)
        {
        }

        WGPUBindGroupLayout handle = nullptr;

        ResultCode Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBindGroup : BindGroup
    {
        IBindGroup(TL::StringView name = {})
            : BindGroup(name)
        {
        }

        WGPUBindGroupLayout layout = nullptr;
        WGPUBindGroup        handle = nullptr;

        ResultCode Init(IDevice* device, const BindGroupCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        void Update(IDevice* device, const BindGroupUpdateInfo& updateInfo);
    };

    struct IShaderModule : ShaderModule
    {
        IShaderModule(TL::StringView name = {})
            : ShaderModule(name)
        {
        }

        WGPUShaderModule handle = nullptr;

        ResultCode Init(IDevice* device, const ShaderModuleCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IPipelineLayout : PipelineLayout
    {
        IPipelineLayout(TL::StringView name = {})
            : PipelineLayout(name)
        {
        }

        WGPUPipelineLayout handle = nullptr;

        ResultCode Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IGraphicsPipeline : GraphicsPipeline
    {
        IGraphicsPipeline(TL::StringView name = {})
            : GraphicsPipeline(name)
        {
        }

        WGPURenderPipeline handle = nullptr;
        IPipelineLayout*   layout = nullptr;

        ResultCode Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IComputePipeline : ComputePipeline
    {
        IComputePipeline(TL::StringView name = {})
            : ComputePipeline(name)
        {
        }

        WGPUComputePipeline handle = nullptr;
        IPipelineLayout*    layout = nullptr;

        ResultCode Init(IDevice* device, const ComputePipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IRayTracingPipeline : RayTracingPipeline
    {
        IRayTracingPipeline(TL::StringView name = {})
            : RayTracingPipeline(name)
        {
        }

        // Unsupported on WebGPU.

        ResultCode Init(IDevice* device, const RayTracingPipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        void GetShaderBindingTableEntry(IDevice* device, uint32_t group, size_t size, void* dstHandle);
    };

    struct IQueryPool : QueryPool
    {
        IQueryPool(TL::StringView name = {})
            : QueryPool(name)
        {
        }

        WGPUQuerySet handle = nullptr;

        ResultCode Init(IDevice* device, const QueryPoolCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBuffer : Buffer
    {
        IBuffer(TL::StringView name = {})
            : Buffer(name)
        {
        }

        WGPUBuffer handle = nullptr;

        ResultCode Init(IDevice* device, const BufferCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        DeviceMemoryPtr Map(IDevice* device);
        void            Unmap(IDevice* device);
    };

    struct IImage : Image
    {
        IImage(TL::StringView name = {})
            : Image(name)
        {
        }

        WGPUTexture     handle     = nullptr;
        WGPUTextureView viewHandle = nullptr;

        // TODO: the following should be removed
        ImageSize3D           size;
        Format                format;
        ImageSubresourceRange subresources;

        ResultCode Init(IDevice* device, const ImageCreateInfo& createInfo);
        ResultCode Init(IDevice* device, const ImageViewCreateInfo& createInfo);
        ResultCode Init(IDevice* device, WGPUTexture texture, const WGPUSurfaceConfiguration& configuration);
        void       Shutdown(IDevice* device);
    };

    struct ISampler : Sampler
    {
        ISampler(TL::StringView name = {})
            : Sampler(name)
        {
        }

        WGPUSampler handle = nullptr;

        ResultCode Init(IDevice* device, const SamplerCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IAccelerationStructure : AccelerationStructure
    {
        IAccelerationStructure(TL::StringView name = {})
            : AccelerationStructure(name)
        {
        }

        // Unsupported on WebGPU.

        ResultCode Init(IDevice* device, const AccelerationStructureCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IMicromap : Micromap
    {
        IMicromap(TL::StringView name = {})
            : Micromap(name)
        {
        }

        // Unsupported on WebGPU.

        ResultCode Init(IDevice* device, const MicromapCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct ISwapchain : Swapchain
    {
        ISwapchain(TL::StringView name = {})
            : Swapchain(name)
        {
        }

        ResultCode Init(IDevice* device, const SwapchainCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        // Interface
        uint32_t               GetImagesCount() const;
        SwapchainAcquireResult AcquireSwapchainImage();
        SurfaceCapabilities    GetSurfaceCapabilities(IDevice* device) const;
        ResultCode             ResizeSwapchain(IDevice* device, const ImageSize2D& size);
        ResultCode             ConfigureSwapchain(IDevice* device, const SwapchainConfigureInfo& configInfo);
        ResultCode             Present(IDevice* device);

        WGPUSurface             m_surface       = nullptr;
        IImage*                 m_imageHandle   = nullptr;
        uint32_t                m_imageCount    = 0;
        SwapchainConfigureInfo  m_configuration = {};
    };
} // namespace RHI::WebGPU
