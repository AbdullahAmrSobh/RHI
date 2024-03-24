#pragma once

#include "RHI/Format.hpp"
#include "RHI/Access.hpp"

#include "RHI/Common/Handle.hpp"
#include "RHI/Common/Flags.hpp"
#include "RHI/Common/Span.hpp"
#include "RHI/Common/Hash.hpp"
#include "RHI/Common/Containers.h"

#include <variant> // todo: remove

namespace RHI
{
    inline static constexpr uint32_t c_MaxRenderTargetAttachmentsCount           = 16u;
    inline static constexpr uint32_t c_MaxSwapchainBackBuffersCount              = 3u;
    inline static constexpr uint32_t c_MaxImageBindingArrayElementsCount         = 32u;
    inline static constexpr uint32_t c_MaxBufferBindingArrayElementsCount        = 32u;
    inline static constexpr uint32_t c_MaxBufferViewBindingArrayElementsCount    = 32u;
    inline static constexpr uint32_t c_MaxBufferSamplerBindingArrayElementsCount = 32u;
    inline static constexpr uint32_t c_MaxBindGroupElementsCount                 = 32u;
    inline static constexpr uint32_t c_MaxPipelineVertexBindings                 = 32u;
    inline static constexpr uint32_t c_MaxPipelineVertexAttributes               = 32u;
    inline static constexpr uint32_t c_MaxPipelineBindGroupsCount                = 4u;
    inline static constexpr uint32_t c_MaxShaderBindGroupElementsCount           = 32u;

    class Context;
    class ShaderModule;
    class ImageAttachment;
    class ResourcePool;

    // clang-format off
    struct Image {};
    struct Buffer {};
    struct ImageView {};
    struct BufferView {};
    struct BindGroupLayout {};
    struct BindGroup {};
    struct PipelineLayout {};
    struct GraphicsPipeline {};
    struct ComputePipeline {};
    struct Sampler {};

    // clang-format on

    /// @brief Represents a pointer to GPU device memory
    using DeviceMemoryPtr = void*;

    /// @brief Enumeration representing the memory allocation startegy of a resource pool.
    enum class AllocationAlgorithm
    {
        Linear, /// @brief Memory will be allocated linearly.
        Optimal,
    };

    /// @brief Enumeration representing different memory types and their locations.
    enum class MemoryType
    {
        CPU,       // The memory is located in the system main memory.
        GPULocal,  // The memory is locaed in the GPU, and can't be accessed by the CPU.
        GPUShared, // The memory is locaed in the GPU, but can be accessed by the CPU.
    };

    /// @brief Enumerates the multisample count in an image or an graphics pipeline multisample state
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

    /// @brief Enumeration representing how the image resource is intented to used.
    enum class ImageUsage
    {
        None            = 0 << 0, // Invalid flag
        ShaderResource  = 1 << 1, // The image will be used in an shader as bind resource.
        StorageResource = 1 << 2,
        Color           = 1 << 3, // The image will be the render target color attachment.
        Depth           = 1 << 4, // The image will be the render target depth attachment.
        Stencil         = 1 << 5, // The image will be the render target stencil attachment.
        DepthStencil    = Depth | Stencil,
        CopySrc         = 1 << 6, // The image content will be copied.
        CopyDst         = 1 << 7, // The image content will be overwritten by a copy command.
    };

    /// @brief Enumeration representing the dimensions of an image resource.
    enum class ImageType
    {
        None,    // Invalid flag
        Image1D, // Image is 1 dimentional.
        Image2D, // Image is 2 dimentional.
        Image3D, // Image is 3 dimentional.
    };

    /// @brief Enumeration representing the aspects of an image resource.
    enum class ImageAspect
    {
        None         = 0,
        Color        = 1 << 1,
        Depth        = 1 << 2,
        Stencil      = 1 << 3,
        DepthStencil = Depth | Stencil,
        All          = Color | DepthStencil,
    };

    /// @brief Enumeration representing the component mapping.
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

