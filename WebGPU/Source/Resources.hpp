#pragma once

#include <RHI/BindGroup.hpp>
#include <RHI/Pipeline.hpp>
#include <RHI/Resources.hpp>
#include <RHI/Result.hpp>

#include <dawn/webgpu.h>

namespace RHI::WebGPU
{
    class IDevice;

    WGPUTextureFormat        ConvertToTextureFormat(Format format);
    WGPUVertexFormat         ConvertToVertexFormat(Format format);
    WGPUFilterMode           ConvertToSamplerFilter(SamplerFilter filter);
    uint32_t                 ConvertToSampleCount(SampleCount count);
    WGPUTextureUsage         ConvertToTextureUsage(TL::Flags<ImageUsage> usage);
    WGPUBufferUsage          ConvertToBufferUsage(TL::Flags<BufferUsage> filter);
    WGPUTextureDimension     ConvertToTextureDimension(ImageType type);
    WGPUTextureViewDimension ConvertToTextureViewDimension(ImageType type, bool asArray);
    WGPUExtent2D             ConvertToExtent2D(ImageSize2D extent);
    WGPUExtent3D             ConvertToExtent3D(ImageSize3D extent);
    WGPUOrigin2D             ConvertToOffset2D(ImageOffset2D extent);
    WGPUOrigin3D             ConvertToOffset3D(ImageOffset3D extent);
    WGPUAddressMode          ConvertToAddressMode(SamplerAddressMode mode);
    WGPUMipmapFilterMode     ConvertToMipmapFilter(SamplerFilter filter);
    WGPUBufferUsage          ConvertToBufferUsage(TL::Flags<BufferUsage> usage);
    WGPUTextureAspect        ConvertToTextureAspect(TL::Flags<ImageAspect> type);
    uint32_t                 ConvertToSampleCount(SampleCount count);
    WGPUPrimitiveTopology    ConvertToPrimitiveTopology(PipelineTopologyMode topology);
    WGPUIndexFormat          ConvertToIndexFormat(IndexType indexType);
    WGPUFrontFace            ConvertToFrontFace(PipelineRasterizerStateFrontFace frontFace);
    WGPUCullMode             ConvertToCullMode(PipelineRasterizerStateCullMode);
    WGPUCompareFunction      ConvertToCompareFunction(CompareOperator compareOp);
    WGPUVertexStepMode       ConvertToVertexStepMode(PipelineVertexInputRate rate);
    WGPUShaderStage          ConvertToShaderStage(TL::Flags<ShaderStage> stage);

    struct IBindGroupLayout : BindGroupLayout
    {
        WGPUBindGroupLayout bindGroupLayout;

        ResultCode Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBindGroup : BindGroup
    {
        WGPUBindGroupLayout layout;
        WGPUBindGroup       bindGroup;

        ResultCode Init(IDevice* device, const BindGroupCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        void Update(IDevice* device, const BindGroupUpdateInfo& updateInfo);
    };

    class IShaderModule final : public ShaderModule
    {
    public:
        WGPUShaderModule module;

        ResultCode Init(IDevice* device, const ShaderModuleCreateInfo& createInfo);
        void       Shutdown();
    };

    struct IPipelineLayout : PipelineLayout
    {
        WGPUPipelineLayout layout;

        ResultCode Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IGraphicsPipeline : GraphicsPipeline
    {
        WGPUPipelineLayout layout;
        WGPURenderPipeline pipeline;

        ResultCode Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IComputePipeline : ComputePipeline
    {
        WGPUPipelineLayout  layout;
        WGPUComputePipeline pipeline;

        ResultCode Init(IDevice* device, const ComputePipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBuffer : Buffer
    {
        WGPUBuffer buffer;

        ResultCode Init(IDevice* device, const BufferCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IImage : Image
    {
        WGPUTexture     texture;
        WGPUTextureView view;

        ResultCode Init(IDevice* device, const ImageCreateInfo& createInfo);
        ResultCode Init(IDevice* device, WGPUTexture texture, WGPUSurfaceConfiguration desc);
        void       Shutdown(IDevice* device);
    };

    struct ISampler : Sampler
    {
        WGPUSampler sampler;

        ResultCode Init(IDevice* device, const SamplerCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };
} // namespace RHI::WebGPU