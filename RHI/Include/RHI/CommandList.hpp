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

    struct MicromapBuildInfo
    {
        const char* name = nullptr;
    };

    struct TopLevelAccelerationStructureBuildInfo
    {
        const char* name = nullptr;
    };

    struct BottomLevelAccelerationStructureBuildInfo
    {
        const char* name = nullptr;
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

    /// @brief Represents a list of commands to be executed on the GPU.
    class RHI_EXPORT CommandList
    {
    public:
        CommandList()          = default;
        virtual ~CommandList() = default;

        /// @brief Begins recording commands to the command list.
        virtual void Begin() = 0;

        /// @brief Ends recording commands to the command list.
        virtual void End() = 0;

        /// @brief Represents a list of commands to be executed.
        /// @brief Adds pipeline barriers to synchronize resource state transitions.
        /// @param barriers Array of memory barrier states to synchronize memory access.
        /// @param imageBarriers Array of image barrier information for image state transitions.
        /// @param bufferBarriers Array of buffer barrier information for buffer state transitions.
        virtual void AddPipelineBarrier(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers) = 0;

        /// @brief Begins a render pass for drawing commands.
        /// @param beginInfo Information needed to begin the render pass.
        virtual void BeginRenderPass(const RenderPassBeginInfo& beginInfo) = 0;

        /// @brief Ends the current render pass.
        virtual void EndRenderPass() = 0;

        /// @brief Begins a compute pass for dispatch commands.
        /// @param beginInfo Information needed to begin the compute pass.
        virtual void BeginComputePass(const ComputePassBeginInfo& beginInfo) = 0;

        /// @brief Ends the current compute pass.
        virtual void EndComputePass() = 0;

        /// @brief Pushes a debug marker with a name and color onto the command list.
        /// @param name Name of the debug marker.
        /// @param color Color value of the debug marker.
        virtual void PushDebugMarker(const char* name, uint32_t bgra) = 0;

        /// @brief Pops the last debug marker off the command list.
        virtual void PopDebugMarker() = 0;

        /// @brief Inserts a debug marker at the current position in the command list.
        /// @param name Name of the debug marker.
        /// @param bgra Color value of the debug marker in BGRA format.
        virtual void InsertDebugMarker(const char* name, uint32_t bgra) = 0;

        /// @brief Begins a conditional command block based on a buffer.
        /// @param conditionBuffer Binding information for the condition buffer.
        /// @param inverted If true, the condition is inverted.
        virtual void BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted) = 0;

        /// @brief Ends a conditional command block.
        virtual void EndConditionalCommands() = 0;

        /// @brief Executes a set of command lists.
        /// @param commandLists Span of command lists to execute.
        virtual void Execute(TL::Span<const CommandList*> commandLists) = 0;

        /// @brief Binds a pipeline layout for resource binding.
        /// @param bindPoint The bind point (graphics or compute) for which to bind the layout.
        /// @param pipelineLayout Pointer to the pipeline layout to bind.
        virtual void BindPipelineLayout(BindPoint bindPoint, const PipelineLayout* pipelineLayout) = 0;

        /// @brief Sets push constant values for the current pipeline layout.
        /// @param bindPoint The bind point (graphics or compute) for which to set constants.
        /// @param offset Offset in bytes within the push constant range.
        /// @param content Block of data containing the push constant values.
        virtual void SetPushConstants(BindPoint bindPoint, uint32_t offset, TL::Block content) = 0;

        /// @brief Pushes bind group updates to the command list.
        /// @param bindPoint The bind point (graphics or compute) for which to update bind groups.
        /// @param firstGroup Index of the first bind group to update.
        /// @param updateInfos Span of bind group update information.
        virtual void PushBindGroup(BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos) = 0;

        /// @brief Sets bind groups for resource binding.
        /// @param bindPoint The bind point (graphics or compute) for which to set bind groups.
        /// @param bindGroups Span of bind group binding information to set.
        virtual void SetBindGroups(BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups) = 0;

        /// @brief Binds a graphics pipeline.
        /// @param pipelineState Pointer to the graphics pipeline.
        /// @param bindGroups Span of bind group binding information.
        virtual void BindGraphicsPipeline(const GraphicsPipeline* pipelineState) = 0;

        /// @brief Binds a compute pipeline.
        /// @param pipelineState Pointer to the compute pipeline.
        /// @param bindGroups Span of bind group binding information.
        virtual void BindComputePipeline(const ComputePipeline* pipelineState) = 0;

        /// @brief Sets the viewport for rendering.
        /// @param viewport The viewport to set.
        virtual void SetViewport(const Viewport& viewport) = 0;

        /// @brief Sets the scissor rectangle for rendering.
        /// @param scissor The scissor rectangle to set.
        virtual void SetScissor(const Scissor& sicssor) = 0;

        /// @brief Binds the vertex buffers for drawing.
        /// @param firstBinding Index of the first binding.
        /// @param vertexBuffers Span of vertex buffer binding information.
        virtual void BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers) = 0;

        /// @brief Binds the index buffer for drawing.
        /// @param indexBuffer Information about the index buffer binding.
        /// @param indexType Type of indices in the index buffer.
        virtual void BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType) = 0;

        /// @brief Issues a draw command.
        /// @param parameters Parameters for the draw command.
        virtual void Draw(const DrawParameters& parameters) = 0;

        /// @brief Issues an indexed draw command.
        /// @param parameters Parameters for the indexed draw command.
        virtual void DrawIndexed(const DrawIndexedParameters& parameters) = 0;

        /// @brief Issues an indirect draw command.
        /// @param argumentBuffer Binding information about the buffer containing draw arguments.
        /// @param countBuffer Binding information about the buffer containing draw counts.
        /// @param maxDrawCount Maximum number of draws to issue in this indirect draw call.
        /// @param stride Stride between draw commands in bytes.
        virtual void DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) = 0;

        /// @brief Issues an indexed indirect draw command.
        /// @param argumentBuffer Binding information about the buffer containing draw arguments.
        /// @param countBuffer Binding information about the buffer containing draw counts.
        /// @param maxDrawCount Maximum number of draws to issue in this indirect draw call.
        /// @param stride Stride between draw commands in bytes.
        virtual void DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) = 0;

        /// @brief Issues a mesh task drawing command.
        /// @param drawMeshTasksDesc Information for the mesh task drawing command.
        virtual void DrawMeshTasks(const DispatchParameters drawMeshTasksDesc) = 0;

        /// @brief Issues an indirect mesh task drawing command.
        /// @param argumentBuffer Binding information about the buffer containing draw arguments.
        /// @param countBuffer Binding information about the buffer containing draw counts.
        /// @param drawNum Number of draws to issue.
        /// @param stride Stride between draw commands in bytes.
        virtual void DrawMeshTasksIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride) = 0;

        /// @brief Issues a ray tracing dispatch command.
        /// @param dispatchRaysDesc Information for the ray tracing dispatch command.
        virtual void DispatchRays(const DispatchRaysInfo& dispatchRaysDesc) = 0;

        /// @brief Issues an indirect ray tracing dispatch command.
        /// @param argumentBuffer Binding information about the buffer containing dispatch arguments.
        virtual void DispatchRaysIndirect(const BufferBindingInfo& argumentBuffer) = 0;

        /// @brief Issues a compute dispatch command.
        /// @param parameters Information for the dispatch command.
        virtual void Dispatch(const DispatchParameters& parameters)            = 0;
        virtual void DispatchIndirect(const BufferBindingInfo& argumentBuffer) = 0;

        virtual void CopyBuffer(const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size) = 0;
        virtual void CopyImage(const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size)                    = 0;
        virtual void CopyImageToBuffer(const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer)                = 0;
        virtual void CopyBufferToImage(const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout)                = 0;
        virtual void CopyMicromap(Micromap* dst, const Micromap* src, CopyMode copyMode)                                                 = 0;
        virtual void CopyAccelerationStructure(AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode)          = 0;

        /// @brief Builds micromaps for ray tracing acceleration structures.
        /// @param buildInfos Span of micromap build descriptions.
        virtual void BuildMicromaps(TL::Span<const MicromapBuildInfo> buildInfos) = 0;

        /// @brief Writes micromap sizes to a query pool.
        /// @param micromaps Span of micromaps to query.
        /// @param queryPool Query pool to write the sizes to.
        /// @param queryPoolOffset Offset in the query pool to write to.
        virtual void WriteMicromapsSizes(TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset) = 0;

        /// @brief Builds top-level acceleration structures for ray tracing.
        /// @param buildInfos Span of TLAS build descriptions.
        virtual void BuildTopLevelAccelerationStructures(TL::Span<const TopLevelAccelerationStructureBuildInfo> buildInfos) = 0;

        /// @brief Builds bottom-level acceleration structures for ray tracing.
        /// @param buildInfos Span of BLAS build descriptions.
        virtual void BuildBottomLevelAccelerationStructures(TL::Span<const BottomLevelAccelerationStructureBuildInfo> buildInfos) = 0;

        /// @brief Writes acceleration structure sizes to a query pool.
        /// @param accelerationStructures Span of acceleration structures to query.
        /// @param queryPool Query pool to write the sizes to.
        /// @param queryPoolOffset Offset in the query pool to write to.
        virtual void WriteAccelerationStructuresSizes(TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset) = 0;
    };
} // namespace RHI
