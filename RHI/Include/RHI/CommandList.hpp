#pragma once

#include "RHI/Resources.hpp"
#include "RHI/Definitions.hpp"

#include "RHI/Common/Span.hpp"

namespace RHI
{
    class RenderGraph;
    class Pass;

    RHI_DECALRE_OPAQUE_RESOURCE(CommandList);

    enum class CommandFlags
    {
        None      = 0,
        Transient = 0x01,
        Reset     = 0x02,
        Secondary = 0x08,
    };

    enum class CommandConditionMode
    {
        None = 0,
        Inverted,
    };

    struct Viewport
    {
        float offsetX;
        float offsetY;
        float width;
        float height;
        float minDepth;
        float maxDepth;
    };

    struct Scissor
    {
        int32_t  offsetX;
        int32_t  offsetY;
        uint32_t width;
        uint32_t height;
    };

    struct CommandListBeginInfo
    {
        RenderGraph*                        renderGraph;
        Handle<Pass>                        pass;
        TL::Span<const LoadStoreOperations> loadStoreOperations;
    };

    struct BufferCopyInfo
    {
        Handle<Buffer> srcBuffer;
        size_t         srcOffset;
        Handle<Buffer> dstBuffer;
        size_t         dstOffset;
        size_t         size;
    };

    struct ImageCopyInfo
    {
        Handle<Image>          srcImage;
        ImageSubresourceLayers srcSubresource;
        ImageOffset3D          srcOffset;
        ImageSize3D            srcSize;
        Handle<Image>          dstImage;
        ImageSubresourceLayers dstSubresource;
        ImageOffset3D          dstOffset;
    };

    struct BufferImageCopyInfo
    {
        Handle<Image>          image;
        ImageSubresourceLayers subresource;
        ImageSize3D            imageSize;
        ImageOffset3D          imageOffset;
        Handle<Buffer>         buffer;
        size_t                 bufferOffset;
        size_t                 bufferSize;
        uint32_t               bytesPerRow;
        uint32_t               bytesPerImage;
    };

    struct BufferBindingInfo
    {
        Handle<Buffer> buffer;
        size_t         offset;
    };

    struct BindGroupBindingInfo
    {
        Handle<BindGroup>        bindGroup;
        TL::Span<const uint32_t> dynamicOffsets;
    };

    struct DrawParameters
    {
        uint32_t elementsCount;
        uint32_t instanceCount;
        uint32_t firstElement;
        int32_t  vertexOffset;
        uint32_t firstInstance;
    };

    struct DispatchParameters
    {
        uint32_t offsetX;
        uint32_t offsetY;
        uint32_t offsetZ;
        uint32_t countX;
        uint32_t countY;
        uint32_t countZ;
    };

    struct DrawInfo
    {
        Handle<GraphicsPipeline>             pipelineState;
        TL::Span<const BindGroupBindingInfo> bindGroups;
        TL::Span<const BufferBindingInfo>    vertexBuffers;
        BufferBindingInfo                    indexBuffer;
        DrawParameters                       parameters;
    };

    /// @brief Structure describing a compute command.
    struct DispatchInfo
    {
        Handle<ComputePipeline>              pipelineState;
        TL::Span<const BindGroupBindingInfo> bindGroups;
        DispatchParameters                   parameters;
    };

    /// @brief Interface for encoding various graphics commands
    ///
    /// This class defines methods for allocating, recording, and managing graphics commands.
    class RHI_EXPORT CommandEncoder
    {
    public:
        /// @brief Virtual destructor to ensure proper cleanup of derived classes
        ///
        virtual ~CommandEncoder() = default;

        /// @brief Allocates command lists with specified flags
        ///
        /// @param flags Flags specifying command list properties
        /// @param commandLists Span of command list handles to allocate
        virtual void Allocate(Flags<CommandFlags> flags, RHI_OUT_PARM TL::Span<Handle<CommandList>> commandLists) = 0;

        /// @brief Releases previously allocated command lists
        ///
        /// @param commandList Span of command list handles to release
        virtual void Release(TL::Span<Handle<CommandList>> commandList) = 0;

        /// @brief Resets list of command lists back to their initial state
        ///
        /// @param commandList Span of command list handles to reset
        virtual void Reset(TL::Span<Handle<CommandList>> commandList) = 0;

