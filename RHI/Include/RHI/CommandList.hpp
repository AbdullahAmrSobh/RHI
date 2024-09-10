#pragma once

#include "RHI/RenderGraph.hpp"
#include "RHI/RGPass.hpp"
#include "RHI/Queue.hpp"

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

namespace RHI
{
    struct GraphicsPipeline;
    struct ComputePipeline;

    class Pass;
    class CommandList;
    class Pass;
    template<typename T>
    struct ColorValue;

    enum class IndexType
    {
        uint16,
        uint32,
    };

    /// @brief Flags for command pool configuration.
    enum class CommandPoolFlags
    {
        None      = 0,    ///< No flags set.
        Transient = 0x01, ///< The command pool is transient.
        Reset     = 0x02, ///< The command pool supports resetting.
    };

    /// @brief Specifies the level of a command list.
    enum class CommandListLevel
    {
        Primary,   ///< Primary command list, which can be executed directly.
        Secondary, ///< Secondary command list, which can be executed within a primary command list.
    };

    /// @brief Defines a viewport for rendering.
    struct Viewport
    {
        float offsetX;  ///< X offset of the viewport.
        float offsetY;  ///< Y offset of the viewport.
        float width;    ///< Width of the viewport.
        float height;   ///< Height of the viewport.
        float minDepth; ///< Minimum depth value of the viewport.
        float maxDepth; ///< Maximum depth value of the viewport.
    };

    /// @brief Defines a scissor rectangle for rendering.
    struct Scissor
    {
        int32_t  offsetX; ///< X offset of the scissor rectangle.
        int32_t  offsetY; ///< Y offset of the scissor rectangle.
        uint32_t width;   ///< Width of the scissor rectangle.
        uint32_t height;  ///< Height of the scissor rectangle.
    };

    /// @brief Contains information needed to begin recording commands in a command list.
    struct CommandListBeginInfo
    {
        RenderGraph*                        renderGraph;         ///< Pointer to the render graph.
        Handle<Pass>                        pass;                ///< Handle to the pass.
        TL::Span<const LoadStoreOperations> loadStoreOperations; ///< Span of load/store operations.
    };

    /// @brief Contains information needed to copy a buffer.
    struct BufferCopyInfo
    {
        Handle<Buffer> srcBuffer; ///< Handle to the source buffer.
        size_t         srcOffset; ///< Offset in the source buffer.
        Handle<Buffer> dstBuffer; ///< Handle to the destination buffer.
        size_t         dstOffset; ///< Offset in the destination buffer.
        size_t         size;      ///< Size of the data to copy.
    };

    /// @brief Defines subresource layers of an image.
    struct ImageSubresourceLayers
    {
        TL::Flags<ImageAspect> imageAspects; ///< Image aspects to access.
        uint32_t               mipLevel;     ///< Mipmap level.
        uint32_t               arrayBase;    ///< Base array layer.
        uint32_t               arrayCount;   ///< Number of array layers.
    };

    /// @brief Contains information needed to copy an image.
    struct ImageCopyInfo
    {
        Handle<Image>          srcImage;       ///< Handle to the source image.
        ImageSubresourceLayers srcSubresource; ///< Subresource layers of the source image.
        ImageOffset3D          srcOffset;      ///< Offset in the source image.
        ImageSize3D            srcSize;        ///< Size of the source image region.
        Handle<Image>          dstImage;       ///< Handle to the destination image.
        ImageSubresourceLayers dstSubresource; ///< Subresource layers of the destination image.
        ImageOffset3D          dstOffset;      ///< Offset in the destination image.
    };

    /// @brief Contains information needed to copy between a buffer and an image.
    struct BufferImageCopyInfo
    {
        Handle<Image>          image;         ///< Handle to the image.
        ImageSubresourceLayers subresource;   ///< Subresource layers of the image.
        ImageSize3D            imageSize;     ///< Size of the image.
        ImageOffset3D          imageOffset;   ///< Offset in the image.
        Handle<Buffer>         buffer;        ///< Handle to the buffer.
        size_t                 bufferOffset;  ///< Offset in the buffer.
        size_t                 bufferSize;    ///< Size of the buffer region.
        uint32_t               bytesPerRow;   ///< Number of bytes per row in the buffer.
        uint32_t               bytesPerImage; ///< Number of bytes per image in the buffer.
    };