    /// @brief Enumeration representing how the buffer resource is intented to used.
    enum class BufferUsage
    {
        None    = 0 << 0,
        Storage = 1 << 1, // The buffer will be used as a storage buffer object.
        Uniform = 1 << 2, // The buffer will be used as an uniform buffer object.
        Vertex  = 1 << 3, // The buffer will be used as a vertex buffer object.
        Index   = 1 << 4, // The buffer will be used as a index buffer object.
        CopySrc = 1 << 5, // This buffer content will be copied from.
        CopyDst = 1 << 6, // This buffer content will be overwritten by a copy command.
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

    /// @brief The type of the shader resource to be bound.
    enum class ShaderBindingType
    {
        None,
        Sampler,
        SampledImage,
        StorageImage,
        UniformBuffer,
        StorageBuffer,
        BufferView,
        StorageBufferView,
        Count,
    };

    /// @brief Pipeline vertex
    enum class PipelineVertexInputRate
    {
        None,
        PerInstance,
        PerVertex,
    };

    /// @brief Pipeline rasterizer state cull mode.
    enum class PipelineRasterizerStateCullMode
    {
        None,
        FrontFace,
        BackFace,
        Discard,
    };

    /// @brief Pipeline rasterizer state fill mode.
    enum class PipelineRasterizerStateFillMode
    {
        Point,
        Triangle,
        Line
    };

    /// @brief Pipeline topology mode.
    enum class PipelineTopologyMode
    {
        Points,
        Lines,
        Triangles,
    };

    /// @brief The orientation of triangle fornt faces
    enum class PipelineRasterizerStateFrontFace
    {
        Clockwise,
        CounterClockwise,
    };

    /// @brief Operator used to compare two values
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

    /// @brief Enumeration representing the sampler filtering.
    enum class SamplerFilter
    {
        Point,
        Linear,
    };

    /// @brief Enumeration representing the sampler addressing mode.
    enum class SamplerAddressMode
    {
        Repeat,
        Clamp,
    };

    /// @brief Enumeration representing the sampler compare opertion.
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

    /// @brief Enumerates ...
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

    /// @brief Enumerates ...
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

    enum class SwapchainPresentMode
    {
        Immediate,
        Fifo,
        FifoRelaxed,
        Mailbox,
    };

    enum class FenceState
    {
        NotSubmitted,
        Pending,
        Signaled,
    };

    /// @brief Represent the offset into an image resource.
    struct ImageOffset
    {
        int32_t     x; // Offset in the X direction
        int32_t     y; // Offset in the Y direction
        int32_t     z; // Offset in the Z direction

        inline bool operator==(const ImageOffset& other) const { return x == other.x && y == other.y && z == other.z; }

        inline bool operator!=(const ImageOffset& other) const { return !(*this == other); }
    };

    /// @brief Represent the size of an image resource or subregion.
    struct ImageSize2D
    {
        uint32_t    width;  // The width of the image.
        uint32_t    height; // The height of the image.

        inline bool operator==(const ImageSize2D& other) const { return width == other.width && height == other.height; }

        inline bool operator!=(const ImageSize2D& other) const { return !(*this == other); }
    };

    /// @brief Represent the size of an image resource or subregion.
    struct ImageSize3D
    {
        uint32_t    width;  // The width of the image.
        uint32_t    height; // The height of the image.
        uint32_t    depth;  // The depth of the image.

        inline bool operator==(const ImageSize3D& other) const { return width == other.width && height == other.height && depth == other.depth; }

        inline bool operator!=(const ImageSize3D& other) const { return !(*this == other); }
    };

    /// @brief Represent the texel color swizzling operation
    struct ComponentMapping
    {
        ComponentMapping() = default;

        ComponentSwizzle r = ComponentSwizzle::R;
        ComponentSwizzle g = ComponentSwizzle::G;
        ComponentSwizzle b = ComponentSwizzle::B;
        ComponentSwizzle a = ComponentSwizzle::A;

        inline bool      operator==(const ComponentMapping& other) const { return r == other.r && g == other.g && b == other.b && a == other.a; }

        inline bool      operator!=(const ComponentMapping& other) const { return !(*this == other); }
    };

