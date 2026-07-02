#pragma once

#include "RHI/Export.hpp"

#include <TL/Span.hpp>
#include <TL/Flags.hpp>
#include <TL/Compiler.hpp>
#include <TL/Containers/Vector.hpp>
#include <TL/Containers/String.hpp>
#include <TL/Containers/StringView.hpp>

namespace RHI
{
    class Queue;
    class Device;
    class CommandPool;
    class CommandList;

    static constexpr uint64_t RemainingSize     = UINT64_MAX;
    static constexpr uint32_t BindlessArraySize = UINT32_MAX;
    static constexpr uint8_t  AllLayers         = UINT8_MAX;
    static constexpr uint8_t  AllMipLevels      = UINT8_MAX;
    static constexpr uint32_t ShaderUnused      = UINT32_MAX;

    using DeviceMemoryPtr = void*;
    using BufferAddress   = uint64_t;
    using DeviceSize      = uint64_t;

    // Result

    // TODO: Break this down to more granular enums
    // e.g. memory allocation results, sync result, swapchain results
    enum class TL_NODISCARD ResultCode
    {
        Success,
        SuccessSuboptimal,
        ErrorUnknown,
        ErrorOutOfMemory,
        ErrorDeviceOutOfMemory,
        ErrorAllocationFailed,
        ErrorPoolOutOfMemory,
        ErrorDeviceRemoved,
        ErrorInvalidArguments,
        ErrorTimeout,
        ErrorOutdated,
        ErrorSurfaceLost,
        ErrorDeviceLost,
    };

    inline static bool IsSuccess(ResultCode result)
    {
        return result == ResultCode::Success || result == ResultCode::SuccessSuboptimal;
    }

    inline static bool IsError(ResultCode result)
    {
        return !IsSuccess(result);
    }

    // Device

    enum class BackendType
    {
        Vulkan1_3,
        DirectX12_2,
        WebGPU,
    };

    enum class NativeHandleType
    {
        None,
        Device,
        CommandList,
        Buffer,
        Image,
        ImageView,
        Sampler,
        ShaderModule,
        Pipeline,
        PipelineLayout,
        BindGroupLayout,
        BindGroup,
        Swapchain,
    };

    // Format

    enum class Format : uint8_t
    {
        Unknown,

        R8_UINT,
        R8_SINT,
        R8_UNORM,
        R8_SNORM,
        RG8_UINT,
        RG8_SINT,
        RG8_UNORM,
        RG8_SNORM,
        R16_UINT,
        R16_SINT,
        R16_UNORM,
        R16_SNORM,
        R16_FLOAT,
        BGRA4_UNORM,
        B5G6R5_UNORM,
        B5G5R5A1_UNORM,
        RGBA8_UINT,
        RGBA8_SINT,
        RGBA8_UNORM,
        RGBA8_SNORM,
        BGRA8_UNORM,
        SRGBA8_UNORM,
        SBGRA8_UNORM,
        R10G10B10A2_UNORM,
        R11G11B10_FLOAT,
        RG16_UINT,
        RG16_SINT,
        RG16_UNORM,
        RG16_SNORM,
        RG16_FLOAT,
        R32_UINT,
        R32_SINT,
        R32_FLOAT,
        RGBA16_UINT,
        RGBA16_SINT,
        RGBA16_FLOAT,
        RGBA16_UNORM,
        RGBA16_SNORM,
        RG32_UINT,
        RG32_SINT,
        RG32_FLOAT,
        RGB32_UINT,
        RGB32_SINT,
        RGB32_FLOAT,
        RGBA32_UINT,
        RGBA32_SINT,
        RGBA32_FLOAT,

        D16,
        D24S8,
        X24G8_UINT,
        D32,
        D32S8,
        X32G8_UINT,

        BC1_UNORM,
        BC1_UNORM_SRGB,
        BC2_UNORM,
        BC2_UNORM_SRGB,
        BC3_UNORM,
        BC3_UNORM_SRGB,
        BC4_UNORM,
        BC4_SNORM,
        BC5_UNORM,
        BC5_SNORM,
        BC6H_UFLOAT,
        BC6H_SFLOAT,
        BC7_UNORM,
        BC7_UNORM_SRGB,

