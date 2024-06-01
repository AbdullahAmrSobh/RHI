#pragma once

#include "RHI/Format.hpp"

#include "RHI/Common/Handle.hpp"
#include "RHI/Common/Flags.hpp"
#include "RHI/Common/Span.hpp"
#include "RHI/Common/Hash.hpp"

namespace RHI
{
    inline static constexpr uint32_t c_MaxRenderTargetAttachmentsCount           = 16u;
    inline static constexpr uint32_t c_MaxImageBindingArrayElementsCount         = 32u;
    inline static constexpr uint32_t c_MaxBufferBindingArrayElementsCount        = 32u;
    inline static constexpr uint32_t c_MaxBufferViewBindingArrayElementsCount    = 32u;
    inline static constexpr uint32_t c_MaxBufferSamplerBindingArrayElementsCount = 32u;
    inline static constexpr uint32_t c_MaxBindGroupElementsCount                 = 32u;
    inline static constexpr uint32_t c_MaxPipelineVertexBindings                 = 32u;
    inline static constexpr uint32_t c_MaxPipelineVertexAttributes               = 32u;
    inline static constexpr uint32_t c_MaxPipelineBindGroupsCount                = 4u;
    inline static constexpr uint32_t c_MaxShaderBindGroupElementsCount           = 32u;

    class ShaderModule;

    RHI_DECALRE_OPAQUE_RESOURCE(Image);
    RHI_DECALRE_OPAQUE_RESOURCE(Buffer);
    RHI_DECALRE_OPAQUE_RESOURCE(ImageView);
    RHI_DECALRE_OPAQUE_RESOURCE(BufferView);
    RHI_DECALRE_OPAQUE_RESOURCE(BindGroupLayout);
    RHI_DECALRE_OPAQUE_RESOURCE(BindGroup);
    RHI_DECALRE_OPAQUE_RESOURCE(PipelineLayout);
    RHI_DECALRE_OPAQUE_RESOURCE(GraphicsPipeline);
    RHI_DECALRE_OPAQUE_RESOURCE(ComputePipeline);
    RHI_DECALRE_OPAQUE_RESOURCE(Sampler);

    enum class Access
    {
        None      = 0,
        Read      = 1 << 0,
        Write     = 1 << 1,
        ReadWrite = Read | Write,
    };

    enum class AllocationAlgorithm
    {
        Linear,
        Optimal,
    };

    enum class MemoryType
    {
        CPU,
        GPULocal,
        GPUShared,
    };

    enum class SampleCount
    {
        None      = 0 << 0,
        Samples1  = 1 << 0,
        Samples2  = 1 << 1,
        Samples4  = 1 << 2,
        Samples8  = 1 << 3,
        Samples16 = 1 << 4,
        Samples32 = 1 << 5,
        Samples64 = 1 << 6,
    };

    enum class ImageUsage
    {
        None            = 0 << 0,
        ShaderResource  = 1 << 1,
        StorageResource = 1 << 2,
        Color           = 1 << 3,
        Depth           = 1 << 4,
        Stencil         = 1 << 5,
        DepthStencil    = Depth | Stencil,
        CopySrc         = 1 << 6,
        CopyDst         = 1 << 7,
        Resolve         = CopyDst
    };

    enum class ImageType
    {
        None,
        Image1D,
        Image2D,
        Image3D,
    };

    enum class ImageViewType
    {
        None,
        View1D,
        View1DArray,
        View2D,
        View2DArray,
        View3D,
        CubeMap,
    };

    enum class ImageAspect
    {
        None         = 0,
        Color        = 1 << 1,
        Depth        = 1 << 2,
        Stencil      = 1 << 3,
        DepthStencil = Depth | Stencil,
        All          = Color | DepthStencil,
    };

    enum class ComponentSwizzle
    {
        Identity = 0,
        Zero,
        One,
        R,
        G,
        B,
        A,
    };

    enum class BufferUsage
    {
        None    = 0 << 0,
        Storage = 1 << 1,
        Uniform = 1 << 2,
        Vertex  = 1 << 3,
        Index   = 1 << 4,
        CopySrc = 1 << 5,
        CopyDst = 1 << 6,
    };

