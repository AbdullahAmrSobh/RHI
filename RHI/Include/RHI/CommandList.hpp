#pragma once

#include "RHI/Resources.hpp"

#include <TL/Containers/Vector.hpp>
#include <TL/Span.hpp>

namespace RHI
{
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

    /// @brief Defines a viewport for rendering.
    struct Viewport
    {
        float offsetX  = 0.0f; ///< X offset of the viewport.
        float offsetY  = 0.0f; ///< Y offset of the viewport.
        float width    = 1.0f; ///< Width of the viewport.
        float height   = 1.0f; ///< Height of the viewport.
        float minDepth = 0.0f; ///< Minimum depth value of the viewport.
        float maxDepth = 1.0f; ///< Maximum depth value of the viewport.
    };

    /// @brief Defines a scissor rectangle for rendering.
    struct Scissor
    {
        int32_t  offsetX = 0;          ///< X offset of the scissor rectangle.
        int32_t  offsetY = 0;          ///< Y offset of the scissor rectangle.
        uint32_t width   = UINT32_MAX; ///< Width of the scissor rectangle.
        uint32_t height  = UINT32_MAX; ///< Height of the scissor rectangle.
    };

    struct ImageMemoryLayout
    {
        uint64_t offset;
        uint32_t bytesPerRow;
        uint32_t rowsPerImage;
    };

    /// @brief Contains information needed to copy an image.
    struct ImageCopyInfo
    {
        Image*        image      = nullptr; ///< Pointer to the source image.
        uint32_t      mipLevel   = 0;       ///< Mipmap level of the source image.
        uint32_t      arrayLayer = 0;       ///< Array layer of the source image
        ImageOffset3D offset     = {};      ///< Offset in the source image.
        ImageAspect   aspect     = ImageAspect::All;
    };

    /// @brief Contains information about binding a bind group.
    struct BindGroupBindingInfo
    {
        BindGroup*               bindGroup      = nullptr; ///< Pointer to the bind group.
        TL::Span<const uint32_t> dynamicOffsets = {};      ///< Span of dynamic offsets for the bind group.
    };

    /// @brief Parameters for drawing primitives.
    struct DrawParameters
    {
        uint32_t vertexCount   = 0; ///< Number of vertices to draw.
        uint32_t instanceCount = 1; ///< Number of instances to draw.
        uint32_t firstVertex   = 0; ///< Index of the first vertex to draw.
        uint32_t firstInstance = 0; ///< Index of the first instance to draw.
    };

    /// @brief Parameters for drawing indexed primitives.
    struct DrawIndexedParameters
    {
        uint32_t indexCount    = 0; ///< Number of indices to draw.
        uint32_t instanceCount = 1; ///< Number of instances to draw.
        uint32_t firstIndex    = 0; ///< Index of the first index to draw.
        int32_t  vertexOffset  = 0; ///< Offset added to each index.
        uint32_t firstInstance = 0; ///< Index of the first instance to draw.
    };

    /// @brief Parameters for dispatching compute work.
    struct DispatchParameters
    {
        uint32_t x = 1;
        uint32_t y = 1;
        uint32_t z = 1;
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

    /// @brief Contains information for creating a command list.
    struct CommandListCreateInfo
    {
        const char* name      = nullptr;             ///< Name of the command list.
        QueueType   queueType = QueueType::Graphics; ///< Type of queue for the command list.
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

    class CommandList;

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

    struct MicromapBuildInfo
    {
    };

    struct TlasBuildInfo
    {
        AccelerationStructure* src;
        AccelerationStructure* dst;
        uint32_t              instanceCount;
        Buffer*               instanceBuffer;
        uint32_t              instanceBufferOffset;
        Buffer*               scratchBuffer;
        uint32_t              scratchBufferOffset;
    };

    struct BlasBuildInfo
    {
        AccelerationStructure* src;
        AccelerationStructure* dst;
        uint32_t              geometryCount;
        const AccelerationStructureGeometry* geometries;
        Buffer*               scratchBuffer;
        uint32_t              scratchBufferOffset;
    };

    class RHI_EXPORT CommandList
    {
    public:
        CommandList()          = default;
        virtual ~CommandList() = default;

        virtual void Begin() = 0;
        virtual void End()   = 0;

        virtual void AddPipelineBarrier(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers) = 0;

        virtual void BeginRenderPass(const RenderPassBeginInfo& beginInfo) = 0;
        virtual void EndRenderPass()                                       = 0;

        virtual void BeginComputePass(const ComputePassBeginInfo& beginInfo) = 0;
        virtual void EndComputePass()                                        = 0;

        virtual void PushDebugMarker(const char* name, uint32_t bgra)   = 0;
        virtual void PopDebugMarker()                                   = 0;
        virtual void InsertDebugMarker(const char* name, uint32_t bgra) = 0;

        virtual void BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted) = 0;
        virtual void EndConditionalCommands()                                                          = 0;

        virtual void Execute(TL::Span<const CommandList*> commandLists) = 0;

        virtual void BindPipelineLayout(BindPoint bindPoint, const PipelineLayout* pipelineLayout)                            = 0;
        virtual void SetPushConstants(BindPoint bindPoint, uint32_t offset, TL::Block content)                                = 0;
        virtual void PushBindGroup(BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos) = 0;
        virtual void SetBindGroups(BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups)                      = 0;
        virtual void BindGraphicsPipeline(const GraphicsPipeline* pipelineState)                                              = 0;
        virtual void BindComputePipeline(const ComputePipeline* pipelineState)                                                = 0;
        virtual void BindRayTracingPipeline(const RayTracingPipeline* pipelineState)                                          = 0;

        virtual void SetViewport(const Viewport& viewport) = 0;
        virtual void SetScissor(const Scissor& sicssor)    = 0;

        virtual void BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers) = 0;
        virtual void BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType)                = 0;

        virtual void Draw(const DrawParameters& parameters)                                                                                                     = 0;
        virtual void DrawIndexed(const DrawIndexedParameters& parameters)                                                                                       = 0;
        virtual void DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)        = 0;
        virtual void DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) = 0;
        virtual void DrawMeshTasks(const DispatchParameters drawMeshTasksDesc)                                                                                  = 0;
        virtual void DrawMeshTasksIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride)    = 0;

        virtual void DispatchRays(const DispatchRaysInfo& dispatchRaysDesc)        = 0;
        virtual void DispatchRaysIndirect(const BufferBindingInfo& argumentBuffer) = 0;

        virtual void Dispatch(const DispatchParameters& parameters)            = 0;
        virtual void DispatchIndirect(const BufferBindingInfo& argumentBuffer) = 0;

        virtual void CopyBuffer(const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size) = 0;
        virtual void CopyImage(const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size)                    = 0;
        virtual void CopyImageToBuffer(const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer)          = 0;
        virtual void CopyBufferToImage(const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout)          = 0;

        virtual void CopyAccelerationStructure(AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode)          = 0;
        virtual void CopyMicromap(Micromap* dst, const Micromap* src, CopyMode copyMode)                                                 = 0;

        virtual void BuildTlas(TL::Span<const TlasBuildInfo> buildInfos)          = 0;
        virtual void BuildBlas(TL::Span<const BlasBuildInfo> buildInfos)          = 0;
        virtual void BuildMicromaps(TL::Span<const MicromapBuildInfo> buildInfos) = 0;

        virtual void WriteAccelerationStructuresSizes(TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset) = 0;
        virtual void WriteMicromapsSizes(TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset)                                        = 0;
    };
} // namespace RHI
