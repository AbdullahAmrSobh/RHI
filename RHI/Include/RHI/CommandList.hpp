#pragma once

#include "RHI/Resources.hpp"
#include "RHI/RenderGraph.hpp"
#include "RHI/QueueType.hpp"

#include "RHI/Common/Span.hpp"
#include "RHI/Common/Containers.h"

namespace RHI
{
    struct GraphicsPipeline;
    struct ComputePipeline;

    class Pass;
    class CommandList;

    enum class CommandPoolFlags
    {
        None      = 0,
        Transient = 0x01,
        Reset     = 0x02,
    };

    enum class CommandListLevel
    {
        Primary,
        Secondary,
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
        RenderGraph*               renderGraph;
        Handle<Pass>               pass;
        TL::Span<const ClearValue> clearValues;
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
        BufferBindingInfo() = default;

        BufferBindingInfo(Handle<Buffer> buffer, size_t offset = 0)
            : buffer(buffer)
            , offset(offset)
        {
        }

        Handle<Buffer> buffer;
        size_t         offset;
    };

    struct BindGroupBindingInfo
    {
        BindGroupBindingInfo() = default;

        BindGroupBindingInfo(Handle<BindGroup> bindGroup, TL::Span<const uint32_t> dynamicOffsets = {})
            : bindGroup(bindGroup)
            , dynamicOffsets(dynamicOffsets)
        {
        }

        Handle<BindGroup>        bindGroup;
        TL::Span<const uint32_t> dynamicOffsets;
    };

    struct DrawParameters
    {
        uint32_t elementsCount;
        uint32_t instanceCount = 1;
        uint32_t firstElement  = 0;
        int32_t  vertexOffset  = 0;
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

    struct DrawInfo
    {
        DrawInfo() = default;

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

    class RHI_EXPORT CommandPool
    {
    public:
        CommandPool()                   = default;
        CommandPool(const CommandPool&) = delete;
        CommandPool(CommandPool&&)      = delete;
        virtual ~CommandPool()          = default;

        /// @brief Resets all command lists allocated from this allocator
        virtual void Reset() = 0;

        /// @brief Allocates a new command list object
        RHI_NODISCARD inline CommandList* Allocate(QueueType queueType, CommandListLevel level)
        {
            return Allocate(queueType, level, 1).front();
        }

        /// @brief Allocates a new command list object
        RHI_NODISCARD virtual TL::Vector<CommandList*> Allocate(QueueType queueType, CommandListLevel level, uint32_t count) = 0;

        /// @brief Releases a command list object (must be allocated through here)
        virtual void Release(TL::Span<const CommandList* const> commandLists) = 0;
    };

    /// @brief Command list record a list of GPU commands that are exectued in the same pass.
    class RHI_EXPORT CommandList
    {
    public:
        CommandList()                         = default;
        CommandList(const CommandList& other) = delete;
        CommandList(CommandList&& other)      = default;
        virtual ~CommandList()                = default;

        /// @brief Marks the begining of this command list recording
        virtual void Begin() = 0;

        /// @brief Marks the begining of this command list recording inside a pass
        virtual void Begin(const CommandListBeginInfo& beginInfo) = 0;

        /// @brief Marks the ending of this command list recording
        virtual void End() = 0;

        /// @brief Begins a new debug marker region
        virtual void DebugMarkerPush(const char* name, ColorValue<float> color) = 0;

        /// @brief Ends the last debug marker region
        virtual void DebugMarkerPop() = 0;

        /// @brief Define the beginning of a conditional command list block
        virtual void BeginConditionalCommands(Handle<Buffer> buffer, size_t offset, bool inverted) = 0;

        /// @brief Define the ending of a conditional command list block
        virtual void EndConditionalCommands() = 0;

        /// @brief Execute a secondary command list from a primary command list
        virtual void Execute(TL::Span<const CommandList*> commandLists) = 0;

        /// @brief Sets the rendering viewport
        virtual void SetViewport(const Viewport& viewport) = 0;

        /// @brief Sets the rendering scissor
        virtual void SetSicssor(const Scissor& sicssor) = 0;

        /// @brief Submit a draw command
        virtual void Draw(const DrawInfo& drawInfo) = 0;

        /// @brief Submit a compute command
        virtual void Dispatch(const DispatchInfo& dispatchInfo) = 0;

        /// @brief Submit a buffer copy command
        virtual void CopyBuffer(const BufferCopyInfo& copyInfo) = 0;

        /// @brief Submit a image copy command
        virtual void CopyImage(const ImageCopyInfo& copyInfo) = 0;

        /// @brief Submit a buffer to image copy command
        virtual void CopyImageToBuffer(const BufferImageCopyInfo& copyInfo) = 0;

        /// @brief Submit a image to buffer copy command
        virtual void CopyBufferToImage(const BufferImageCopyInfo& copyInfo) = 0;
    };
} // namespace RHI