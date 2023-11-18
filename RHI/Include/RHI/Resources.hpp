#pragma once

#include "RHI/Flags.hpp"
#include "RHI/Format.hpp"
#include "RHI/Handle.hpp"
#include "RHI/Result.hpp"
#include "RHI/Span.hpp"

#include <unordered_map>
#include <variant>

namespace RHI
{
    class Context;
    class ShaderModule;

    // clang-format off
    struct Image {};
    struct Buffer {};
    struct ImageView {};
    struct BufferView {};
    struct ShaderBindGroup {};
    struct PipelineLayout {};
    struct GraphicsPipeline {};
    struct ComputePipeline {};
    struct Sampler {};
    // clang-format on

    struct ImagePassAttachment;
    struct BufferPassAttachment;

    /// @brief Represents a pointer to GPU device memory

    /// @brief Enumeration for common allocation size constants
    namespace AllocationSizeConstants
    {

        inline static constexpr uint32_t KB = 1024;

        inline static constexpr uint32_t MB = 1024 * KB;

        inline static constexpr uint32_t GB = 1024 * MB;

    }; // namespace AllocationSizeConstants

    /// @brief Enumeration representing the memory allocation startegy of a resource pool.
    enum class AllocationAlgorithm
    {
        /// @brief Memory will be allocated linearly.
        Linear,
    };

    /// @brief Enumeration representing different memory types and their locations.
    enum class MemoryType
    {
        /// @brief The memory is located in the system main memory.
        CPU,

        /// @brief The memory is locaed in the GPU, and can't be accessed by the CPU.
        GPULocal,

        /// @brief The memory is locaed in the GPU, but can be accessed by the CPU.
        GPUShared,
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
        None = 0 << 0,

        /// @brief The image will be used in an shader as bind resource.
        ShaderResource = 1 << 1,

        /// @brief The image will be the render target color attachment.
        Color = 1 << 3,

        /// @brief The image will be the render target depth attachment.
        Depth = 1 << 4,

        /// @brief The image will be the render target stencil attachment.
        Stencil = 1 << 5,

        /// @brief The image content will be copied.
        CopySrc = 1 << 6,

        /// @brief The image content will be overwritten by a copy command.
        CopyDst = 1 << 7,
    };

    /// @brief Enumeration representing the dimensions of an image resource.
    enum class ImageType
    {
        None,

        /// @brief Image is 1 dimentional.
        Image1D,

        /// @brief Image is 2 dimentional.
        Image2D,

        /// @brief Image is 3 dimentional.
        Image3D,
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
        None = 0 << 0,

        /// @brief The buffer will be used as a storage buffer object.
        Storage = 1 << 1,

        /// @brief The buffer will be used as an uniform buffer object.
        Uniform = 1 << 2,

        /// @brief The buffer will be used as a vertex buffer object.
        Vertex = 1 << 3,

        /// @brief The buffer will be used as a index buffer object.
        Index = 1 << 4,

        /// @brief This buffer content will be copied from.
        CopySrc = 1 << 5,

        /// @brief This buffer content will be overwritten by a copy command.
        CopyDst = 1 << 6,
    };

    enum class ShaderStage
    {
        Vertex,
        Pixel,
        Compute,
    };

    /// @brief The type of the shader resource to be bound.
    enum class ShaderBindingType
    {
        None,
        Sampler,
        Image,
        Buffer,
    };

    /// @brief How the resource will be accessed in the shader.
    enum class ShaderBindingAccess
    {
        OnlyRead,
        ReadWrite,
    };

    /// @brief Pipeline vertex
    enum class PipelineVertexInputRate
    {
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

    /// @brief Report describe the current state of the resource pool.
    struct ResourcePoolReport
    {
        MemoryType type;

        size_t     size;

        size_t     alignment;

        size_t     allocationsCount;
    };

    /// @brief Represent the offset into an image resource.
    struct ImageOffset
    {
        /// @brief Offset in the X direction.
        int32_t     x;

        /// @brief Offset in the Y direction.
        int32_t     y;

        /// @brief Offset in the Z direction.
        int32_t     z;

        inline bool operator==(const ImageOffset& other) const
        {
            return x == other.x && y == other.y && z == other.z;
        }