        /// @brief Begins recording commands to a command list
        ///
        /// @param commandList Handle to the command list to begin recording
        virtual void Begin(Handle<CommandList> commandList) = 0;

        /// @brief Begins recording commands with additional begin information
        ///
        /// @param commandList Handle to the command list to begin recording
        /// @param beginInfo Additional information for beginning command recording
        virtual void Begin(Handle<CommandList> commandList, const CommandListBeginInfo& beginInfo) = 0;

        /// @brief Ends recording commands to a command list
        ///
        /// @param commandList Handle to the command list to end recording
        virtual void End(Handle<CommandList> commandList) = 0;

        /// @brief Pushes a debug marker with a name and color onto the command list
        ///
        /// @param commandList Handle to the command list
        /// @param name Name of the debug marker
        /// @param color Color of the debug marker
        virtual void DebugMarkerPush(Handle<CommandList> commandList, const char* name, ColorValue<float> color) = 0;

        /// @brief Pops the most recently pushed debug marker from the command list
        ///
        /// @param commandList Handle to the command list
        virtual void DebugMarkerPop(Handle<CommandList> commandList) = 0;

        /// @brief Begins a block of conditional commands based on buffer contents
        ///
        /// @param commandList Handle to the command list
        /// @param buffer Handle to the buffer containing the condition
        /// @param offset Offset into the buffer for the condition
        /// @param inverted Mode for conditional rendering
        virtual void BeginConditionalCommands(Handle<CommandList> commandList, Handle<Buffer> buffer, size_t offset, CommandConditionMode inverted) = 0;

        /// @brief Ends the most recently begun conditional commands block
        ///
        /// @param commandList Handle to the command list
        virtual void EndConditionalCommands(Handle<CommandList> commandList) = 0;

        /// @brief Executes a set of command lists within the current command list
        ///
        /// @param commandList Handle to the current command list
        /// @param commandLists Span of command lists to execute
        virtual void Execute(Handle<CommandList> commandList, TL::Span<const Handle<CommandList>> commandLists) = 0;

        /// @brief Sets the viewport for rendering
        ///
        /// @param commandList Handle to the command list
        /// @param viewport Viewport to set
        virtual void SetViewport(Handle<CommandList> commandList, const Viewport& viewport) = 0;

        /// @brief Sets the scissor rectangle for rendering
        ///
        /// @param commandList Handle to the command list
        /// @param scissor Scissor rectangle to set
        virtual void SetScissor(Handle<CommandList> commandList, const Scissor& scissor) = 0;

        /// @brief Adds a draw command to the command list
        ///
        /// @param commandList Handle to the command list
        /// @param drawInfo Information for the draw command
        virtual void Draw(Handle<CommandList> commandList, const DrawInfo& drawInfo) = 0;

        /// @brief Adds a compute dispatch command to the command list
        ///
        /// @param commandList Handle to the command list
        /// @param dispatchInfo Information for the dispatch command
        virtual void Dispatch(Handle<CommandList> commandList, const DispatchInfo& dispatchInfo) = 0;

        /// @brief Copies data between buffers
        ///
        /// @param commandList Handle to the command list
        /// @param copyInfo Information for the buffer copy operation
        virtual void CopyBuffer(Handle<CommandList> commandList, const BufferCopyInfo& copyInfo) = 0;

        /// @brief Copies data between images
        ///
        /// @param commandList Handle to the command list
        /// @param copyInfo Information for the image copy operation
        virtual void CopyImage(Handle<CommandList> commandList, const ImageCopyInfo& copyInfo) = 0;

        /// @brief Copies data from an image to a buffer
        ///
        /// @param commandList Handle to the command list
        /// @param copyInfo Information for the image-to-buffer copy operation
        virtual void CopyImageToBuffer(Handle<CommandList> commandList, const BufferImageCopyInfo& copyInfo) = 0;

        /// @brief Copies data from a buffer to an image
        ///
        /// @param commandList Handle to the command list
        /// @param copyInfo Information for the buffer-to-image copy operation
        virtual void CopyBufferToImage(Handle<CommandList> commandList, const BufferImageCopyInfo& copyInfo) = 0;
    };
} // namespace RHI