    /// @brief Contains information about binding a buffer.
    struct BufferBindingInfo
    {
        Handle<Buffer> buffer; ///< Handle to the buffer.
        size_t         offset; ///< Offset into the buffer.
    };

    /// @brief Contains information about binding a bind group.
    struct BindGroupBindingInfo
    {
        Handle<BindGroup>        bindGroup;      ///< Handle to the bind group.
        TL::Span<const uint32_t> dynamicOffsets; ///< Span of dynamic offsets for the bind group.
    };

    /// @brief Parameters for drawing primitives.
    struct DrawParameters
    {
        uint32_t elementsCount; ///< Number of elements to draw.
        uint32_t instanceCount; ///< Number of instances to draw.
        uint32_t firstElement;  ///< Index of the first element to draw.
        int32_t  vertexOffset;  ///< Offset in the vertex buffer.
        uint32_t firstInstance; ///< Index of the first instance to draw.
    };

    /// @brief Parameters for dispatching compute work.
    struct DispatchParameters
    {
        uint32_t offsetX; ///< X offset for the dispatch.
        uint32_t offsetY; ///< Y offset for the dispatch.
        uint32_t offsetZ; ///< Z offset for the dispatch.
        uint32_t countX;  ///< Number of work groups in X dimension.
        uint32_t countY;  ///< Number of work groups in Y dimension.
        uint32_t countZ;  ///< Number of work groups in Z dimension.
    };

    /// @brief Contains information about a draw command.
    struct DrawInfo
    {
        Handle<GraphicsPipeline>             pipelineState; ///< Handle to the graphics pipeline.
        TL::Span<const BindGroupBindingInfo> bindGroups;    ///< Span of bind group bindings.
        TL::Span<const BufferBindingInfo>    vertexBuffers; ///< Span of vertex buffer bindings.
        BufferBindingInfo                    indexBuffer;   ///< Index buffer binding.
        DrawParameters                       parameters;    ///< Draw parameters.
    };

    /// @brief Contains information about a dispatch command.
    struct DispatchInfo
    {
        Handle<ComputePipeline>              pipelineState; ///< Handle to the compute pipeline.
        TL::Span<const BindGroupBindingInfo> bindGroups;    ///< Span of bind group bindings.
        DispatchParameters                   parameters;    ///< Dispatch parameters.
    };

    /// @brief Contains information needed to blit (copy and scale) an image.
    struct ImageBlitInfo
    {
        ImageSubresourceLayers srcSubresource; ///< Source subresource layers.
        ImageOffset3D          srcOffsets[2];  ///< Source offsets (top-left and bottom-right).
        ImageSubresourceLayers dstSubresource; ///< Destination subresource layers.
        ImageOffset3D          dstOffsets[2];  ///< Destination offsets (top-left and bottom-right).
    };

    /// @brief Represents a pool of command lists.
    class RHI_EXPORT CommandPool
    {
    public:
        CommandPool()                   = default;
        CommandPool(const CommandPool&) = delete;
        CommandPool(CommandPool&&)      = delete;
        virtual ~CommandPool()          = default;

        /// @brief Resets the command pool, clearing any allocated command lists.
        virtual void                     Reset() = 0;

        /// @brief Allocates a command list from the pool.
        /// @param queueType Type of queue to allocate for.
        /// @param level Level of the command list to allocate.
        /// @return A pointer to the allocated command list.
        TL_NODISCARD inline CommandList* Allocate(QueueType queueType, CommandListLevel level)
        {
            return Allocate(queueType, level, 1).front();
        }

        /// @brief Allocates multiple command lists from the pool.
        /// @param queueType Type of queue to allocate for.
        /// @param level Level of the command lists to allocate.
        /// @param count Number of command lists to allocate.
        /// @return A vector of pointers to the allocated command lists.
        TL_NODISCARD virtual TL::Vector<CommandList*> Allocate(QueueType queueType, CommandListLevel level, uint32_t count) = 0;
    };