        COUNT,
    };

    enum class FormatType : uint8_t
    {
        Integer,
        Normalized,
        Float,
        DepthStencil,
    };

    // Shaders

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

    // Bind groups & layout

    enum class BindingType
    {
        None,
        Sampler,
        SampledImage,
        StorageImage,
        UniformTexelBuffer,
        StorageTexelBuffer,
        UniformBuffer,
        StorageBuffer,
        UniformBufferDynamic,
        StorageBufferDynamic,
        InputAttachment,
        AccelerationStructure,
        Count,
    };

    enum class BindPoint
    {
        Graphics,
        Compute,
        RayTracing,
    };

    // Buffers

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

    // Images & samplers

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

    // Ray tracing & acceleration structures

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

    // Pipeline state

    enum class PipelineVertexInputRate
    {
        None,
        PerInstance,
        PerVertex,
    };

    enum class PipelineTopologyMode
    {
        Points,
        Lines,
        Triangles,
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

    // Synchronization

    enum class PipelineStage
    {
        None                          = 0 << 0,
        TopOfPipe                     = 1 << 0,
        DrawIndirect                  = 1 << 1,
        VertexInput                   = 1 << 2,
        VertexShader                  = 1 << 3,
        TessellationControlShader     = 1 << 4,
        TessellationEvaluationShader  = 1 << 5,
        PixelShader                   = 1 << 6,
        EarlyFragmentTests            = 1 << 7,
        LateFragmentTests             = 1 << 8,
        ColorAttachmentOutput         = 1 << 9,
        ComputeShader                 = 1 << 10,
        Transfer                      = 1 << 12,
        BottomOfPipe                  = 1 << 13,
        Host                          = 1 << 14,
        AllGraphics                   = 1 << 15,
        AllCommands                   = 1 << 16,
        Copy                          = 1 << 17,
        Resolve                       = 1 << 18,
        Blit                          = 1 << 19,
        Clear                         = 1 << 20,
        IndexInput                    = 1 << 21,
        VertexAttributeInput          = 1 << 22,
        PreRasterizationShaders       = 1 << 23,
        TransformFeedback             = 1 << 24,
        ConditionalRendering          = 1 << 25,
        FragmentShadingRateAttachment = 1 << 26,
        AccelerationStructureBuild    = 1 << 27,
        RayTracingShader              = 1 << 28,
        TaskShader                    = 1 << 29,
        MeshShader                    = 1 << 30,
        AccelerationStructureCopy     = 1 << 31,
    };

    TL_DEFINE_FLAG_OPERATORS(PipelineStage);

    enum class Access
    {
        None      = 0,
        Read      = 1 << 0,
        Write     = 1 << 1,
        ReadWrite = Read | Write,
    };

    TL_DEFINE_FLAG_OPERATORS(Access);

    // Queues

    enum class QueueType : uint8_t
    {
        Graphics,
        Compute,
        Transfer,
        Count,
    };

    // Passes & clears

    enum class LoadOperation : uint8_t
    {
        DontCare,
        Load,
        Discard,
    };

    enum class StoreOperation : uint8_t
    {
        DontCare,
        Store,
    };

    enum class ResolveMode : uint8_t
    {
        None,
        Min,
        Max,
        Avg,
    };

    enum class CopyMode : uint8_t
    {
        Clone,
        Compact,
    };

    // Queries

    enum class QueryType
    {
        Timestamp,
        PipelineStatistics,
        Occlusion,
        AccelerationStructureSize,
        AccelerationStructureCompactedSize,
        MicromapSize,
    };

    // Swapchain

    enum class SwapchainAlphaMode : uint32_t
    {
        None           = 0 << 0,
        PreMultiplied  = 1 << 0,
        PostMultiplied = 1 << 1,
    };

