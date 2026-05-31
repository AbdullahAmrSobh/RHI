#pragma once

#include <RHI/Device.hpp>
#include <RHI/Resources.hpp>
#include <RHI/Result.hpp>
#include <RHI/Swapchain.hpp>

#include <webgpu/webgpu.h>

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

    struct IFence : Fence
    {
        // WebGPU has no fence object; submission progress is tracked via futures / OnSubmittedWorkDone.
        uint64_t value = 0;

        ResultCode Init(IDevice* device, const FenceCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBindGroupLayout : BindGroupLayout
    {
        WGPUBindGroupLayout bindGroupLayout = nullptr;

        ResultCode Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBindGroup : BindGroup
    {
        WGPUBindGroupLayout layout    = nullptr;
        WGPUBindGroup       bindGroup = nullptr;

        ResultCode Init(IDevice* device, const BindGroupCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        void Update(IDevice* device, const BindGroupUpdateInfo& updateInfo);
    };

    struct IShaderModule : ShaderModule
    {
        WGPUShaderModule module = nullptr;

        ResultCode Init(IDevice* device, const ShaderModuleCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IPipelineLayout : PipelineLayout
    {
        WGPUPipelineLayout layout = nullptr;

        ResultCode Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IGraphicsPipeline : GraphicsPipeline
    {
        WGPUPipelineLayout layout   = nullptr;
        WGPURenderPipeline pipeline = nullptr;

        ResultCode Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IComputePipeline : ComputePipeline
    {
        WGPUPipelineLayout  layout   = nullptr;
        WGPUComputePipeline pipeline = nullptr;

        ResultCode Init(IDevice* device, const ComputePipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IRayTracingPipeline : RayTracingPipeline
    {
        ResultCode Init(IDevice* device, const RayTracingPipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
        void       GetShaderBindingTableEntry(IDevice* device, uint32_t group, size_t size, void* dstHandle);
    };

    struct IQueryPool : QueryPool
    {
        WGPUQuerySet querySet = nullptr;

        ResultCode Init(IDevice* device, const QueryPoolCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBuffer : Buffer
    {
        WGPUBuffer buffer = nullptr;

        ResultCode Init(IDevice* device, const BufferCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        DeviceMemoryPtr Map(IDevice* device);   // noop
        void            Unmap(IDevice* device); // noop
    };

    struct IImage : Image
    {
        WGPUTexture     texture = nullptr;
        WGPUTextureView view    = nullptr;

        // TODO: the following should be removed
        ImageSize3D           size         = {};
        Format                format       = Format::Unknown;
        ImageSubresourceRange subresources = {};

        ResultCode Init(IDevice* device, const ImageCreateInfo& createInfo);
        ResultCode Init(IDevice* device, const ImageViewCreateInfo& createInfo);
        ResultCode Init(IDevice* device, WGPUTexture texture, const WGPUSurfaceConfiguration& configuration);
        void       Shutdown(IDevice* device);
    };

    struct ISampler : Sampler
    {
        WGPUSampler sampler = nullptr;

        ResultCode Init(IDevice* device, const SamplerCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IAccelerationStructure : AccelerationStructure
    {
        ResultCode Init(IDevice* device, const AccelerationStructureCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IMicromap : Micromap
    {
        ResultCode Init(IDevice* device, const MicromapCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    class ISwapchain final : public Swapchain
    {
    public:
        constexpr static auto MaxImageCount = 4;

        ISwapchain();
        ~ISwapchain();

        ResultCode Init(IDevice* device, const SwapchainCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        // Interface
        uint32_t               GetImagesCount() const override;
        SwapchainAcquireResult AcquireImage() override;
        SurfaceCapabilities    GetSurfaceCapabilities() const override;
        ResultCode             Resize(const ImageSize2D& size) override;
        ResultCode             Configure(const SwapchainConfigureInfo& configInfo) override;

        ResultCode Present();

        IDevice*               m_device        = nullptr;
        WGPUSurface            m_surface       = nullptr;
        IImage*                m_imageHandle   = nullptr;
        uint32_t               m_imageCount    = 0;
        SwapchainConfigureInfo m_configuration = {};
    };
} // namespace RHI::WebGPU
