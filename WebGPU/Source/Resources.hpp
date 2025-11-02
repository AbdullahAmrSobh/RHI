#pragma once

#include <RHI/BindGroup.hpp>
#include <RHI/Pipeline.hpp>
#include <RHI/Resources.hpp>
#include <RHI/Result.hpp>

#include <dawn/webgpu.h>

namespace RHI::WebGPU
{
    class IDevice;
    struct IBindGroup;
    struct IBindGroupLayout;

    WGPUTextureFormat        ConvertToTextureFormat(Format format);
    WGPUVertexFormat         ConvertToVertexFormat(Format format);
    WGPUFilterMode           ConvertToSamplerFilter(SamplerFilter filter);
    uint32_t                 ConvertToSampleCount(SampleCount count);
    WGPUTextureUsage         ConvertToTextureUsage(TL::Flags<ImageUsage> usage);
    WGPUBufferUsage          ConvertToBufferUsage(TL::Flags<BufferUsage> bufferUsageFlags);
    WGPUTextureDimension     ConvertToTextureDimension(ImageType imageType);
    WGPUTextureViewDimension ConvertToTextureViewDimension(ImageViewType imageViewType, bool asArray);
    WGPUExtent2D             ConvertToExtent2D(ImageSize2D size);
    WGPUExtent3D             ConvertToExtent3D(ImageSize3D size);
    WGPUExtent2D             ConvertToExtent2D(ImageSize3D size);
    WGPUOrigin2D             ConvertToOffset2D(ImageOffset2D offset);
    WGPUOrigin3D             ConvertToOffset3D(ImageOffset3D offset);
    WGPUAddressMode          ConvertToAddressMode(SamplerAddressMode addressMode);
    WGPUMipmapFilterMode     ConvertToMipmapFilter(SamplerFilter samplerFilter);
    WGPUTextureAspect        ConvertToTextureAspect(TL::Flags<ImageAspect> imageAspect, Format format);
    WGPUShaderStage          ConvertToShaderStage(TL::Flags<ShaderStage> shaderStageFlags);
    WGPUPrimitiveTopology    ConvertToPrimitiveTopology(PipelineTopologyMode topology);
    WGPUIndexFormat          ConvertToIndexFormat(IndexType indexType);
    WGPUFrontFace            ConvertToFrontFace(PipelineRasterizerStateFrontFace frontFace);
    WGPUCullMode             ConvertToCullMode(PipelineRasterizerStateCullMode cullMode);
    WGPUCompareFunction      ConvertToCompareFunction(CompareOperator compareOperator);
    WGPUVertexStepMode       ConvertToVertexStepMode(PipelineVertexInputRate rate);

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

    struct IRayTracingPipeline : RayTracingPipeline
    {
        WGPUPipelineLayout layout;
        // WebGPU doesn't support ray tracing yet
        void* pipeline;

        ResultCode Init(IDevice* device, const ComputePipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBuffer : Buffer
    {
        WGPUBuffer buffer;
        void*      mappedPtr;
        bool       mapped;

        ResultCode Init(IDevice* device, const BufferCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        DeviceMemoryPtr Map(IDevice* device);
        void           Unmap(IDevice* device);
    };

    struct IImage : Image
    {
        WGPUTexture     texture;
        WGPUTextureView view;

        // TODO: the following should be removed
        ImageSize3D           size;
        Format                format;
        ImageSubresourceRange subresources;

        ResultCode Init(IDevice* device, const ImageCreateInfo& createInfo);
        ResultCode Init(IDevice* device, WGPUTexture texture, WGPUSurfaceConfiguration desc);
        void       Shutdown(IDevice* device);
        void       Write(IDevice* device, uint32_t mipLevel, TL::Block data);

        // Selects specific aspect from the available image aspects
        WGPUTextureAspect SelectImageAspect(ImageAspect aspect);
    };

    struct ISampler : Sampler
    {
        WGPUSampler sampler;

        ResultCode Init(IDevice* device, const SamplerCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    class ISwapchain final : public Swapchain
    {
    public:
        ISwapchain();
        ~ISwapchain();

        ResultCode Init(IDevice* device, const SwapchainCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        // Interface
        uint32_t            GetImagesCount() const override;
        Image*              GetImage() const override;
        SurfaceCapabilities GetSurfaceCapabilities() const override;
        ResultCode          Resize(const ImageSize2D& size) override;
        ResultCode          Configure(const SwapchainConfigureInfo& configInfo) override;
        ResultCode          Present() override;

    private:
        ResultCode SwapBackTextures();

        IDevice*               m_device  = nullptr;
        WGPUSurface            m_surface = nullptr;
        SwapchainConfigureInfo m_configuration;
        Image*                 m_imageHandle = nullptr;
        uint32_t               m_imageCount  = 0;
    };
} // namespace RHI::WebGPU