    enum class SwapchainPresentMode : uint32_t
    {
        None        = 0 << 0,
        Immediate   = 1 << 0,
        Fifo        = 1 << 1,
        FifoRelaxed = 1 << 2,
        Mailbox     = 1 << 3,
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
    RHI_DEFINE_HANDLE(ImageView);
    RHI_DEFINE_HANDLE(Sampler);
    RHI_DEFINE_HANDLE(ShaderModule);
    RHI_DEFINE_HANDLE(PipelineLayout);
    RHI_DEFINE_HANDLE(GraphicsPipeline);
    RHI_DEFINE_HANDLE(ComputePipeline);
    RHI_DEFINE_HANDLE(RayTracingPipeline);
    RHI_DEFINE_HANDLE(AccelerationStructure);
    RHI_DEFINE_HANDLE(Micromap);
    RHI_DEFINE_HANDLE(QueryPool);
    RHI_DEFINE_HANDLE(Swapchain);

    // Device

    struct DeviceFeatures
    {
        bool hasRaytracing;
        bool hasMeshShaders;
        bool hasPushBindGroups;
    };

    struct DeviceLimits
    {
        uint32_t minUniformBufferOffsetAlignment;
        uint32_t minStorageBufferOffsetAlignment;
        uint32_t maxMeshWorkGroupInvocations;
        uint32_t maxMeshWorkGroupSize[3];

        uint32_t minAccelerationStructureScratchOffsetAlignment;
        uint32_t rayTracingShaderGroupHandleSize;
        uint32_t rayTracingShaderGroupHandleAlignment;
        uint32_t rayTracingShaderGroupBaseAlignment;
    };

    // Format

    struct FormatInfo
    {
        const char* name;
        Format      format;
        uint8_t     bytesPerBlock;
        uint8_t     blockSize;
        FormatType  type;
        bool        hasRed     : 1;
        bool        hasGreen   : 1;
        bool        hasBlue    : 1;
        bool        hasAlpha   : 1;
        bool        hasDepth   : 1;
        bool        hasStencil : 1;
        bool        isSigned   : 1;
        bool        isSRGB     : 1;
    };

    // Shaders

    struct ShaderModuleCreateInfo
    {
        const char*              name = nullptr;
        TL::Span<const uint32_t> code = {};
    };

    struct PipelineShaderStage
    {
        const char*   name   = nullptr; ///< Entry point name.
        ShaderModule* module = nullptr;
        ShaderStage   stage  = ShaderStage::None;
    };

    // Bind groups & layout

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

    struct BindGroupBindingInfo
    {
        BindGroup*               bindGroup      = nullptr; ///< Pointer to the bind group.
        TL::Span<const uint32_t> dynamicOffsets = {};      ///< Span of dynamic offsets for the bind group.
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

    // Buffers

    struct BufferCreateInfo
    {
        const char*            name       = nullptr;
        TL::Flags<BufferUsage> usageFlags = BufferUsage::None;
        size_t                 byteSize   = 0;
    };

    struct BufferSubregion
    {
        size_t offset = 0;
        size_t size   = RemainingSize;

        bool   operator==(const BufferSubregion& other) const = default;
    };

    // Images & samplers

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

        // TODO: remove and update defaults to use AllMips/AllLayers
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

    // Ray tracing & acceleration structures

    struct Transform
    {
        float transform[3][4]; ///< 3x4 row-major affine transformation matrix.
    };

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

    struct AccelerationStructureSizesInfo
    {
        uint64_t size              = 0;
        uint64_t buildScratchSize  = 0;
        uint64_t updateScratchSize = 0;
    };

    struct MicromapCreateInfo
    {
        const char* name = nullptr;
        // TODO: micromap build inputs
    };

    struct MicromapBuildInfo
    {
        const char* name = nullptr;
    };

    struct TlasBuildInfo
    {
        AccelerationStructure* src;
        AccelerationStructure* dst;
        uint32_t               instanceCount;
        Buffer*                instanceBuffer;
        uint32_t               instanceBufferOffset;
        Buffer*                scratchBuffer;
        uint32_t               scratchBufferOffset;
    };

