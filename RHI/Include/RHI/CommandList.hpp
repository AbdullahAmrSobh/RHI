#pragma once

#include "RHI/Resources.hpp"

#include "RHI/Common/Containers.h"

namespace RHI
{
    enum class QueueType;
    struct GraphicsPipeline;
    struct ComputePipeline;
    class Pass;
    class CommandList;

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
        ImageOffset            srcOffset;
        ImageSize3D            srcSize;
        Handle<Image>          dstImage;
        ImageSubresourceLayers dstSubresource;
        ImageOffset            dstOffset;
    };

    struct BufferToImageCopyInfo
    {
        Handle<Buffer>         srcBuffer;
        uint32_t               srcOffset;
        uint32_t               srcBytesPerRow;
        uint32_t               srcBytesPerImage;
        Handle<Image>          dstImage;
        ImageOffset            dstOffset;
        ImageSize3D            dstSize;
        ImageSubresourceLayers dstSubresource;
    };

    struct ImageToBufferCopyInfo
    {
        Handle<Image>          srcImage;
        ImageSubresourceLayers srcSubresource;
        ImageOffset            srcOffset;
        ImageSize3D            srcSize;
        Handle<Buffer>         dstBuffer;
        uint32_t               dstOffset;
        uint32_t               dstBytesPerRow;
        uint32_t               dstBytesPerImage;
        // The destinationBuffer format is usually same as sourceImage's format. When source image contains more than one aspect,
        // the format should be compatiable with the aspect of the source image's subresource
        Format                 dstFormat;
    };

    struct DrawParameters
    {
        uint32_t elementCount;
        uint32_t instanceCount = 1;
        uint32_t firstElement  = 0;
        uint32_t vertexOffset  = 0;
        int32_t  firstInstance = 0;
    };

    struct DispatchParameters
    {
        uint32_t offsetX = 0u;
        uint32_t offsetY = 0u;
        uint32_t offsetZ = 0u;
        uint32_t countX  = 32u;
        uint32_t countY  = 32u;
        uint32_t countZ  = 32u;
    };

    /// @brief Structure describing a draw command.
    struct DrawInfo
    {
        Handle<GraphicsPipeline>       pipelineState;
        TL::Span<Handle<BindGroup>>    bindGroups;
        TL::Span<const Handle<Buffer>> vertexBuffers;
        Handle<Buffer>                 indexBuffers;
        DrawParameters                 parameters;
    };

    /// @brief Structure describing a compute command.
    struct DispatchInfo
    {
        Handle<ComputePipeline>     pipelineState;
        TL::Span<Handle<BindGroup>> bindGroups;
        DispatchParameters          parameters;
    };

    /// @brief Allocates or reuse existing command lists (create on object per thread).
    class RHI_EXPORT CommandListAllocator
    {
    public:
        CommandListAllocator()                                                         = default;
        CommandListAllocator(const CommandListAllocator&)                              = delete;
        CommandListAllocator(CommandListAllocator&&)                                   = delete;
        virtual ~CommandListAllocator()                                                = default;

        /// @brief Resets all command lists allocated from this allocator
        virtual void                     Reset()                                       = 0;

        /// @brief Allocates a new command list object
        virtual CommandList*             Allocate(QueueType queueType)                 = 0;

        /// @brief Allocates a new command list object
        virtual TL::Vector<CommandList*> Allocate(QueueType queueType, uint32_t count) = 0;

        /// @brief Releases a command list object (must be allocated through here)
        virtual void                     Release(TL::Span<CommandList*> commandLists)  = 0;
    };

    /// @brief Command list record a list of GPU commands that are exectued in the same pass.
    class RHI_EXPORT CommandList
    {
    protected:
        CommandList()                         = default;
        CommandList(const CommandList& other) = delete;
        CommandList(CommandList&& other)      = default;

    public:
        virtual ~CommandList()                                                                     = default;

        /// @brief Marks the begining of this command list recording
        virtual void Begin()                                                                       = 0;

        /// @brief Marks the begining of this command list recording inside a pass
        virtual void Begin(Pass& pass)                                                             = 0;

        /// @brief Marks the ending of this command list recording
        virtual void End()                                                                         = 0;

        /// @brief Begins a new debug marker region
        virtual void DebugMarkerPush(const char* name, const struct ColorValue& color)             = 0;

        /// @brief Ends the last debug marker region
        virtual void DebugMarkerPop()                                                              = 0;

        /// @brief Define the beginning of a conditional command list block
        virtual void BeginConditionalCommands(Handle<Buffer> buffer, size_t offset, bool inverted) = 0;

        /// @brief Define the ending of a conditional command list block
        virtual void EndConditionalCommands()                                                      = 0;

        // TODO: add indirect commands here

        /// @brief Execute a secondary command list from a primary command list
        virtual void Execute(TL::Span<const CommandList*> commandLists)                            = 0;

        /// @brief Sets the rendering viewport
        virtual void SetViewport(const Viewport& viewport)                                         = 0;

        /// @brief Sets the rendering scissor
        virtual void SetSicssor(const Scissor& sicssor)                                            = 0;

        /// @brief Submit a draw command
        virtual void Draw(const DrawInfo& drawInfo)                                                = 0;

        /// @brief Submit a compute command
        virtual void Dispatch(const DispatchInfo& dispatchInfo)                                    = 0;

        /// @brief Submit a buffer copy command
        virtual void Copy(const BufferCopyInfo& copyInfo)                                          = 0;

        /// @brief Submit a image copy command
        virtual void Copy(const ImageCopyInfo& copyInfo)                                           = 0;

        /// @brief Submit a buffer to image copy command
        virtual void Copy(const BufferToImageCopyInfo& copyInfo)                                   = 0;

        /// @brief Submit a image to buffer copy command
        virtual void Copy(const ImageToBufferCopyInfo& copyInfo)                                   = 0;
    };
} // namespace RHI