    /// @brief Represent a subview into a an image resource.
    struct ImageSubresource
    {
        ImageSubresource()              = default;

        Flags<ImageAspect> imageAspects = ImageAspect::All;
        uint32_t           mipLevel     = 0;
        uint32_t           arrayLayer   = 0;

        inline bool        operator==(const ImageSubresource& other) const { return imageAspects == other.imageAspects && mipLevel == other.mipLevel && arrayLayer == other.arrayLayer; }

        inline bool        operator!=(const ImageSubresource& other) const { return !(*this == other); }
    };

    struct ImageSubresourceLayers
    {
        ImageSubresourceLayers()        = default;

        Flags<ImageAspect> imageAspects = ImageAspect::All;
        uint32_t           mipLevel     = 0;
        uint32_t           arrayBase    = 0;
        uint32_t           arrayCount   = 1;

        inline bool        operator==(const ImageSubresourceLayers& other) const { return imageAspects == other.imageAspects && mipLevel == other.mipLevel && arrayBase == other.arrayBase && arrayCount == other.arrayCount; }

        inline bool        operator!=(const ImageSubresourceLayers& other) const { return !(*this == other); }
    };

    struct ImageSubresourceRange
    {
        ImageSubresourceRange()          = default;

        Flags<ImageAspect> imageAspects  = ImageAspect::All;
        uint32_t           mipBase       = 0;
        uint32_t           mipLevelCount = 1;
        uint32_t           arrayBase     = 0;
        uint32_t           arrayCount    = 1;

        inline bool        operator==(const ImageSubresourceRange& other) const { return imageAspects == other.imageAspects && mipBase == other.mipBase && mipLevelCount == other.mipLevelCount && arrayBase == other.arrayBase && arrayCount == other.arrayCount; }

        inline bool        operator!=(const ImageSubresourceRange& other) const { return !(*this == other); }
    };

    /// @brief Represent a subview into a an buffer resource.
    struct BufferSubregion
    {
        BufferSubregion() = default;

        size_t      byteSize;
        size_t      byteOffset;

        inline bool operator==(const BufferSubregion& other) const { return byteSize == other.byteSize && byteOffset == other.byteOffset; }

        inline bool operator!=(const BufferSubregion& other) const { return !(*this == other); }
    };

    /// @brief Specifies a single shader resource binding.
    struct ShaderBinding
    {
        ShaderBindingType  type;
        Access             access;
        uint32_t           arrayCount;
        Flags<ShaderStage> stages;
    };

    /// @brief A shader bind group layout is an list of shader bindings.
    struct BindGroupLayoutCreateInfo
    {
        ShaderBinding bindings[c_MaxBindGroupElementsCount];
    };

    /// @brief An object that groups shader resources that are bound together.
    struct BindGroupData
    {
        BindGroupData() = default;

        void BindImages(uint32_t index, TL::Span<Handle<ImageView>> handles, uint32_t arrayOffset = 0);

        void BindBuffers(uint32_t index, TL::Span<Handle<Buffer>> handles, uint32_t arrayOffset = 0);

        void BindBuffers(uint32_t index, TL::Span<Handle<BufferView>> handles, uint32_t arrayOffset = 0);

        void BindSamplers(uint32_t index, TL::Span<Handle<Sampler>> samplers, uint32_t arrayOffset = 0);

        struct ResourceImageBinding
        {
            uint32_t                      arrayOffset;
            TL::Vector<Handle<ImageView>> views;
        };

        struct ResourceBufferBinding
        {
            uint32_t                   arrayOffset;
            TL::Vector<Handle<Buffer>> views;
        };

        struct ResourceBufferViewBinding
        {
            uint32_t                       arrayOffset;
            TL::Vector<Handle<BufferView>> views;
        };

        struct ResourceSamplerBinding
        {
            uint32_t                    arrayOffset;
            TL::Vector<Handle<Sampler>> samplers;
        };

        using ResourceBinding                                         = std::variant<ResourceImageBinding, ResourceBufferBinding, ResourceSamplerBinding>;

        ResourceBinding m_bindings[c_MaxShaderBindGroupElementsCount] = {};
    };