    struct BlasBuildInfo
    {
        AccelerationStructure*                        src;
        AccelerationStructure*                        dst;
        TL::Span<const AccelerationStructureGeometry> geometries;
        Buffer*                                       scratchBuffer;
        uint32_t                                      scratchBufferOffset;
    };

    struct StridedDeviceAddressRegion
    {
        size_t offset = 0;
        size_t stride = 0;
        size_t size   = 0;
    };

    struct DispatchRaysInfo
    {
        StridedDeviceAddressRegion raygenShader;
        StridedDeviceAddressRegion missShaders;
        StridedDeviceAddressRegion hitShaderGroups;
        StridedDeviceAddressRegion callableShaders;
        uint32_t                   x;
        uint32_t                   y;
        uint32_t                   z;
    };

    // Pipeline state

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
        TL::Span<const Format> colors  = {};
        Format                 depth   = Format::Unknown;
        Format                 stencil = Format::Unknown;
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

    struct ComputePipelineCreateInfo
    {
        const char*         name          = nullptr;
        PipelineShaderStage computeShader = {};
        PipelineLayout*     layout        = nullptr;
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

    // Synchronization

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

    struct BarrierState
    {
        TL::Flags<PipelineStage> stage  = PipelineStage::None;
        TL::Flags<Access>        access = Access::None;
    };

    struct ImageBarrierState
    {
        ImageUsage               usage  = ImageUsage::None;
        TL::Flags<PipelineStage> stage  = PipelineStage::None;
        TL::Flags<Access>        access = Access::None;

        bool                     operator==(const ImageBarrierState& self) const = default;
    };

    struct BufferBarrierState
    {
        BufferUsage              usage  = BufferUsage::None;
        TL::Flags<PipelineStage> stage  = PipelineStage::None;
        TL::Flags<Access>        access = Access::None;

        bool                     operator==(const BufferBarrierState& self) const = default;
    };

    struct BarrierInfo
    {
        BarrierState srcState = {};
        BarrierState dstState = {};
    };

    struct ImageBarrierInfo
    {
        Image*                image       = nullptr;
        ImageBarrierState     srcState    = {};
        ImageBarrierState     dstState    = {};
        ImageSubresourceRange subresource = ImageSubresourceRange::All();
    };

    struct BufferBarrierInfo
    {
        Buffer*            buffer    = nullptr;
        BufferBarrierState srcState  = {};
        BufferBarrierState dstState  = {};
        BufferSubregion    subregion = {};
    };

    // Queues

    struct QueueSubmitInfo
    {
        TL::Span<const FenceSubmitInfo> waitFences        = {};
        TL::Span<CommandList* const>    commandLists      = {};
        TL::Span<const FenceSubmitInfo> signalFences      = {};
        TL::Span<Swapchain*>            presentSwapchains = {};
    };

    // Passes & clears

    struct ColorF32
    {
        float r = 0.0, g = 0.0, b = 0.0, a = 0.0;
    };

    struct ColorU32
    {
        uint32_t r = 0, g = 0, b = 0, a = 0;
    };

    struct ColorI32
    {
        int32_t r = 0, g = 0, b = 0, a = 0;
    };

    struct DepthStencilValue
    {
        float   depthValue   = 1.0f;
        uint8_t stencilValue = 0u;
    };

    union ClearValue
    {
        ColorF32          f32;
        ColorU32          u32;
        ColorI32          i32;
        DepthStencilValue ds;
    };

    struct ComputePassBeginInfo
    {
        const char* name;
    };

    struct ColorAttachment
    {
        Image*         view        = nullptr;
        LoadOperation  loadOp      = LoadOperation::Discard;
        StoreOperation storeOp     = StoreOperation::Store;
        ClearValue     clearValue  = {0.0f, 0.0f, 0.0f, 1.0f};
        ResolveMode    resolveMode = ResolveMode::None;
        Image*         resolveView = nullptr;
    };

