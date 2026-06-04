#pragma once

#include "RHI/Format.hpp"
#include "RHI/PipelineAccess.hpp"

#include <TL/Flags.hpp>
#include <TL/Span.hpp>

#include <TL/Containers/String.hpp>
#include <TL/Containers/StringView.hpp>

#include <atomic>

namespace RHI
{
    static constexpr uint64_t RemainingSize     = UINT64_MAX;
    static constexpr uint32_t BindlessArraySize = UINT32_MAX;
    static constexpr uint8_t  AllLayers         = UINT8_MAX;
    static constexpr uint8_t  AllMipLevels      = UINT8_MAX;
    static constexpr uint32_t ShaderUnused      = UINT32_MAX;

    using DeviceMemoryPtr = void*;
    using BufferAddress   = uint64_t;
    using DeviceSize      = uint64_t;

    struct Transform
    {
        float transform[3][4]; ///< 3x4 row-major affine transformation matrix.
    };

    struct ResourceBase
    {
        ResourceBase() = default;

        ResourceBase(TL::StringView name)
            : m_name(name)
        {
        }


        // TL::StringView getName() const noexcept
        const TL::String& getName() const noexcept
        {
            return m_name;
        }

        void addRef() const noexcept
        {
            m_refCount.fetch_add(1, std::memory_order_relaxed);
        }

        bool release() const noexcept
        {
            uint32_t prev = m_refCount.fetch_sub(1, std::memory_order_acq_rel);
            return prev == 1;
        }

        uint32_t getRefCount() const noexcept
        {
            return m_refCount.load(std::memory_order_relaxed);
        }

    private:
        TL::String                    m_name;
        mutable std::atomic<uint16_t> m_refCount{1};
    };

#define RHI_DEFINE_HANDLE(X)              \
    struct X : ResourceBase               \
    {                                     \
        using ResourceBase::ResourceBase; \
    };
    RHI_DEFINE_HANDLE(Fence);
    RHI_DEFINE_HANDLE(BindGroupLayout);
    RHI_DEFINE_HANDLE(BindGroup);
    RHI_DEFINE_HANDLE(Buffer);
    RHI_DEFINE_HANDLE(Image);
    RHI_DEFINE_HANDLE(Sampler);
    RHI_DEFINE_HANDLE(ShaderModule);
    RHI_DEFINE_HANDLE(PipelineLayout);
    RHI_DEFINE_HANDLE(GraphicsPipeline);
    RHI_DEFINE_HANDLE(ComputePipeline);
    RHI_DEFINE_HANDLE(RayTracingPipeline);
    RHI_DEFINE_HANDLE(AccelerationStructure);
    RHI_DEFINE_HANDLE(Micromap);
    RHI_DEFINE_HANDLE(QueryPool);

    enum class QueryType
    {
        Timestamp,
        PipelineStatistics,
        Occlusion,
        AccelerationStructureSize,
        AccelerationStructureCompactedSize,
        MicromapSize,
    };