    // @brief the layout of pipeline shaders
    struct PipelineLayoutCreateInfo
    {
        Handle<BindGroupLayout> layouts[c_MaxPipelineBindGroupsCount]; // remove this
    };

    /// @brief Structure specifying the blending parameters for an image render target attachment.
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

        inline bool           operator==(const ColorAttachmentBlendStateDesc& other) const { return blendEnable == other.blendEnable && colorBlendOp == other.colorBlendOp && srcColor == other.srcColor && dstColor == other.dstColor && alphaBlendOp == other.alphaBlendOp && srcAlpha == other.srcAlpha && dstAlpha == other.dstAlpha; }

        inline bool           operator!=(const ColorAttachmentBlendStateDesc& other) const { return !(*this == other); }
    };

    /// @brief Structure specifying the render target layout.
    struct PipelineRenderTargetLayout
    {
        Format colorAttachmentsFormats[c_MaxRenderTargetAttachmentsCount] = { Format::BGRA8_UNORM }; // default: BGRA8 List of the formats of color attachments.
        Format depthAttachmentFormat                                      = Format::Unknown;         // default: none Format of an optional depth and/or stencil attachment.
        Format stencilAttachmentFormat                                    = Format::Unknown;         // default: none Format of an optional depth and/or stencil attachment.
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

    /// @brief Structure specifying the rasterizer state.
    struct PipelineRasterizerStateDesc
    {
        PipelineRasterizerStateCullMode  cullMode  = PipelineRasterizerStateCullMode::BackFace;
        PipelineRasterizerStateFillMode  fillMode  = PipelineRasterizerStateFillMode::Triangle;
        PipelineRasterizerStateFrontFace frontFace = PipelineRasterizerStateFrontFace::CounterClockwise;
        float                            lineWidth = 1.0f;
    };

    /// @brief Structure specifying the multisample state.
    struct PipelineMultisampleStateDesc
    {
        SampleCount sampleCount   = SampleCount::Samples1;
        bool        sampleShading = false;
    };

    /// @brief Structure specifying the depth and stencil state.
    struct PipelineDepthStencilStateDesc
    {
        bool            depthTestEnable   = false;
        bool            depthWriteEnable  = false;
        CompareOperator compareOperator   = CompareOperator::Always;
        bool            stencilTestEnable = false;
    };

    /// @brief Structure specifying the color attachments blend state.
    struct PipelineColorBlendStateDesc
    {
        ColorAttachmentBlendStateDesc blendStates[c_MaxRenderTargetAttachmentsCount];
        float                         blendConstants[4];
    };

    /// @brief Represent the creation parameters of an resource pool.
    struct ResourcePoolCreateInfo
    {
        AllocationAlgorithm allocationAlgorithm = AllocationAlgorithm::Linear;
        MemoryType          heapType;
        size_t              blockSize;
        size_t              minBlockCount;
        size_t              maxBlockCount;
        size_t              minBlockAlignment;
    };

    struct AllocationInfo
    {
        // optional pool which the resource will be allocated from
        /// @note: if a pool specified, the pool's settings will override
        ResourcePool* pool;
        MemoryType    heapType;
    };

    /// @brief Represent the creation parameters of an image resource.
    struct ImageCreateInfo
    {
        const char*       debugName;
        AllocationInfo    allocationInfo;
        Flags<ImageUsage> usageFlags;                          // Usage flags.
        ImageType         type;                                // The type of the image.
        ImageSize3D       size;                                // The size of the image.
        Format            format;                              // The format of the image.
        SampleCount       sampleCount = SampleCount::Samples1; // The number of samples in each texel.
        uint32_t          mipLevels   = 1;                     // The number of mip levels in the image.
        uint32_t          arrayCount  = 1;                     // The number of images in the images array.

        inline bool       operator==(const ImageCreateInfo& other) const { return usageFlags == other.usageFlags && type == other.type && size == other.size && format == other.format && mipLevels == other.mipLevels && arrayCount == other.arrayCount; }

        inline bool       operator!=(const ImageCreateInfo& other) const { return !(*this == other); }
    };