    enum class ShaderStage
    {
        None    = 0 << 0,
        Vertex  = 1 << 1,
        Pixel   = 1 << 2,
        Compute = 1 << 3,
    };

    RHI_DEFINE_FLAG_OPERATORS(SampleCount);
    RHI_DEFINE_FLAG_OPERATORS(ImageAspect);
    RHI_DEFINE_FLAG_OPERATORS(BufferUsage);
    RHI_DEFINE_FLAG_OPERATORS(ShaderStage);

    enum class ShaderBindingType
    {
        None,
        Sampler,
        SampledImage,
        StorageImage,
        UniformBuffer,
        StorageBuffer,

        DynamicUniformBuffer,
        DynamicStorageBuffer,

        BufferView,
        StorageBufferView,
        Count,
    };

    enum class PipelineVertexInputRate
    {
        None,
        PerInstance,
        PerVertex,
    };

    enum class PipelineRasterizerStateCullMode
    {
        None,
        FrontFace,
        BackFace,
        Discard,
    };

    enum class PipelineRasterizerStateFillMode
    {
        Point,
        Triangle,
        Line
    };

    enum class PipelineTopologyMode
    {
        Points,
        Lines,
        Triangles,
    };

    enum class PipelineRasterizerStateFrontFace
    {
        Clockwise,
        CounterClockwise,
    };

    enum class CompareOperator
    {
        Never,
        Equal,
        NotEqual,
        Greater,
        GreaterOrEqual,
        Less,
        LessOrEqual,
        Always,
    };

    enum class SamplerFilter
    {
        Point,
        Linear,
    };

    enum class SamplerAddressMode
    {
        Repeat,
        Clamp,
    };

    enum class SamplerCompareOperation
    {
        Never,
        Equal,
        NotEqual,
        Always,
        Less,
        LessEq,
        Greater,
        GreaterEq,
    };

    enum class BlendFactor
    {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
    };

    enum class BlendEquation
    {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max,
    };

    enum class ColorWriteMask
    {
        Red   = 0x01,
        Green = 0x02,
        Blue  = 0x04,
        Alpha = 0x08,
        All   = Red | Green | Blue | Alpha,
    };

    enum class MemoryAllocationFlags
    {
        None,
        LazilyAllocated,
        Mappable
    };

    enum class FenceState
    {
        NotSubmitted,
        Pending,
        Signaled,
    };

    struct ImageOffset2D
    {
        int32_t x;
        int32_t y;

        inline bool operator==(const ImageOffset2D& other) const { return x == other.x && y == other.y; }
    };

    struct ImageOffset3D
    {
        int32_t x;
        int32_t y;
        int32_t z;

        inline bool operator==(const ImageOffset3D& other) const { return x == other.x && y == other.y && z == other.z; }
    };

    struct ImageSize2D
    {
        uint32_t width;
        uint32_t height;

        inline bool operator==(const ImageSize2D& other) const { return width == other.width && height == other.height; }
    };

    struct ImageSize3D
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth;