    enum class BindingType
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
        InputAttachment,
        RayTracingAccelerationStructure,
        Count,
    };

    enum class BindPoint
    {
        Graphics,
        Compute,
        RayTracing,
    };

    enum class ShaderStage : uint32_t
    {
        None          = 0,
        Vertex        = 1 << 0,
        Pixel         = 1 << 1,
        Compute       = 1 << 2,
        Hull          = 1 << 3,
        Domain        = 1 << 4,
        RayGen        = 1 << 5,
        RayIntersect  = 1 << 6,
        RayAnyHit     = 1 << 7,
        RayClosestHit = 1 << 8,
        RayMiss       = 1 << 9,
        RayCallable   = 1 << 10,
        Mesh          = 1 << 11,
        Amplification = 1 << 12,
        AllGraphics   = Vertex | Pixel,
        AllStages     = AllGraphics | Compute,
    };

    TL_DEFINE_FLAG_OPERATORS(ShaderStage);

    enum class BufferUsage : uint32_t
    {
        None                              = 0,
        Storage                           = 1 << 0,
        Uniform                           = 1 << 1,
        Vertex                            = 1 << 2,
        Index                             = 1 << 3,
        VertexIndex                       = Vertex | Index,
        CopySrc                           = 1 << 4,
        CopyDst                           = 1 << 5,
        Indirect                          = 1 << 6,
        HostMapped                        = 1 << 7,
        DeviceBufferAddress               = 1 << 8,
        AccelerationStructureInput        = 1 << 9,
        AccelerationStructureBuildScratch = 1 << 10,
        RayTracingShaderBindingTable      = 1 << 11,
    };

    TL_DEFINE_FLAG_OPERATORS(BufferUsage);

    enum class IndexType
    {
        None,
        uint8,
        uint16,
        uint32,
    };

    enum class ImageUsage
    {
        None            = 0,
        ShaderResource  = 1 << 0,
        StorageResource = 1 << 1,
        Color           = 1 << 2,
        Depth           = 1 << 3,
        Stencil         = 1 << 4,
        DepthStencil    = Depth | Stencil,
        CopySrc         = 1 << 5,
        CopyDst         = 1 << 6,
        Resolve         = CopyDst,
        Present         = 1 << 7,
    };

    TL_DEFINE_FLAG_OPERATORS(ImageUsage);

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

    enum class ImageAspect : uint8_t
    {
        None         = 0,
        Color        = 1 << 0,
        Depth        = 1 << 1,
        Stencil      = 1 << 2,
        DepthStencil = Depth | Stencil,
        All          = Color | DepthStencil,
    };

    TL_DEFINE_FLAG_OPERATORS(ImageAspect);

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
        Line,
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

    enum class ColorWriteMask : uint32_t
    {
        Red   = 0x01,
        Green = 0x02,
        Blue  = 0x04,
        Alpha = 0x08,
        All   = Red | Green | Blue | Alpha,
    };

    TL_DEFINE_FLAG_OPERATORS(ColorWriteMask);

    enum class SampleCount : uint32_t
    {
        None      = 0,
        Samples1  = 1 << 0,
        Samples2  = 1 << 1,
        Samples4  = 1 << 2,
        Samples8  = 1 << 3,
        Samples16 = 1 << 4,
        Samples32 = 1 << 5,
        Samples64 = 1 << 6,
    };

    TL_DEFINE_FLAG_OPERATORS(SampleCount);

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

    enum class CompareOperator
    {
        Undefined,
        Never,
        Equal,
        NotEqual,
        Greater,
        GreaterOrEqual,
        Less,
        LessOrEqual,
        Always,
    };

    struct BufferSubregion
    {
        size_t offset = 0;
        size_t size   = RemainingSize;

        bool   operator==(const BufferSubregion& other) const = default;
    };

    struct BufferBindingInfo
    {
        Buffer*  buffer = nullptr;
        uint32_t offset = 0;
        uint32_t range  = 0; // TODO: use RemainingSize here
    };

    struct BindGroupAccelerationStructureBindingInfo
    {
        uint32_t               dstBinding            = 0;
        uint32_t               dstArrayElement       = 0;
        AccelerationStructure* accelerationStructure = nullptr;
    };

    struct ShaderBinding
    {
        BindingType            type         = BindingType::None;
        Access                 access       = Access::Read;
        uint32_t               arrayCount   = 1;
        TL::Flags<ShaderStage> stages       = ShaderStage::None;
        size_t                 bufferStride = 0;
    };

    struct BindGroupLayoutCreateInfo
    {
        const char*                   name     = nullptr;
        bool                          pushable = false;
        TL::Span<const ShaderBinding> bindings = {};
    };

    struct BindGroupCreateInfo
    {
        const char*      name               = nullptr;
        BindGroupLayout* layout             = nullptr;
        uint32_t         bindlessArrayCount = 0;
    };

    struct BindGroupImagesUpdateInfo
    {
        uint32_t               dstBinding      = 0;
        uint32_t               dstArrayElement = 0;
        TL::Span<Image* const> images          = {};
    };

    struct BindGroupBuffersUpdateInfo
    {
        uint32_t                          dstBinding      = 0;
        uint32_t                          dstArrayElement = 0;
        TL::Span<const BufferBindingInfo> buffers         = {};
    };

    struct BindGroupSamplersUpdateInfo
    {
        uint32_t                 dstBinding      = 0;
        uint32_t                 dstArrayElement = 0;
        TL::Span<Sampler* const> samplers        = {};
    };

    struct BindGroupUpdateInfo
    {
        TL::Span<const BindGroupBuffersUpdateInfo>                buffers                = {};
        TL::Span<const BindGroupImagesUpdateInfo>                 images                 = {};
        TL::Span<const BindGroupSamplersUpdateInfo>               samplers               = {};
        TL::Span<const BindGroupAccelerationStructureBindingInfo> accelerationStructures = {};
    };

    struct BufferCreateInfo
    {
        const char*            name       = nullptr;
        TL::Flags<BufferUsage> usageFlags = BufferUsage::None;
        size_t                 byteSize   = 0;
    };

    struct ImageOffset2D
    {
        int32_t x = 0;
        int32_t y = 0;

        bool    operator==(const ImageOffset2D& other) const = default;
    };

    struct ImageOffset3D
    {
        int32_t x = 0;
        int32_t y = 0;
        int32_t z = 0;

        bool    operator==(const ImageOffset3D& other) const = default;
    };

    struct ImageSize2D
    {
        uint32_t width  = 1;
        uint32_t height = 1;

        bool     operator==(const ImageSize2D& other) const = default;
    };

    struct ImageSize3D
    {
        uint32_t width  = 1;
        uint32_t height = 1;
        uint32_t depth  = 1;

        bool     operator==(const ImageSize3D& other) const = default;
    };

    struct ComponentMapping
    {
        ComponentSwizzle r = ComponentSwizzle::Identity;
        ComponentSwizzle g = ComponentSwizzle::Identity;
        ComponentSwizzle b = ComponentSwizzle::Identity;
        ComponentSwizzle a = ComponentSwizzle::Identity;

        bool             operator==(const ComponentMapping& other) const = default;
    };

    struct ImageSubresourceRange
    {
        TL::Flags<ImageAspect>       imageAspects  = ImageAspect::All;
        uint8_t                      mipBase       = 0;
        uint8_t                      mipLevelCount = 1;
        uint8_t                      arrayBase     = 0;
        uint8_t                      arrayCount    = 1;

        bool                         operator==(const ImageSubresourceRange& other) const = default;

        static ImageSubresourceRange All() { return {ImageAspect::All, 0, AllMipLevels, 0, AllLayers}; }
    };

    struct ImageCreateInfo
    {
        const char*           name        = nullptr;
        TL::Flags<ImageUsage> usageFlags  = ImageUsage::None;
        ImageType             type        = ImageType::None;
        ImageSize3D           size        = {};
        Format                format      = Format::Unknown;
        SampleCount           sampleCount = SampleCount::Samples1;
        uint32_t              mipLevels   = 1;
        uint32_t              arrayCount  = 1;
    };

    struct ImageViewCreateInfo
    {
        const char*           name        = nullptr;
        Image*                image       = nullptr;
        Format                format      = Format::Unknown; // Overrides the underlying image's format.
        ImageViewType         viewType    = ImageViewType::None;
        ComponentMapping      components  = {};
        ImageSubresourceRange subresource = {};

        bool                  operator==(const ImageViewCreateInfo& other) const = default;
    };

    enum class RayTracingGroupType
    {
        General,
        TrianglesHitGroup,
        ProceduralHitGroup,
    };

    enum class AccelerationStructureType
    {
        BottomLevel,
        TopLevel,
    };

    enum class AccelerationStructureFlags : uint32_t
    {
        None            = 0,
        AllowUpdate     = 1 << 0,
        AllowCompaction = 1 << 1,
    };

    TL_DEFINE_FLAG_OPERATORS(AccelerationStructureFlags);

    enum class GeometryType
    {
        Triangles,
        Aabbs,
    };

    enum class InstanceFlags : uint32_t
    {
        None                       = 0,
        TriangleFacingCullDisable  = 1 << 0,
        TriangleFlipFacing         = 1 << 2,
        ForceOpaque                = 1 << 3,
        ForceNoOpaque              = 1 << 4,
        ForceOpacityMicromap2State = 1 << 5,
        DisableOpacityMicromaps    = 1 << 6,
    };

    TL_DEFINE_FLAG_OPERATORS(InstanceFlags);

    struct AccelerationStructureGeometry
    {
        GeometryType  geometryType;

        Format        vertexFormat;
        // AABB or triangle data
        BufferAddress data;
        uint32_t      stride;
        uint32_t      count;

        IndexType     indexType;
        BufferAddress indexData;
        uint32_t      indexCount;

        BufferAddress transformData;
        uint32_t      transformCount;
    };

    struct AccelerationStructureInstance
    {
        Transform     transform;
        uint32_t      instanceCustomIndex                    : 24;
        uint32_t      mask                                   : 8;
        uint32_t      instanceShaderBindingTableRecordOffset : 24;
        InstanceFlags flags                                  : 8;
        BufferAddress accelerationStructureReference;
    };

    struct AccelerationStructureCreateInfo
    {
        const char*                           name  = nullptr;
        TL::Flags<AccelerationStructureFlags> flags = AccelerationStructureFlags::None;
        AccelerationStructureType             type  = AccelerationStructureType::BottomLevel;

        union
        {
            TL::Span<const AccelerationStructureGeometry> geometries;
            TL::Span<const AccelerationStructureInstance> instances;
        };
    };

    struct MicromapCreateInfo
    {
        const char* name = nullptr;
        // TODO: micromap build inputs
    };

    struct SamplerCreateInfo
    {
        const char*        name       = nullptr;
        SamplerFilter      filterMin  = SamplerFilter::Linear;
        SamplerFilter      filterMag  = SamplerFilter::Linear;
        SamplerFilter      filterMip  = SamplerFilter::Linear;
        CompareOperator    compare    = CompareOperator::Undefined;
        float              mipLodBias = 0.0f;
        SamplerAddressMode addressU   = SamplerAddressMode::Repeat;
        SamplerAddressMode addressV   = SamplerAddressMode::Repeat;
        SamplerAddressMode addressW   = SamplerAddressMode::Repeat;
        float              minLod     = 0.0f;
        float              maxLod     = 1000.0f;
    };

    struct PushConstantRange
    {
        TL::Flags<RHI::ShaderStage> stages = RHI::ShaderStage::None;
        uint32_t                    offset = 0;
        uint32_t                    size   = 0;
    };

    struct PipelineLayoutCreateInfo
    {
        const char*                      name          = nullptr;
        TL::Span<BindGroupLayout* const> layouts       = {};
        TL::Span<PushConstantRange>      pushConstants = {};
    };

    struct ColorAttachmentBlendStateDesc
    {
        bool                      blendEnable  = false;
        BlendEquation             colorBlendOp = BlendEquation::Add;
        BlendFactor               srcColor     = BlendFactor::One;
        BlendFactor               dstColor     = BlendFactor::Zero;
        BlendEquation             alphaBlendOp = BlendEquation::Add;
        BlendFactor               srcAlpha     = BlendFactor::One;
        BlendFactor               dstAlpha     = BlendFactor::Zero;
        TL::Flags<ColorWriteMask> writeMask    = ColorWriteMask::All;
    };

    struct PipelineRenderTargetLayout
    {
        TL::Span<const Format> colorAttachmentsFormats = {};
        Format                 depthAttachmentFormat   = Format::Unknown;
        Format                 stencilAttachmentFormat = Format::Unknown;
    };

    struct PipelineVertexAttributeDesc
    {
        uint32_t offset = 0;
        Format   format = Format::Unknown;
    };

    struct PipelineVertexBindingDesc
    {
        uint32_t                                    stride     = 0;
        PipelineVertexInputRate                     stepRate   = PipelineVertexInputRate::PerVertex;
        TL::Span<const PipelineVertexAttributeDesc> attributes = {};
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
        CompareOperator compareOperator   = CompareOperator::Less;
        bool            stencilTestEnable = false;
    };

    struct PipelineColorBlendStateDesc
    {
        TL::Span<const ColorAttachmentBlendStateDesc> blendStates       = {};
        float                                         blendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    };

    struct QueryPoolCreateInfo
    {
        const char* name  = nullptr;
        QueryType   type  = QueryType::Timestamp;
        uint32_t    count = 0;
    };

    struct PipelineShaderStage
    {
        const char*   name   = nullptr; ///< Entry point name.
        ShaderModule* module = nullptr;
        ShaderStage   stage  = ShaderStage::None;
    };

    struct GraphicsPipelineCreateInfo
    {
        const char*                               name                 = nullptr;
        TL::Span<const PipelineShaderStage>       shaderStages         = {};
        PipelineLayout*                           layout               = nullptr;
        TL::Span<const PipelineVertexBindingDesc> vertexBufferBindings = {};
        PipelineRenderTargetLayout                renderTargetLayout   = {};
        PipelineColorBlendStateDesc               colorBlendState      = {};
        PipelineTopologyMode                      topologyMode         = PipelineTopologyMode::Triangles;
        PipelineRasterizerStateDesc               rasterizationState   = {};
        PipelineMultisampleStateDesc              multisampleState     = {};
        PipelineDepthStencilStateDesc             depthStencilState    = {};
    };

    struct RayTracingShaderGroupCreateInfo
    {
        RayTracingGroupType type               = RayTracingGroupType::General;
        uint32_t            generalShader      = ShaderUnused;
        uint32_t            closestHitShader   = ShaderUnused;
        uint32_t            anyHitShader       = ShaderUnused;
        uint32_t            intersectionShader = ShaderUnused;
    };

    struct RayTracingPipelineCreateInfo
    {
        const char*                                     name              = nullptr;
        TL::Span<const PipelineShaderStage>             shaderStages      = {};
        PipelineLayout*                                 layout            = nullptr;
        TL::Span<const RayTracingShaderGroupCreateInfo> shaderGroups      = {};
        uint32_t                                        maxRecursionDepth = 0;
    };

    struct ComputePipelineCreateInfo
    {
        const char*         name          = nullptr;
        PipelineShaderStage computeShader = {};
        PipelineLayout*     layout        = nullptr;
    };

    struct ShaderModuleCreateInfo
    {
        const char*              name = nullptr;
        TL::Span<const uint32_t> code = {};
    };

    struct FenceCreateInfo
    {
        const char* name         = nullptr;
        uint64_t    initialValue = 0;
    };

    struct FenceSubmitInfo
    {
        Fence*        fence = nullptr;
        uint64_t      value = 0;
        PipelineStage stage = PipelineStage::None;
    };
} // namespace RHI