    struct DepthStencilAttachment
    {
        Image*            view           = nullptr;
        LoadOperation     depthLoadOp    = LoadOperation::Discard;
        StoreOperation    depthStoreOp   = StoreOperation::Store;
        LoadOperation     stencilLoadOp  = LoadOperation::Discard;
        StoreOperation    stencilStoreOp = StoreOperation::Store;
        DepthStencilValue clearValue     = {0.0f, 0};
    };

    struct RenderPassBeginInfo
    {
        ImageSize2D                     size;
        ImageOffset2D                   offset;
        TL::Span<const ColorAttachment> colorAttachments;
        DepthStencilAttachment          depthStencilAttachment;
    };

    struct ImageMemoryLayout
    {
        uint64_t offset;
        uint32_t bytesPerRow;
        uint32_t rowsPerImage;
    };

    struct ImageCopyInfo
    {
        Image*        image      = nullptr; ///< Pointer to the source image.
        uint32_t      mipLevel   = 0;       ///< Mipmap level of the source image.
        uint32_t      arrayLayer = 0;       ///< Array layer of the source image
        ImageOffset3D offset     = {};      ///< Offset in the source image.
        ImageAspect   aspect     = ImageAspect::All;
    };

    // Commands

    struct CommandListCreateInfo
    {
        const char* name      = nullptr;             ///< Name of the command list.
        QueueType   queueType = QueueType::Graphics; ///< Type of queue for the command list.
    };

    struct CommandPoolCreateInfo
    {
        const char* name = nullptr;
        QueueType   queue;
    };

    // Queries

    struct QueryPoolCreateInfo
    {
        const char* name  = nullptr;
        QueryType   type  = QueryType::Timestamp;
        uint32_t    count = 0;
    };

    // Swapchain

    struct Win32WindowDesc
    {
        void* hwnd;
        void* hinstance;
    };

    struct SwapchainCreateInfo
    {
        const char* name;

        union
        {
            Win32WindowDesc win32Window;
        };
    };

    struct SwapchainConfigureInfo
    {
        ImageSize2D           size;
        uint32_t              imageCount;
        TL::Flags<ImageUsage> imageUsage;
        Format                format;
        SwapchainPresentMode  presentMode;
        SwapchainAlphaMode    alphaMode;
    };

    struct SurfaceCapabilities
    {
        ImageSize2D                     minImageSize;
        ImageSize2D                     maxImageSize;
        uint32_t                        minImageCount;
        uint32_t                        maxImageCount;
        TL::Flags<ImageUsage>           usages;
        TL::Flags<SwapchainPresentMode> presentModes;
        TL::Flags<SwapchainAlphaMode>   alphaModes;
        // TODO: move out (no allocations in the interface) or make it span to const Format that is owned by the swapchain?
        TL::Vector<Format>              formats;
    };

    struct SwapchainAcquireResult
    {
        Image* image = nullptr;
        Fence* fence = nullptr;
    };

    class RHI_EXPORT Queue
    {
    public:
        // Backends derive from this facade; m_impl is set to `this` so it always points at the
        // most-derived (backend) object, which RHI.cpp casts back to the backend type.
        Queue()
        {
            m_impl = (Impl*)this;
        }

        ~Queue() = default;

        void BeginAnnotation(const char* name, uint32_t bgra);
        void EndAnnotation();
        void InsertAnnotation(const char* name, uint32_t bgra);

        void Submit(const QueueSubmitInfo& submitInfo);

        void WaitIdle();
        void WaitFence(Fence* fence, uint64_t value);

    private:
        struct Impl* m_impl = nullptr;
    };

    class RHI_EXPORT Device
    {
    public:
        // Backends derive from this facade; m_impl is set to `this` so it always points at the
        // most-derived (backend) object, which RHI.cpp casts back to the backend type.
        Device()
        {
            m_impl = (Impl*)this;
        }

        ~Device() = default;

        BackendType                    GetBackend() const;

        DeviceFeatures                 GetFeatures() const;

        DeviceLimits                   GetLimits() const;

        uint64_t                       GarbageCollect(uint64_t graphicsTimeline);
        uint64_t                       GetNativeHandle(NativeHandleType type, uint64_t handle);

        Queue*                         GetQueue(QueueType queueType);