        inline bool operator==(const ImageSize3D& other) const { return width == other.width && height == other.height && depth == other.depth; }
    };

    struct ComponentMapping
    {
        ComponentMapping() = default;

        ComponentSwizzle r = ComponentSwizzle::R;
        ComponentSwizzle g = ComponentSwizzle::G;
        ComponentSwizzle b = ComponentSwizzle::B;
        ComponentSwizzle a = ComponentSwizzle::A;

        inline bool operator==(const ComponentMapping& other) const { return r == other.r && g == other.g && b == other.b && a == other.a; }
    };

    struct ImageSubresource
    {
        ImageSubresource() = default;

        Flags<ImageAspect> imageAspects = ImageAspect::All;
        uint32_t           mipLevel     = 0;
        uint32_t           arrayLayer   = 0;

        inline bool operator==(const ImageSubresource& other) const { return imageAspects == other.imageAspects && mipLevel == other.mipLevel && arrayLayer == other.arrayLayer; }
    };

    struct ImageSubresourceLayers
    {
        ImageSubresourceLayers() = default;

        Flags<ImageAspect> imageAspects = ImageAspect::All;
        uint32_t           mipLevel     = 0;
        uint32_t           arrayBase    = 0;
        uint32_t           arrayCount   = 1;

        inline bool operator==(const ImageSubresourceLayers& other) const { return imageAspects == other.imageAspects && mipLevel == other.mipLevel && arrayBase == other.arrayBase && arrayCount == other.arrayCount; }
    };

    struct ImageSubresourceRange
    {
        ImageSubresourceRange() = default;

        Flags<ImageAspect> imageAspects  = ImageAspect::All;
        uint32_t           mipBase       = 0;
        uint32_t           mipLevelCount = 1;
        uint32_t           arrayBase     = 0;
        uint32_t           arrayCount    = 1;

        inline bool operator==(const ImageSubresourceRange& other) const { return imageAspects == other.imageAspects && mipBase == other.mipBase && mipLevelCount == other.mipLevelCount && arrayBase == other.arrayBase && arrayCount == other.arrayCount; }
    };

    struct ShaderBinding
    {
        inline static constexpr uint32_t VariableArraySize = UINT32_MAX;

        ShaderBindingType  type;
        Access             access;
        uint32_t           arrayCount;
        Flags<ShaderStage> stages;
    };

    struct BindGroupLayoutCreateInfo
    {
        ShaderBinding bindings[c_MaxBindGroupElementsCount];
    };

    struct ResourceBinding
    {
        enum class Type
        {
            Image,
            Buffer,
            DynamicBuffer,
            Sampler,
        };

        struct DynamicBufferBinding
        {
            DynamicBufferBinding(Handle<Buffer> buffer, size_t offset, size_t range)
                : buffer(buffer)
                , offset(offset)
                , range(range)
            {
            }

            Handle<Buffer> buffer;
            size_t         offset, range;
        };

        union ResourceData
        {
            ResourceData() {}

            ~ResourceData() {}

            TL::Span<const Handle<ImageView>>    images;
            TL::Span<const Handle<Buffer>>       buffers;
            TL::Span<const DynamicBufferBinding> dynamicBuffers;
            TL::Span<const Handle<Sampler>>      samplers;
        };

        uint32_t     binding;
        uint32_t     dstArrayElement;
        Type         type;
        ResourceData data;

        ResourceBinding(uint32_t binding, uint32_t dstArrayElement, TL::Span<const Handle<ImageView>> images)
            : binding(binding)
            , dstArrayElement(dstArrayElement)
            , type(Type::Image)
        {
            data.images = images;
        }

        ResourceBinding(uint32_t binding, uint32_t dstArrayElement, TL::Span<const Handle<Buffer>> buffers)
            : binding(binding)
            , dstArrayElement(dstArrayElement)
            , type(Type::Buffer)
        {
            data.buffers = buffers;
        }

        ResourceBinding(uint32_t binding, uint32_t dstArrayElement, TL::Span<const DynamicBufferBinding> dynamicBuffers)
            : binding(binding)
            , dstArrayElement(dstArrayElement)
            , type(Type::DynamicBuffer)
        {
            data.dynamicBuffers = dynamicBuffers;
        }

        ResourceBinding(uint32_t binding, uint32_t dstArrayElement, TL::Span<const Handle<Sampler>> samplers)
            : binding(binding)
            , dstArrayElement(dstArrayElement)
            , type(Type::Sampler)
        {
            data.samplers = samplers;
        }
    };

    struct PipelineLayoutCreateInfo
    {
        Handle<BindGroupLayout> layouts[c_MaxPipelineBindGroupsCount];
    };

    struct ColorAttachmentBlendStateDesc
    {
        bool                  blendEnable  = false;
        BlendEquation         colorBlendOp = BlendEquation::Add;
        BlendFactor           srcColor     = BlendFactor::One;
        BlendFactor           dstColor     = BlendFactor::Zero;
        BlendEquation         alphaBlendOp = BlendEquation::Add;
        BlendFactor           srcAlpha     = BlendFactor::One;
        BlendFactor           dstAlpha     = BlendFactor::Zero;
        Flags<ColorWriteMask> writeMask    = ColorWriteMask::All;

        inline bool operator==(const ColorAttachmentBlendStateDesc& other) const { return blendEnable == other.blendEnable && colorBlendOp == other.colorBlendOp && srcColor == other.srcColor && dstColor == other.dstColor && alphaBlendOp == other.alphaBlendOp && srcAlpha == other.srcAlpha && dstAlpha == other.dstAlpha; }
    };

    struct PipelineRenderTargetLayout
    {
        Format colorAttachmentsFormats[c_MaxRenderTargetAttachmentsCount] = { Format::BGRA8_UNORM };
        Format depthAttachmentFormat                                      = Format::Unknown;
        Format stencilAttachmentFormat                                    = Format::Unknown;
    };

    struct PipelineVertexBindingDesc
    {
        uint32_t                binding;
        uint32_t                stride;
        PipelineVertexInputRate stepRate;
    };

    struct PipelineVertexAttributeDesc
    {
        uint32_t location;
        uint32_t binding;
        Format   format;
        uint32_t offset;
    };

    struct PipelineInputAssemblerStateDesc
    {
        PipelineVertexBindingDesc   bindings[c_MaxPipelineVertexBindings];
        PipelineVertexAttributeDesc attributes[c_MaxPipelineVertexAttributes];
    };

    struct PipelineRasterizerStateDesc
    {
        PipelineRasterizerStateCullMode  cullMode  = PipelineRasterizerStateCullMode::BackFace;
        PipelineRasterizerStateFillMode  fillMode  = PipelineRasterizerStateFillMode::Triangle;
        PipelineRasterizerStateFrontFace frontFace = PipelineRasterizerStateFrontFace::CounterClockwise;
        float                            lineWidth = 1.0f;
    };

    struct PipelineMultisampleStateDesc
    {
        SampleCount sampleCount   = SampleCount::Samples1;
        bool        sampleShading = false;
    };

    struct PipelineDepthStencilStateDesc
    {
        bool            depthTestEnable   = false;
        bool            depthWriteEnable  = false;
        CompareOperator compareOperator   = CompareOperator::Always;
        bool            stencilTestEnable = false;
    };

    struct PipelineColorBlendStateDesc
    {
        ColorAttachmentBlendStateDesc blendStates[c_MaxRenderTargetAttachmentsCount];
        float                         blendConstants[4];
    };

    struct ImageCreateInfo
    {
        const char*       name = nullptr;
        Flags<ImageUsage> usageFlags;
        ImageType         type;
        ImageSize3D       size;
        Format            format;
        SampleCount       sampleCount = SampleCount::Samples1;
        uint32_t          mipLevels   = 1;
        uint32_t          arrayCount  = 1;

        inline bool operator==(const ImageCreateInfo& other) const { return usageFlags == other.usageFlags && type == other.type && size == other.size && format == other.format && mipLevels == other.mipLevels && arrayCount == other.arrayCount; }
    };

    struct BufferCreateInfo
    {
        const char*        name = nullptr;
        MemoryType         heapType;
        Flags<BufferUsage> usageFlags;
        size_t             byteSize;

        inline bool operator==(const BufferCreateInfo& other) const { return usageFlags == other.usageFlags && byteSize == other.byteSize; }
    };

    struct ImageViewCreateInfo
    {
        ImageViewCreateInfo() = default;

        inline ImageViewCreateInfo(RHI::ImageAspect aspect)
            : components{}
            , subresource{}
        {
            subresource.imageAspects = aspect;
        }

        const char*           name = nullptr;
        Handle<Image>         image;
        ImageViewType         viewType;
        ComponentMapping      components;
        ImageSubresourceRange subresource;

        inline bool operator==(const ImageViewCreateInfo& other) const { return components == other.components && subresource == other.subresource; }
    };

    struct BufferViewCreateInfo
    {
        const char*    name;
        Handle<Buffer> buffer;
        Format         format;
        size_t         byteOffset;
        size_t         byteSize;

        inline bool operator==(const BufferViewCreateInfo& other) const { return byteOffset == other.byteOffset && byteSize == other.byteSize && format == other.format; }
    };

    struct GraphicsPipelineCreateInfo
    {
        const char*                     name = nullptr;
        const char*                     vertexShaderName;
        ShaderModule*                   vertexShaderModule;
        const char*                     pixelShaderName;
        ShaderModule*                   pixelShaderModule;
        Handle<PipelineLayout>          layout;
        PipelineInputAssemblerStateDesc inputAssemblerState;
        PipelineRenderTargetLayout      renderTargetLayout;
        PipelineColorBlendStateDesc     colorBlendState    = PipelineColorBlendStateDesc{};
        PipelineTopologyMode            topologyMode       = PipelineTopologyMode::Triangles;
        PipelineRasterizerStateDesc     rasterizationState = PipelineRasterizerStateDesc{};
        PipelineMultisampleStateDesc    multisampleState   = PipelineMultisampleStateDesc{};
        PipelineDepthStencilStateDesc   depthStencilState  = PipelineDepthStencilStateDesc{};
    };

    struct ComputePipelineCreateInfo
    {
        const char*            name = nullptr;
        const char*            shaderName;
        ShaderModule*          shaderModule;
        Handle<PipelineLayout> layout;
    };

    struct SamplerCreateInfo
    {
        SamplerCreateInfo() = default;

        SamplerCreateInfo(SamplerFilter           filter,
                          SamplerAddressMode      addressMode,
                          SamplerCompareOperation compare    = SamplerCompareOperation::Always,
                          float                   mipLodBias = 0.0f,
                          float                   minLod     = 0.0f,
                          float                   maxLod     = 1.0f);

        const char*             name       = nullptr;
        SamplerFilter           filterMin  = SamplerFilter::Point;
        SamplerFilter           filterMag  = SamplerFilter::Point;
        SamplerFilter           filterMip  = SamplerFilter::Point;
        SamplerCompareOperation compare    = SamplerCompareOperation::Always;
        float                   mipLodBias = 0.0f;
        SamplerAddressMode      addressU   = SamplerAddressMode::Clamp;
        SamplerAddressMode      addressV   = SamplerAddressMode::Clamp;
        SamplerAddressMode      addressW   = SamplerAddressMode::Clamp;
        float                   minLod     = 0.0f;
        float                   maxLod     = 1.0f;

        inline bool operator==(const SamplerCreateInfo& other) const { return filterMin == other.filterMin && filterMag == other.filterMag && filterMip == other.filterMip && compare == other.compare && mipLodBias == other.mipLodBias && addressU == other.addressU && addressV == other.addressV && addressW == other.addressW && minLod == other.minLod && maxLod == other.maxLod; }
    };

    class RHI_EXPORT ShaderModule
    {
    public:
        ShaderModule()          = default;
        virtual ~ShaderModule() = default;
    };

    class RHI_EXPORT Fence
    {
    public:
        Fence()          = default;
        virtual ~Fence() = default;

        inline bool Wait(uint64_t timeout = UINT64_MAX)
        {
            return WaitInternal(timeout);
        }

        virtual void       Reset()    = 0;
        virtual FenceState GetState() = 0;

    protected:
        virtual bool WaitInternal(uint64_t timeout) = 0;
    };

} // namespace RHI

namespace RHI
{
    inline SamplerCreateInfo::SamplerCreateInfo(SamplerFilter           filter,
                                                SamplerAddressMode      addressMode,
                                                SamplerCompareOperation compare,
                                                float                   mipLodBias,
                                                float                   minLod,
                                                float                   maxLod)
        : filterMin(filter)
        , filterMag(filter)
        , filterMip(filter)
        , compare(compare)
        , mipLodBias(mipLodBias)
        , addressU(addressMode)
        , addressV(addressMode)
        , addressW(addressMode)
        , minLod(minLod)
        , maxLod(maxLod)
    {
    }
} // namespace RHI

namespace std
{
    RHI_DEFINE_POD_HASH(RHI::ImageViewCreateInfo);
    RHI_DEFINE_POD_HASH(RHI::BufferViewCreateInfo);
    RHI_DEFINE_POD_HASH(RHI::SamplerCreateInfo);
} // namespace std