    /// @brief Represent the creation parameters of an buffer resource.
    struct BufferCreateInfo
    {
        const char*        debugName;
        AllocationInfo     allocationInfo;
        Flags<BufferUsage> usageFlags; // Usage flags.
        size_t             byteSize;   // The size of the buffer.

        inline bool        operator==(const BufferCreateInfo& other) const { return usageFlags == other.usageFlags && byteSize == other.byteSize; }

        inline bool        operator!=(const BufferCreateInfo& other) const { return !(*this == other); }
    };

    /// @brief Structure specifying the parameters of an image attachment.
    struct ImageViewCreateInfo
    {
        ImageViewCreateInfo() = default;

        inline ImageViewCreateInfo(RHI::ImageAspect aspect)
            : components{}
            , subresource{}
        {
            subresource.imageAspects = aspect;
        }

        const char*           debugName;
        Handle<Image>         image;
        ComponentMapping      components;
        ImageSubresourceRange subresource;

        inline bool           operator==(const ImageViewCreateInfo& other) const { return components == other.components && subresource == other.subresource; }

        inline bool           operator!=(const ImageViewCreateInfo& other) const { return !(*this == other); }
    };

    /// @brief Structure specifying the parameters of an buffer attachment.
    struct BufferViewCreateInfo
    {
        const char*    debugName;
        Handle<Buffer> buffer;
        Format         format;
        size_t         byteOffset;
        size_t         byteSize;

        inline bool    operator==(const BufferViewCreateInfo& other) const { return byteOffset == other.byteOffset && byteSize == other.byteSize && format == other.format; }

        inline bool    operator!=(const BufferViewCreateInfo& other) const { return !(*this == other); }
    };

    /// @brief Description of the graphics pipeline states.
    struct GraphicsPipelineCreateInfo
    {
        const char*                     debugName;

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

    /// @brief Description of a compute pipeline state.
    struct ComputePipelineCreateInfo
    {
        const char*            debugName;

        const char*            shaderName;
        ShaderModule*          shaderModule;
        Handle<PipelineLayout> layout;
    };

    /// @brief Structure describing the creation parameters of a sampler state.
    struct SamplerCreateInfo
    {
        SamplerCreateInfo() = default;

        SamplerCreateInfo(SamplerFilter           filter,
                          SamplerAddressMode      addressMode,
                          SamplerCompareOperation compare    = SamplerCompareOperation::Always,
                          float                   mipLodBias = 0.0f,
                          float                   minLod     = 0.0f,
                          float                   maxLod     = 1.0f)
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

        const char*             debugName;

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

        inline bool             operator==(const SamplerCreateInfo& other) const { return filterMin == other.filterMin && filterMag == other.filterMag && filterMip == other.filterMip && compare == other.compare && mipLodBias == other.mipLodBias && addressU == other.addressU && addressV == other.addressV && addressW == other.addressW && minLod == other.minLod && maxLod == other.maxLod; }

        inline bool             operator!=(const SamplerCreateInfo& other) const { return !(*this == other); }
    };

    class RHI_EXPORT ShaderModule
    {
    public:
        ShaderModule()          = default;
        virtual ~ShaderModule() = default;
    };

    class RHI_EXPORT ResourcePool
    {
    public:
        ResourcePool()          = default;
        virtual ~ResourcePool() = default;
    };

    /// @brief Fence object used to preform CPU-GPU sync
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

#define RHI_DEFINE_POD_HASH_SPECIALIZATION(Type)        \
    template<>                                          \
    struct hash<Type>                                   \
    {                                                   \
        std::size_t operator()(const Type& value) const \
        {                                               \
            return ::RHI::HashAny<Type>(value);         \
        }                                               \
    };

namespace std
{
    RHI_DEFINE_POD_HASH_SPECIALIZATION(RHI::ImageViewCreateInfo);
    RHI_DEFINE_POD_HASH_SPECIALIZATION(RHI::BufferViewCreateInfo);
    RHI_DEFINE_POD_HASH_SPECIALIZATION(RHI::SamplerCreateInfo);
} // namespace std