    /// @brief Represents a list of commands to be executed.
    class RHI_EXPORT CommandList
    {
    public:
        CommandList()                         = default;
        CommandList(const CommandList& other) = delete;
        CommandList(CommandList&& other)      = default;
        virtual ~CommandList()                = default;

        /// @brief Begins recording commands into the command list.
        virtual void Begin() = 0;

        /// @brief Begins recording commands into the command list with additional information.
        /// @param beginInfo Information for beginning command recording.
        virtual void Begin(const CommandListBeginInfo& beginInfo) = 0;

        /// @brief Ends recording commands into the command list.
        virtual void End() = 0;

        /// @brief Pushes a debug marker with a name and color onto the command list.
        /// @param name Name of the debug marker.
        /// @param color Color value of the debug marker.
        virtual void DebugMarkerPush(const char* name, ColorValue<float> color) = 0;

        /// @brief Pops the last debug marker off the command list.
        virtual void DebugMarkerPop() = 0;

        /// @brief Begins a conditional command block based on a buffer.
        /// @param buffer Handle to the buffer used for condition.
        /// @param offset Offset in the buffer.
        /// @param inverted If true, the condition is inverted.
        virtual void BeginConditionalCommands(Handle<Buffer> buffer, size_t offset, bool inverted) = 0;

        /// @brief Ends a conditional command block.
        virtual void EndConditionalCommands() = 0;

        /// @brief Executes a set of command lists.
        /// @param commandLists Span of command lists to execute.
        virtual void Execute(TL::Span<const CommandList*> commandLists) = 0;

        /// @brief Binds a graphics pipeline.
        /// @param pipelineState Handle to the graphics pipeline.
        virtual void BindGraphicsPipeline(Handle<GraphicsPipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups) = 0;

        /// @brief Binds a compute pipeline.
        /// @param pipelineState Handle to the compute pipeline.
        virtual void BindComputePipeline(Handle<ComputePipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups) = 0;

        /// @brief Sets the viewport for rendering.
        /// @param viewport The viewport to set.
        virtual void SetViewport(const Viewport& viewport) = 0;

        /// @brief Sets the scissor rectangle for rendering.
        /// @param scissor The scissor rectangle to set.
        virtual void SetSicssor(const Scissor& sicssor) = 0;

        /// @brief Binds the vertex buffers for drawing.
        /// @param vertexBuffers Span of vertex buffer binding information.
        virtual void BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers) = 0;

        /// @brief Binds the index buffer for drawing.
        /// @param indexBuffer Information about the index buffer binding.
        virtual void BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType) = 0;

        /// @brief Issues a draw command.
        /// @param drawInfo Information for the draw command.
        virtual void Draw(const DrawInfo& drawInfo) = 0;

        /// @brief Issues a dispatch command.
        /// @param dispatchInfo Information for the dispatch command.
        virtual void Dispatch(const DispatchInfo& dispatchInfo) = 0;

        /// @brief Issues a buffer-to-buffer copy command.
        /// @param copyInfo Information for the buffer copy command.
        virtual void CopyBuffer(const BufferCopyInfo& copyInfo) = 0;

        /// @brief Issues an image-to-image copy command.
        /// @param copyInfo Information for the image copy command.
        virtual void CopyImage(const ImageCopyInfo& copyInfo) = 0;

        /// @brief Issues a buffer-to-image copy command.
        /// @param copyInfo Information for the buffer-to-image copy command.
        virtual void CopyImageToBuffer(const BufferImageCopyInfo& copyInfo) = 0;

        /// @brief Issues a image-to-buffer copy command.
        /// @param copyInfo Information for the image-to-buffer copy command.
        virtual void CopyBufferToImage(const BufferImageCopyInfo& copyInfo) = 0;

        /// @brief Issues an image blit command.
        /// @param srcImage The source image resource.
        /// @param srcImage The destination resource.
        /// @param regions The blit regions.
        /// @param filter filter mode.
        virtual void BlitImage(Handle<ImageView> srcImage, Handle<ImageView> dstImage, TL::Span<ImageBlitInfo> regions, SamplerFilter filter) = 0;
    };
} // namespace RHI