        // ShaderModule
        ShaderModule*                  CreateShaderModule(const ShaderModuleCreateInfo& createInfo);
        void                           DestroyShaderModule(ShaderModule* shaderModule);

        // BindGroupLayout
        BindGroupLayout*               CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo);
        void                           DestroyBindGroupLayout(BindGroupLayout* handle);

        // BindGroup
        BindGroup*                     CreateBindGroup(const BindGroupCreateInfo& createInfo);
        void                           DestroyBindGroup(BindGroup* handle);
        void                           UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo);

        // PipelineLayout
        PipelineLayout*                CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo);
        void                           DestroyPipelineLayout(PipelineLayout* handle);

        // Pipelines
        GraphicsPipeline*              CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo);
        void                           DestroyGraphicsPipeline(GraphicsPipeline* handle);
        ComputePipeline*               CreateComputePipeline(const ComputePipelineCreateInfo& createInfo);
        void                           DestroyComputePipeline(ComputePipeline* handle);
        RayTracingPipeline*            CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo);
        void                           DestroyRayTracingPipeline(RayTracingPipeline* handle);
        void                           GetShaderBindingTableEntry(RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle);

        // Buffer
        Buffer*                        CreateBuffer(const BufferCreateInfo& createInfo);
        void                           DestroyBuffer(Buffer* handle);
        uint64_t                       GetBufferDeviceAddress(Buffer* buffer);
        DeviceMemoryPtr                MapBuffer(Buffer* buffer, uint64_t offset, uint64_t sizeBytes);
        void                           UnmapBuffer(Buffer* buffer);

        // Image
        Image*                         CreateImage(const ImageCreateInfo& createInfo);
        Image*                         CreateImageView(const ImageViewCreateInfo& createInfo);
        void                           DestroyImage(Image* handle);

        // Sampler
        Sampler*                       CreateSampler(const SamplerCreateInfo& createInfo);
        void                           DestroySampler(Sampler* handle);

        // Acceleration structures
        AccelerationStructure*         CreateAccelerationStructure(const AccelerationStructureCreateInfo& createInfo);
        void                           DestroyAccelerationStructure(AccelerationStructure* handle);
        uint64_t                       GetAccelerationStructureDeviceAddress(AccelerationStructure* handle);
        AccelerationStructureSizesInfo GetAccelerationStructureSizesInfo(AccelerationStructure* as);
        Micromap*                      CreateMicromap(const MicromapCreateInfo& createInfo);
        void                           DestroyMicromap(Micromap* handle);

        // CommandPool
        CommandPool*                   CreateCommandPool(const CommandPoolCreateInfo& createInfo);
        void                           DestroyCommandPool(CommandPool* handle);

        // Fence
        Fence*                         CreateFence(const FenceCreateInfo& createInfo);
        void                           DestroyFence(Fence* handle);
        uint64_t                       GetFenceValue(Fence* handle);

        // QueryPool
        QueryPool*                     CreateQueryPool(const QueryPoolCreateInfo& createInfo);
        void                           DestroyQueryPool(QueryPool* handle);

        // Swapchain
        Swapchain*                     CreateSwapchain(const SwapchainCreateInfo& createInfo);
        void                           DestroySwapchain(Swapchain* swapchain);
        uint32_t                       GetSwapchainImagesCount(Swapchain* swapchain);
        SwapchainAcquireResult         AcquireSwapchainImage(Swapchain* swapchain);
        SurfaceCapabilities            GetSwapchainSurfaceCapabilities(Swapchain* swapchain);
        ResultCode                     ResizeSwapchain(Swapchain* swapchain, const ImageSize2D& size);
        ResultCode                     ConfigureSwapchain(Swapchain* swapchain, const SwapchainConfigureInfo& configInfo);

