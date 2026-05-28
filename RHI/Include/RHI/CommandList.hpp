#pragma once

#include "RHI/Resources.hpp"

#include <TL/Containers/Vector.hpp>
#include <TL/Span.hpp>

namespace RHI
{
    class CommandList;

    // Enums

    // TODO: Should move from here
    enum class QueueType : uint8_t
    {
        Graphics,
        Compute,
        Transfer,
        Count,
    };

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
        Discard,
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

    // Clear values

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

    // Pipeline dynamic states

    struct Viewport
    {
        float offsetX  = 0.0f;
        float offsetY  = 0.0f;
        float width    = 1.0f;
        float height   = 1.0f;
        float minDepth = 0.0f;
        float maxDepth = 1.0f;
    };

    struct Scissor
    {
        int32_t  offsetX = 0;
        int32_t  offsetY = 0;
        uint32_t width   = UINT32_MAX;
        uint32_t height  = UINT32_MAX;
    };

    // Copy descriptors

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

    // Binding

    struct BindGroupBindingInfo
    {
        BindGroup*               bindGroup      = nullptr; ///< Pointer to the bind group.
        TL::Span<const uint32_t> dynamicOffsets = {};      ///< Span of dynamic offsets for the bind group.
    };

    // Barriers

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
        Image*            image    = nullptr;
        ImageBarrierState srcState = {};
        ImageBarrierState dstState = {};
    };

    struct BufferBarrierInfo
    {
        Buffer*            buffer    = nullptr;
        BufferBarrierState srcState  = {};
        BufferBarrierState dstState  = {};
        BufferSubregion    subregion = {};
    };

    // Pass begin infos

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

    // Dispatch (ray tracing)

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

    // AS / micromap build

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

    // Command list / pool

    struct CommandListCreateInfo
    {
        const char* name      = nullptr;             ///< Name of the command list.
        QueueType   queueType = QueueType::Graphics; ///< Type of queue for the command list.
    };

    struct CommandPoolCreateInfo
    {
        const char* name;
        QueueType   queue;
    };

    class RHI_EXPORT CommandPool
    {
    public:
        virtual void         Reset()    = 0;
        virtual CommandList* Allocate() = 0;
    };

    class RHI_EXPORT CommandList
    {
    public:
        CommandList()          = default;
        virtual ~CommandList() = default;

        // Lifecycle
        virtual void Begin() = 0;
        virtual void End()   = 0;

        // Debug markers
        virtual void PushDebugMarker(const char* name, uint32_t bgra)   = 0;
        virtual void PopDebugMarker()                                   = 0;
        virtual void InsertDebugMarker(const char* name, uint32_t bgra) = 0;

        // Synchronization
        virtual void AddPipelineBarrier(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers) = 0;

        // Pass setup
        virtual void BeginRenderPass(const RenderPassBeginInfo& beginInfo)   = 0;
        virtual void EndRenderPass()                                         = 0;
        virtual void BeginComputePass(const ComputePassBeginInfo& beginInfo) = 0;
        virtual void EndComputePass()                                        = 0;

        // Conditional & device generated commands
        virtual void BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted) = 0;
        virtual void EndConditionalCommands()                                                          = 0;
        virtual void Execute(TL::Span<const CommandList*> commandLists)                                = 0;

        // Pipeline state binding
        virtual void BindPipelineLayout(BindPoint bindPoint, const PipelineLayout* pipelineLayout)                            = 0;
        virtual void SetPushConstants(BindPoint bindPoint, uint32_t offset, TL::Block content)                                = 0;
        virtual void PushBindGroup(BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos) = 0;
        virtual void SetBindGroups(BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups)                      = 0;
        virtual void BindGraphicsPipeline(const GraphicsPipeline* pipelineState)                                              = 0;
        virtual void BindComputePipeline(const ComputePipeline* pipelineState)                                                = 0;
        virtual void BindRayTracingPipeline(const RayTracingPipeline* pipelineState)                                          = 0;

        // Dynamic state
        virtual void SetViewport(const Viewport& viewport) = 0;
        virtual void SetScissor(const Scissor& sicssor)    = 0;

        // Vertex input
        virtual void BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers) = 0;
        virtual void BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType)                = 0;

        // Draw
        virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0)                                = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) = 0;
        virtual void DrawMeshTasks(uint32_t x = 1, uint32_t y = 1, uint32_t z = 1)                                                                               = 0;

        // Draw Indirect
        virtual void DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)        = 0;
        virtual void DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) = 0;
        virtual void DrawMeshTasksIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride)    = 0;

        // Dispatch
        virtual void Dispatch(uint32_t x = 1, uint32_t y = 1, uint32_t z = 1)  = 0;
        virtual void DispatchIndirect(const BufferBindingInfo& argumentBuffer) = 0;

        // Ray Tracing Dispatch
        virtual void DispatchRays(const DispatchRaysInfo& dispatchRaysDesc)        = 0;
        virtual void DispatchRaysIndirect(const BufferBindingInfo& argumentBuffer) = 0;

        // Copy
        virtual void CopyBuffer(const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size) = 0;
        virtual void CopyImage(const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size)                    = 0;
        virtual void CopyImageToBuffer(const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer)          = 0;
        virtual void CopyBufferToImage(const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout)          = 0;
        virtual void CopyAccelerationStructure(AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode)          = 0;
        virtual void CopyMicromap(Micromap* dst, const Micromap* src, CopyMode copyMode)                                                 = 0;

        // Acceleration structure & micromap builds
        virtual void BuildTlas(TL::Span<const TlasBuildInfo> buildInfos)                                                                                             = 0;
        virtual void BuildBlas(TL::Span<const BlasBuildInfo> buildInfos)                                                                                             = 0;
        virtual void BuildMicromaps(TL::Span<const MicromapBuildInfo> buildInfos)                                                                                    = 0;
        virtual void WriteAccelerationStructuresSizes(TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset) = 0;
        virtual void WriteMicromapsSizes(TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset)                                        = 0;
    };
} // namespace RHI
