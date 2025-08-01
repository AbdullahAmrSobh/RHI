#pragma once

#include "RHI/Common.hpp"
#include "RHI/Resources.hpp"

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

namespace RHI
{
    struct GraphicsPipeline;
    struct ComputePipeline;

    class RenderGraph;

    enum class QueueType : uint8_t
    {
        Graphics,
        Compute,
        Transfer,
        Count,
    };

    inline static constexpr uint32_t AsyncQueuesCount = (uint32_t)QueueType::Count;

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

    struct DepthStencilValue
    {
        float   depthValue   = 1.0f;
        uint8_t stencilValue = 0u;
    };

    template<typename T>
    struct ColorValue
    {
        T r = 1;
        T g = 1;
        T b = 1;
        T a = 1;
    };

    union ClearValue
    {
        ColorValue<uint8_t>  u8;
        ColorValue<uint16_t> u16;
        ColorValue<uint32_t> u32;
        ColorValue<float>    f32;
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

    /// @brief Contains information needed to copy a buffer.
    struct BufferCopyInfo
    {
        Handle<Buffer> srcBuffer = NullHandle; ///< Handle to the source buffer.
        size_t         srcOffset = 0;          ///< Offset in the source buffer.
        Handle<Buffer> dstBuffer = NullHandle; ///< Handle to the destination buffer.
        size_t         dstOffset = 0;          ///< Offset in the destination buffer.
        size_t         size      = SIZE_MAX;   ///< Size of the data to copy.
    };

    /// @brief Defines subresource layers of an image.
    struct ImageSubresourceLayers
    {
        TL::Flags<ImageAspect> imageAspects = ImageAspect::All; ///< Image aspects to access.
        uint32_t               mipLevel     = 0;                ///< Mipmap level.
        uint32_t               arrayBase    = 0;                ///< Base array layer.
        uint32_t               arrayCount   = 1;                ///< Number of array layers.
    };

    /// @brief Contains information needed to copy an image.
    struct ImageCopyInfo
    {
        Handle<Image>          srcImage       = NullHandle; ///< Handle to the source image.
        ImageSubresourceLayers srcSubresource = {};         ///< Subresource layers of the source image.
        ImageOffset3D          srcOffset      = {};         ///< Offset in the source image.
        ImageSize3D            srcSize        = {};         ///< Size of the source image region.
        Handle<Image>          dstImage       = NullHandle; ///< Handle to the destination image.
        ImageSubresourceLayers dstSubresource = {};         ///< Subresource layers of the destination image.
        ImageOffset3D          dstOffset      = {};         ///< Offset in the destination image.
    };

    /// @brief Contains information needed to copy between a buffer and an image.
    struct BufferImageCopyInfo
    {
        Handle<Image>          image         = NullHandle; ///< Handle to the image.
        ImageSubresourceLayers subresource   = {};         ///< Subresource layers of the image.
        ImageSize3D            imageSize     = {};         ///< Size of the image.
        ImageOffset3D          imageOffset   = {};         ///< Offset in the image.
        Handle<Buffer>         buffer        = NullHandle; ///< Handle to the buffer.
        size_t                 bufferOffset  = 0;          ///< Offset in the buffer.
        size_t                 bufferSize    = SIZE_MAX;   ///< Size of the buffer region.
        uint32_t               bytesPerRow   = 0;          ///< Number of bytes per row in the buffer.
        uint32_t               bytesPerImage = 0;          ///< Number of bytes per image in the buffer.
    };

    /// @brief Contains information about binding a bind group.
    struct BindGroupBindingInfo
    {
        Handle<BindGroup>        bindGroup      = NullHandle; ///< Handle to the bind group.
        TL::Span<const uint32_t> dynamicOffsets = {};         ///< Span of dynamic offsets for the bind group.
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
        uint32_t countX = 16; ///< Number of work groups in X dimension.
        uint32_t countY = 16; ///< Number of work groups in Y dimension.
        uint32_t countZ = 16; ///< Number of work groups in Z dimension.
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
        Handle<Image>     image    = NullHandle;
        ImageBarrierState srcState = {};
        ImageBarrierState dstState = {};
    };

