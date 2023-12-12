#pragma once

#include "RHI/Resources.hpp"

namespace RHI
{

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

    struct CopyBufferDescriptor
    {
        Handle<Buffer> sourceBuffer;
        uint32_t       sourceOffset = 0;
        Handle<Buffer> destinationBuffer;
        uint32_t       destinationOffset = 0;
        uint32_t       size              = 0;
    };

    struct CopyImageDescriptor
    {
        Handle<Image>          sourceImage;
        ImageSubresourceLayers sourceSubresource;
        ImageOffset            sourceOffset;
        ImageSize3D            sourceSize;
        Handle<Image>          destinationImage;
        ImageSubresourceLayers destinationSubresource;
        ImageOffset            destinationOffset;
    };

    struct CopyBufferToImageDescriptor
    {
        Handle<Buffer>         srcBuffer;
        uint32_t               srcOffset        = 0;
        uint32_t               srcBytesPerRow   = 0;
        uint32_t               srcBytesPerImage = 0;
        ImageSize3D            srcSize;
        Handle<Image>          dstImage;
        ImageSubresourceLayers dstSubresource;
        ImageOffset            dstOffset;
    };

    struct CopyImageToBufferDescriptor
    {
        Handle<Image>          sourceImage;
        ImageSubresourceLayers sourceSubresource;
        ImageOffset            sourceOffset;
        ImageSize3D            sourceSize;
        Handle<Buffer>         destinationBuffer;
        uint32_t               destinationOffset        = 0;
        uint32_t               destinationBytesPerRow   = 0;
        uint32_t               destinationBytesPerImage = 0;
        // The destinationBuffer format is usually same as sourceImage's format. When source image contains more than one aspect,
        // the format should be compatiable with the aspect of the source image's subresource
        Format                 destinationFormat;
    };

    struct DrawParameters
    {
        uint32_t elementCount;
        uint32_t instanceCount = 1;
        uint32_t firstElement  = 0;
        uint32_t vertexOffset  = 0;
        uint32_t firstInstance = 0;
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
    struct CommandDraw
    {
        Handle<GraphicsPipeline>       pipelineState;
        TL::Span<Handle<BindGroup>>    bindGroups;
        TL::Span<const Handle<Buffer>> vertexBuffers;
        Handle<Buffer>                 indexBuffers;
        DrawParameters                 parameters;
    };

    /// @brief Structure describing a compute command.
    struct CommandCompute
    {
        Handle<ComputePipeline>     pipelineState;
        TL::Span<Handle<BindGroup>> bindGroups;
        DispatchParameters          parameters;
    };

    /// @brief Allocates or reuse existing command lists (create on object per thread).
    class RHI_EXPORT CommandListAllocator
    {
    public:
        virtual ~CommandListAllocator() = default;

        virtual void         Flush()    = 0;
        virtual CommandList* Allocate() = 0;
    };

    /// @brief Command list record a list of GPU commands that are exectued in the same pass.
    class RHI_EXPORT CommandList
    {
    public:
        CommandList()                                                   = default;
        CommandList(const CommandList& other)                           = delete;
        CommandList(CommandList&& other)                                = default;
        virtual ~CommandList()                                          = default;

        /// @brief Resets the command lists.
        virtual void Reset()                                            = 0;

        /// @brief Marks the begining of this command list recording.
        virtual void Begin()                                            = 0;

        /// @brief Marks the begining of this command list recording inside a pass.
        virtual void Begin(Pass& pass)                                  = 0;

        /// @brief Marks the ending of this command list recording.
        virtual void End()                                              = 0;

        /// @brief Sets the rendering viewport
        virtual void SetViewport(const Viewport& viewport)              = 0;

        /// @brief Sets the rendering scissor
        virtual void SetSicssor(const Scissor& sicssor)                 = 0;

        /// @brief Submit a draw command.
        virtual void Submit(const CommandDraw& command)                 = 0;

        /// @brief Submit a compute command.
        virtual void Submit(const CommandCompute& command)              = 0;

        /// @brief Submit a buffer copy command.
        virtual void Submit(const CopyBufferDescriptor& command)        = 0;

        /// @brief Submit a image copy command.
        virtual void Submit(const CopyImageDescriptor& command)         = 0;

        /// @brief Submit a buffer to image copy command.
        virtual void Submit(const CopyBufferToImageDescriptor& command) = 0;

        /// @brief Submit a image to buffer copy command.
        virtual void Submit(const CopyImageToBufferDescriptor& command) = 0;
    };

} // namespace RHI