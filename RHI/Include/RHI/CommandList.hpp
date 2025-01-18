#pragma once

#include "RHI/Common.hpp"
#include "RHI/RenderTarget.hpp"
#include "RHI/Queue.hpp"
#include "RHI/Buffer.hpp"
#include "RHI/Image.hpp"
#include "RHI/BindGroup.hpp"

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

namespace RHI
{
    template<typename T>
    struct ColorValue;

    struct GraphicsPipeline;
    struct ComputePipeline;

    class RenderGraph;
    class Pass;

    enum class IndexType
    {
        uint16,
        uint32,
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

    /// @brief Contains information about binding a buffer.
    struct BufferBindingInfo
    {
        Handle<Buffer> buffer = NullHandle; ///< Handle to the buffer.
        size_t         offset = 0;          ///< Offset into the buffer.
    };

    /// @brief Contains information about binding a bind group.
    struct BindGroupBindingInfo
    {
        Handle<BindGroup>        bindGroup      = NullHandle; ///< Handle to the bind group.
        TL::Span<const uint32_t> dynamicOffsets = {};         ///< Span of dynamic offsets for the bind group.
    };

    /// @brief Parameters for drawing primitives.
    struct DrawParameters // TODO: remove this
    {
        uint32_t elementsCount = 0; ///< Number of elements to draw.
        uint32_t instanceCount = 1; ///< Number of instances to draw.
        uint32_t firstElement  = 0; ///< Index of the first element to draw.
        int32_t  vertexOffset  = 0; ///< Offset in the vertex buffer.
        uint32_t firstInstance = 0; ///< Index of the first instance to draw.
    };

    /// @brief Parameters for dispatching compute work.
    struct DispatchParameters
    {
        uint32_t offsetX = 0;  ///< X offset for the dispatch.
        uint32_t offsetY = 0;  ///< Y offset for the dispatch.
        uint32_t offsetZ = 0;  ///< Z offset for the dispatch.
        uint32_t countX  = 16; ///< Number of work groups in X dimension.
        uint32_t countY  = 16; ///< Number of work groups in Y dimension.
        uint32_t countZ  = 16; ///< Number of work groups in Z dimension.
    };

    /// @brief Contains information needed to blit (copy and scale) an image.
    struct ImageBlitRegion
    {
        ImageSubresourceLayers srcSubresource = {}; ///< Source subresource layers.
        ImageOffset3D          srcOffsets[2]  = {}; ///< Source offsets (top-left and bottom-right).
        ImageSubresourceLayers dstSubresource = {}; ///< Destination subresource layers.
        ImageOffset3D          dstOffsets[2]  = {}; ///< Destination offsets (top-left and bottom-right).
    };

    struct ImageBlitInfo
    {
        Handle<Image>             srcImage = NullHandle;
        Handle<Image>             dstImage = NullHandle;
        SamplerFilter             filter   = SamplerFilter::Linear;
        TL::Span<ImageBlitRegion> regions  = {};
    };

    struct CommandListCreateInfo
    {
        const char* name      = nullptr;
        QueueType   queueType = QueueType::Graphics;
    };

    struct DrawIndirectCommandArgs
    {
        uint32_t vertexCount   = 0;
        uint32_t instanceCount = 1;
        uint32_t firstVertex   = 0;
        uint32_t firstInstance = 0;
    };

    struct DrawIndexedIndirectCommandArgs
    {
        uint32_t indexCount    = 0;
        uint32_t instanceCount = 1;
        uint32_t firstIndex    = 0;
        int32_t  vertexOffset  = 0;
        uint32_t firstInstance = 0;
    };

    /// @brief Represents a list of commands to be executed.
    class RHI_EXPORT CommandList
    {
    public:
        RHI_INTERFACE_BOILERPLATE(CommandList);

        /// @brief Pushes a debug marker with a name and color onto the command list.
        /// @param name Name of the debug marker.
        /// @param color Color value of the debug marker.
        virtual void DebugMarkerPush(const char* name, ColorValue<float> color)                                                    = 0;

        /// @brief Pops the last debug marker off the command list.
        virtual void DebugMarkerPop()                                                                                              = 0;

        /// @brief Begins a conditional command block based on a buffer.
        /// @param buffer Handle to the buffer used for condition.
        /// @param offset Offset in the buffer.
        /// @param inverted If true, the condition is inverted.
        virtual void BeginConditionalCommands(Handle<Buffer> buffer, size_t offset, bool inverted)                                 = 0;

        /// @brief Ends a conditional command block.
        virtual void EndConditionalCommands()                                                                                      = 0;

        /// @brief Executes a set of command lists.
        /// @param commandLists Span of command lists to execute.
        virtual void Execute(TL::Span<const CommandList*> commandLists)                                                            = 0;

        /// @brief Binds a graphics pipeline.
        /// @param pipelineState Handle to the graphics pipeline.
        virtual void BindGraphicsPipeline(Handle<GraphicsPipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups) = 0;

        /// @brief Binds a compute pipeline.
        /// @param pipelineState Handle to the compute pipeline.
        virtual void BindComputePipeline(Handle<ComputePipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups)   = 0;

        /// @brief Sets the viewport for rendering.
        /// @param viewport The viewport to set.
        virtual void SetViewport(const Viewport& viewport)                                                                         = 0;

        /// @brief Sets the scissor rectangle for rendering.
        /// @param scissor The scissor rectangle to set.
        virtual void SetSicssor(const Scissor& sicssor)                                                                            = 0;

        /// @brief Binds the vertex buffers for drawing.
        /// @param vertexBuffers Span of vertex buffer binding information.
        virtual void BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers)                     = 0;

        /// @brief Binds the index buffer for drawing.
        /// @param indexBuffer Information about the index buffer binding.
        virtual void BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType)                                    = 0;

        /// @brief Issues a draw command.
        /// @param drawInfo Information for the draw command.
        virtual void Draw(const DrawParameters& parameters)                                                                        = 0;

        /// @brief Issues a dispatch command.
        virtual void Dispatch(const DispatchParameters& parameters)                                                                = 0;

        /// @brief Issues a buffer-to-buffer copy command.
        virtual void CopyBuffer(const BufferCopyInfo& copyInfo)                                                                    = 0;

        /// @brief Issues an image-to-image copy command.
        /// @param copyInfo Information for the image copy command.
        virtual void CopyImage(const ImageCopyInfo& copyInfo)                                                                      = 0;

        /// @brief Issues a buffer-to-image copy command.
        /// @param copyInfo Information for the buffer-to-image copy command.
        virtual void CopyImageToBuffer(const BufferImageCopyInfo& copyInfo)                                                        = 0;

        /// @brief Issues a image-to-buffer copy command.
        /// @param copyInfo Information for the image-to-buffer copy command.
        virtual void CopyBufferToImage(const BufferImageCopyInfo& copyInfo)                                                        = 0;

        /// @brief Issues an image blit command.
        /// @param blitInfo Information for the image blit command.
        virtual void BlitImage(const ImageBlitInfo& blitInfo)                                                                      = 0;
    };
} // namespace RHI