    private:
        struct Impl* m_impl = nullptr;
    };

    class RHI_EXPORT CommandPool
    {
    public:
        // Backends derive from this facade; m_impl is set to `this` so it always points at the
        // most-derived (backend) object, which RHI.cpp casts back to the backend type.
        CommandPool()
        {
            m_impl = (Impl*)this;
        }

        void         Reset();
        CommandList* Allocate();

        // Backend-internal implementation pointer. Assigned and read by the active backend
        // (see RHI/Backend/*). Not for consumer use.
        struct Impl* m_impl = nullptr;
    };

    class RHI_EXPORT CommandList
    {
    public:
        // Backends derive from this facade; m_impl is set to `this` so it always points at the
        // most-derived (backend) object, which RHI.cpp casts back to the backend type.
        CommandList()
        {
            m_impl = (Impl*)this;
        }

        ~CommandList() = default;

        // state
        void Begin();
        void End();

        // Debug markers
        void PushDebugMarker(const char* name, uint32_t bgra);
        void PopDebugMarker();
        void InsertDebugMarker(const char* name, uint32_t bgra);

        // Synchronization
        void AddPipelineBarrier(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers);

        // Pass setup
        void BeginRenderPass(const RenderPassBeginInfo& beginInfo);
        void EndRenderPass();
        void BeginComputePass(const ComputePassBeginInfo& beginInfo);
        void EndComputePass();

        // Conditional & device generated commands
        void BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted);
        void EndConditionalCommands();
        void Execute(TL::Span<const CommandList*> commandLists);

        // Pipeline state binding
        void BindPipelineLayout(BindPoint bindPoint, const PipelineLayout* pipelineLayout);
        void SetPushConstants(BindPoint bindPoint, uint32_t offset, TL::Block content);
        void PushBindGroup(BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos);
        void SetBindGroups(BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups);
        void BindGraphicsPipeline(const GraphicsPipeline* pipelineState);
        void BindComputePipeline(const ComputePipeline* pipelineState);
        void BindRayTracingPipeline(const RayTracingPipeline* pipelineState);

        // Dynamic state
        void SetViewport(float offsetX, float offsetY, float width, float height, float minDepth, float maxDepth);
        void SetScissor(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height);

        // Vertex input
        void BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers);
        void BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType);

        // Draw
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);
        void DrawMeshTasks(uint32_t x = 1, uint32_t y = 1, uint32_t z = 1);

        // Draw Indirect
        void DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride);
        void DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride);
        void DrawMeshTasksIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride);

        // Dispatch
        void Dispatch(uint32_t x = 1, uint32_t y = 1, uint32_t z = 1);
        void DispatchIndirect(const BufferBindingInfo& argumentBuffer);

        // Ray Tracing Dispatch
        void DispatchRays(const DispatchRaysInfo& dispatchRaysDesc);
        void DispatchRaysIndirect(const BufferBindingInfo& argumentBuffer);

        // Copy
        void CopyBuffer(const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size);
        void CopyImage(const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size);
        void CopyImageToBuffer(const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer);
        void CopyBufferToImage(const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout);
        void CopyAccelerationStructure(AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode);
        void CopyMicromap(Micromap* dst, const Micromap* src, CopyMode copyMode);

        // Acceleration structure & micromap builds
        void BuildTlas(TL::Span<const TlasBuildInfo> buildInfos);
        void BuildBlas(TL::Span<const BlasBuildInfo> buildInfos);
        void BuildMicromaps(TL::Span<const MicromapBuildInfo> buildInfos);
        void WriteAccelerationStructuresSizes(TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset);
        void WriteMicromapsSizes(TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset);

    private:
        struct Impl* m_impl = nullptr;
    };

    RHI_EXPORT const FormatInfo& GetFormatInfo(Format format);

    struct Version
    {
        uint16_t major = 0;
        uint16_t minor = 0;
        uint32_t patch = 0;
    };

    struct ApplicationInfo
    {
        const char* applicationName    = nullptr;
        Version     applicationVersion = {};
        const char* engineName         = nullptr;
        Version     engineVersion      = {};
    };

    RHI_EXPORT Device* CreateVulkanDevice(const ApplicationInfo& appInfo);
    RHI_EXPORT void    DestroyVulkanDevice(Device* device);
} // namespace RHI