        inline bool operator!=(const ImageOffset& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Represent the size of an image resource or subregion.
    struct ImageSize
    {
        /// @brief The width of the image.
        uint32_t    width;

        /// @brief The height of the image.
        uint32_t    height;

        /// @brief The depth of the image.
        uint32_t    depth;

        inline bool operator==(const ImageSize& other) const
        {
            return width == other.width && height == other.height && depth == other.depth;
        }

        inline bool operator!=(const ImageSize& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Represent the texel color swizzling operation
    struct ComponentMapping
    {
        ComponentMapping() = default;

        ComponentSwizzle r = ComponentSwizzle::R;

        ComponentSwizzle g = ComponentSwizzle::G;

        ComponentSwizzle b = ComponentSwizzle::B;

        ComponentSwizzle a = ComponentSwizzle::A;

        inline bool      operator==(const ComponentMapping& other) const
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }

        inline bool operator!=(const ComponentMapping& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Represent a subview into a an image resource.
    struct ImageSubresource
    {
        ImageSubresource() = default;

        uint32_t           arrayBase    = 0;
        uint32_t           arrayCount   = 1;
        uint32_t           mipBase      = 0;
        uint32_t           mipCount     = 1;
        Flags<ImageAspect> imageAspects = ImageAspect::All;

        inline bool        operator==(const ImageSubresource& other) const
        {
            return arrayBase == other.arrayBase && arrayCount == other.arrayCount && mipBase == other.mipBase && mipCount == other.mipCount;
        }

        inline bool operator!=(const ImageSubresource& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Represent a subview into a an buffer resource.
    struct BufferSubregion
    {
        BufferSubregion() = default;

        size_t      byteSize;

        size_t      byteOffset;

        Format      format;

        inline bool operator==(const BufferSubregion& other) const
        {
            return byteSize == other.byteSize && byteOffset == other.byteOffset && format == other.format;
        }

        inline bool operator!=(const BufferSubregion& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Specifies a single shader resource binding.
    struct ShaderBinding
    {
        ShaderBindingType       type;
        ShaderBindingAccess     access;
        uint32_t                arrayCount;
        Flags<RHI::ShaderStage> stages;
    };

    /// @brief A shader bind group layout is an list of shader bindings.
    struct ShaderBindGroupLayout
    {
        ShaderBindGroupLayout(std::initializer_list<ShaderBinding> initList)
            : bindings(initList)
        {
        }

        std::vector<ShaderBinding> bindings;
    };

    /// @brief Structure specifying the blending parameters for an image render target attachment.
    struct ColorAttachmentBlendStateDesc
    {
        bool                  blendEnable = false;
        BlendEquation         colorBlendOp;
        BlendFactor           srcColor;
        BlendFactor           dstColor;
        BlendEquation         alphaBlendOp;
        BlendFactor           srcAlpha;
        BlendFactor           dstAlpha;
        Flags<ColorWriteMask> writeMask = ColorWriteMask::All;

        inline bool           operator==(const ColorAttachmentBlendStateDesc& other) const
        {
            return blendEnable == other.blendEnable && colorBlendOp == other.colorBlendOp && srcColor == other.srcColor && dstColor == other.dstColor && alphaBlendOp == other.alphaBlendOp && srcAlpha == other.srcAlpha && dstAlpha == other.dstAlpha;
        }

        inline bool operator!=(const ColorAttachmentBlendStateDesc& other) const
        {
            return !(*this == other);
        }
    };

    struct ShaderStageDesc
    {
    };

    /// @brief Structure specifying the render target layout.
    struct PipelineRenderTargetLayout
    {
        /// @brief List of the formats of color attachments.
        TL::Span<const Format> colorAttachmentsFormats;

        /// @brief Format of an optional depth and/or stencil attachment.
        /// Could be Format::Unkown.
        Format                 depthAttachmentFormat;

        /// @brief Format of an optional depth and/or stencil attachment.
        /// Could be Format::Unkown.
        Format                 stencilAttachmentFormat;
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
        std::vector<PipelineVertexBindingDesc>   bindings;
        std::vector<PipelineVertexAttributeDesc> attributes;
    };

    /// @brief Structure specifying the rasterizer state.
    struct PipelineRasterizerStateDesc
    {
        PipelineRasterizerStateCullMode  cullMode = PipelineRasterizerStateCullMode::BackFace;

        PipelineRasterizerStateFillMode  fillMode = PipelineRasterizerStateFillMode::Triangle;

        PipelineRasterizerStateFrontFace frontFace = PipelineRasterizerStateFrontFace::CounterClockwise;

        float                            lineWidth = 1.0f;
    };

    /// @brief Structure specifying the multisample state.
    struct PipelineMultisampleStateDesc
    {
        SampleCount sampleCount;

        bool        sampleShading;
    };

    /// @brief Structure specifying the depth and stencil state.
    struct PipelineDepthStencilStateDesc
    {
        bool            depthTestEnable;

        bool            depthWriteEnable;

        CompareOperator compareOperator;

        bool            stencilTestEnable;
    };

    /// @brief Structure specifying the color attachments blend state.
    struct PipelineColorBlendStateDesc
    {
        TL::Span<const ColorAttachmentBlendStateDesc> blendStates;

        float                                         blendConstants[4];
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

    /// @brief Represent the creation parameters of an image resource.
    struct ImageCreateInfo
    {
        /// @brief Usage flags.
        Flags<ImageUsage> usageFlags;

        /// @brief The type of the image.
        ImageType         type;

        /// @brief The size of the image.
        ImageSize         size;

        /// @brief The format of the image.
        Format            format;

        /// @brief The number of samples in each texel.
        SampleCount       sampleCount = RHI::SampleCount::Samples1;

        /// @brief The number of mip levels in the image.
        uint32_t          mipLevels = 1;

        /// @brief The number of images in the images array.
        uint32_t          arrayCount = 1;

        inline bool       operator==(const ImageCreateInfo& other) const
        {
            return usageFlags == other.usageFlags && type == other.type && size == other.size && format == other.format && mipLevels == other.mipLevels && arrayCount == other.arrayCount;
        }

        inline bool operator!=(const ImageCreateInfo& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Represent the creation parameters of an buffer resource.
    struct BufferCreateInfo
    {
        /// @brief Usage flags.
        Flags<BufferUsage> usageFlags;

        /// @brief The size of the buffer.
        size_t             byteSize;

        inline bool        operator==(const BufferCreateInfo& other) const
        {
            return usageFlags == other.usageFlags && byteSize == other.byteSize;
        }

        inline bool operator!=(const BufferCreateInfo& other) const
        {
            return !(*this == other);
        }
    };

    struct ShaderModuleCreateInfo
    {
        void*  code;
        size_t size;
    };

    /// @brief Description of the graphics pipeline states.
    struct GraphicsPipelineCreateInfo
    {
        const char*                           vertexShaderName;
        ShaderModule*                         vertexShaderModule;

        const char*                           pixelShaderName;
        ShaderModule*                         pixelShaderModule;

        PipelineInputAssemblerStateDesc       inputAssemblerState;

        PipelineTopologyMode                  topologyMode = RHI::PipelineTopologyMode::Triangles;

        TL::Span<const ShaderBindGroupLayout> bindGroupLayouts;

        PipelineRenderTargetLayout            renderTargetLayout;

        PipelineInputAssemblerStateDesc       vertexInputState;

        PipelineRasterizerStateDesc           rasterizationState;

        PipelineMultisampleStateDesc          multisampleState;

        PipelineDepthStencilStateDesc         depthStencilState;

        PipelineColorBlendStateDesc           colorBlendState;
    };

    /// @brief Description of a compute pipeline state.
    struct ComputePipelineCreateInfo
    {
        const char*                           shaderName;
        ShaderModule*                         shaderModule;

        TL::Span<const ShaderBindGroupLayout> bindGroupLayouts;
    };

    /// @brief Structure describing the creation parameters of a sampler state.
    struct SamplerCreateInfo
    {
        SamplerCreateInfo() = default;

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

        inline bool             operator==(const SamplerCreateInfo& other) const
        {
            return filterMin == other.filterMin && filterMag == other.filterMag && filterMip == other.filterMip && compare == other.compare && mipLodBias == other.mipLodBias && addressU == other.addressU && addressV == other.addressV && addressW == other.addressW && minLod == other.minLod && maxLod == other.maxLod;
        }

        inline bool operator!=(const SamplerCreateInfo& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief An object that groups shader resources that are bound together.
    class ShaderBindGroupData final
    {
    public:
        ShaderBindGroupData() = default;

        /// @brief Binds an image resource to the provided binding index and offset array index.
        /// NOTE: offset + images count should not exceed the count of the resources decalred in the layout or the shader.
        /// @param index index of the resource binding decelration in the shader.
        /// @param images list of handles of an actual resources to bind.
        /// @param arrayOffset starting offset. In case of an resources array it binds the resources starting at this number.
        inline void BindImages(uint32_t index, TL::Span<ImagePassAttachment*> images, uint32_t arrayOffset = 0)
        {
            ResourceImageBinding binding{};
            binding.arrayOffset = arrayOffset;
            binding.views       = { images.begin(), images.end() };
            m_bindings[index]   = binding;
        }

        /// @brief Binds an image resource to the provided binding index and offset array index.
        /// NOTE: offset + buffers count should not exceed the count of the resources decalred in the layout or the shader.
        /// @param index index of the resource binding decelration in the shader.
        /// @param buffers list of handles of an actual resources to bind.
        /// @param arrayOffset starting offset. In case of an resources array it binds the resources starting at this number.
        inline void BindBuffers(uint32_t index, TL::Span<BufferPassAttachment*> buffers, uint32_t arrayOffset = 0)
        {
            ResourceBufferBinding binding{};
            binding.arrayOffset = arrayOffset;
            binding.views       = { buffers.begin(), buffers.end() };
            m_bindings[index]   = binding;
        }

        /// @brief Binds an image resource to the provided binding index and offset array index.
        /// NOTE: offset + samplers count should not exceed the count of the resources decalred in the layout or the shader.
        /// @param index index of the resource binding decelration in the shader.
        /// @param samplers list of handles of an actual resources to bind.
        /// @param arrayOffset starting offset. In case of an resources array it binds the resources starting at this number.
        inline void BindSamplers(uint32_t index, TL::Span<Handle<Sampler>> samplers, uint32_t arrayOffset = 0)
        {
            ResourceSamplerBinding binding{};
            binding.arrayOffset = arrayOffset;
            binding.samplers    = { samplers.begin(), samplers.end() };
            m_bindings[index]   = binding;
        }

        struct ResourceImageBinding
        {
            uint32_t                          arrayOffset;

            std::vector<ImagePassAttachment*> views;
        };

        struct ResourceBufferBinding
        {
            uint32_t                           arrayOffset;

            std::vector<BufferPassAttachment*> views;
        };

        struct ResourceSamplerBinding
        {
            uint32_t                     arrayOffset;

            std::vector<Handle<Sampler>> samplers;
        };

        using ResourceBinding = std::variant<ResourceImageBinding, ResourceBufferBinding, ResourceSamplerBinding>;

        std::unordered_map<uint32_t, ResourceBinding> m_bindings;
    };

    class ShaderBindGroupAllocator
    {
    public:
        ShaderBindGroupAllocator()          = default;
        virtual ~ShaderBindGroupAllocator() = default;

        virtual std::vector<Handle<ShaderBindGroup>> AllocateShaderBindGroups(TL::Span<const ShaderBindGroupLayout> layouts) = 0;

        virtual void                                 Free(TL::Span<Handle<ShaderBindGroup>> groups) = 0;

        virtual void                                 Update(Handle<ShaderBindGroup> group, const ShaderBindGroupData& data) = 0;
    };

    class ShaderModule
    {
    public:
        ShaderModule()          = default;
        virtual ~ShaderModule() = default;
    };

    /// @brief General purpose pool used to allocate all kinds of resources.
    class ResourcePool
    {
    public:
        ResourcePool()          = default;
        virtual ~ResourcePool() = default;

        /// @brief Allocate an image resource.
        virtual Result<Handle<Image>>  Allocate(const ImageCreateInfo& createInfo) = 0;

        /// @brief Allocate a buffer resource.
        virtual Result<Handle<Buffer>> Allocate(const BufferCreateInfo& createInfo) = 0;

        /// @brief Free an allocated image resource.
        virtual void                   Free(Handle<Image> image) = 0;

        /// @brief Free an allocated buffer resource.
        virtual void                   Free(Handle<Buffer> buffer) = 0;

        /// @brief Get the size of an allocated image resource.
        virtual size_t                 GetSize(Handle<Image> image) const = 0;

        /// @brief Get the size of an allocated buffer resource.
        virtual size_t                 GetSize(Handle<Buffer> buffer) const = 0;
    };

} // namespace RHI