    struct BufferBarrierInfo
    {
        Handle<Buffer>     buffer    = NullHandle;
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
        Handle<Image>  view        = NullHandle;
        LoadOperation  loadOp      = LoadOperation::Discard;
        StoreOperation storeOp     = StoreOperation::Store;
        ClearValue     clearValue  = {.f32 = {0.0f, 0.0f, 0.0f, 1.0f}};
        ResolveMode    resolveMode = ResolveMode::None;
        Handle<Image>  resolveView = NullHandle;
    };

    struct DepthStencilAttachment
    {
        Handle<Image>     view           = NullHandle;
        LoadOperation     depthLoadOp    = LoadOperation::Discard;
        StoreOperation    depthStoreOp   = StoreOperation::Store;
        LoadOperation     stencilLoadOp  = LoadOperation::Discard;
        StoreOperation    stencilStoreOp = StoreOperation::Store;
        DepthStencilValue clearValue     = {0.0f, 0};
    };

    struct RenderPassBeginInfo
    {
        ImageSize2D                          size;
        ImageOffset2D                        offset;
        TL::Span<const ColorAttachment>      colorAttachments;
        TL::Optional<DepthStencilAttachment> depthStencilAttachment;
    };

    /// @brief Contains information for creating a command list.
    struct CommandListCreateInfo
    {
        const char* name      = nullptr;             ///< Name of the command list.
        QueueType   queueType = QueueType::Graphics; ///< Type of queue for the command list.
    };

    class RHI_EXPORT CommandList
    {
    public:
        RHI_INTERFACE_BOILERPLATE(CommandList);

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
        virtual void PushDebugMarker(const char* name, ClearValue color) = 0;

        /// @brief Pops the last debug marker off the command list.
        virtual void PopDebugMarker() = 0;

        /// @brief Begins a conditional command block based on a buffer.
        /// @param conditionBuffer Binding information for the condition buffer.
        /// @param inverted If true, the condition is inverted.
        virtual void BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted) = 0;

        /// @brief Ends a conditional command block.
        virtual void EndConditionalCommands() = 0;

        /// @brief Executes a set of command lists.
        /// @param commandLists Span of command lists to execute.
        virtual void Execute(TL::Span<const CommandList*> commandLists) = 0;

        /// @brief Binds a graphics pipeline.
        /// @param pipelineState Handle to the graphics pipeline.
        /// @param bindGroups Span of bind group binding information.
        virtual void BindGraphicsPipeline(Handle<GraphicsPipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups) = 0;

        /// @brief Binds a compute pipeline.
        /// @param pipelineState Handle to the compute pipeline.
        /// @param bindGroups Span of bind group binding information.
        virtual void BindComputePipeline(Handle<ComputePipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups) = 0;

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

        /// @brief Issues a dispatch command.
        /// @param parameters Information for the dispatch command.
        virtual void Dispatch(const DispatchParameters& parameters) = 0;

        /// @brief Issues an indexed dispatch command.
        /// @param parameters Information for the dispatch command.
        virtual void DispatchIndirect(const BufferBindingInfo& argumentBuffer) = 0;

        /// @brief Issues a buffer-to-buffer copy command.
        /// @param copyInfo Information for the buffer copy command.
        virtual void CopyBuffer(const BufferCopyInfo& copyInfo) = 0;

        /// @brief Issues an image-to-image copy command.
        /// @param copyInfo Information for the image copy command.
        virtual void CopyImage(const ImageCopyInfo& copyInfo) = 0;

        /// @brief Issues a buffer-to-image copy command.
        /// @param copyInfo Information for the buffer-to-image copy command.
        virtual void CopyImageToBuffer(const BufferImageCopyInfo& copyInfo) = 0;

        /// @brief Issues an image-to-buffer copy command.
        /// @param copyInfo Information for the image-to-buffer copy command.
        virtual void CopyBufferToImage(const BufferImageCopyInfo& copyInfo) = 0;
    };
} // namespace RHI
