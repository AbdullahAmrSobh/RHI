#pragma once

#include "RHI/Resources.hpp"

namespace RHI
{

    struct GraphicsPipeline;
    struct ComputePipeline;

    enum class CopyCommandType
    {
        Buffer = 0,
        Image,
        BufferToImage,
        ImageToBuffer,
        Invalid
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
        Handle<Image>    sourceImage;
        ImageSubresource sourceSubresource;
        ImageOffset      sourceOffset;
        ImageSize        sourceSize;
        Handle<Image>    destinationImage;
        ImageSubresource destinationSubresource;
        ImageOffset      destinationOffset;
    };

    struct CopyBufferToImageDescriptor
    {
        Handle<Buffer>   srcBuffer;
        uint32_t         srcOffset        = 0;
        uint32_t         srcBytesPerRow   = 0;
        uint32_t         srcBytesPerImage = 0;
        ImageSize        srcSize;
        Handle<Image>    dstImage;
        ImageSubresource dstSubresource;
        ImageOffset      dstOffset;
    };

    struct CopyImageToBufferDescriptor
    {
        Handle<Image>    sourceImage;
        ImageSubresource sourceSubresource;
        ImageOffset      sourceOffset;
        ImageSize        sourceSize;
        Handle<Buffer>   destinationBuffer;
        uint32_t         destinationOffset        = 0;
        uint32_t         destinationBytesPerRow   = 0;
        uint32_t         destinationBytesPerImage = 0;
        // The destinationBuffer format is usually same as sourceImage's format. When source image contains more than one aspect,
        // the format should be compatiable with the aspect of the source image's subresource
        Format           destinationFormat;
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
        Handle<GraphicsPipeline>          pipelineState;

        TL::Span<Handle<ShaderBindGroup>> shaderBindGroups;

        TL::Span<const Handle<Buffer>>    vertexBuffers;

        Handle<Buffer>                    indexBuffers;

        DrawParameters                    parameters;
    };

    /// @brief Structure describing a copy command.
    struct CommandCopy
    {
        CommandCopy()
            : type{ CopyCommandType::Buffer }
            , buffer{}
        {
        }

        CommandCopy(const CopyBufferDescriptor& descriptor)
            : type{ CopyCommandType::Buffer }
            , buffer{ descriptor }
        {
        }

        CommandCopy(const CopyImageDescriptor& descriptor)
            : type{ CopyCommandType::Image }
            , image{ descriptor }
        {
        }

        CommandCopy(const CopyBufferToImageDescriptor& descriptor)
            : type{ CopyCommandType::BufferToImage }
            , bufferToImage{ descriptor }
        {
        }

        CommandCopy(const CopyImageToBufferDescriptor& descriptor)
            : type{ CopyCommandType::ImageToBuffer }
            , imageToBuffer{ descriptor }
        {
        }

        CopyCommandType type;

        union
        {
            CopyBufferDescriptor        buffer;
            CopyImageDescriptor         image;
            CopyBufferToImageDescriptor bufferToImage;
            CopyImageToBufferDescriptor imageToBuffer;
        };
    };

    /// @brief Structure describing a compute command.
    struct CommandCompute
    {
        Handle<ComputePipeline>           pipelineState;

        TL::Span<Handle<ShaderBindGroup>> shaderBindGroups;

        DispatchParameters                parameters;
    };

    /// @brief Command list record a list of GPU commands that are exectued in the same pass.
    class CommandList
    {
    public:
        CommandList()                                      = default;
        virtual ~CommandList()                             = default;

        /// @brief Sets the rendering viewport
        virtual void SetViewport(const Viewport& viewport) = 0;

        /// @brief Sets the rendering scissor
        virtual void SetSicssor(const Scissor& sicssor)    = 0;

        /// @brief Submit a draw command.
        virtual void Submit(const CommandDraw& command)    = 0;

        /// @brief Submit a copy command.
        virtual void Submit(const CommandCopy& command)    = 0;

        /// @brief Submit a compute command.
        virtual void Submit(const CommandCompute& command) = 0;
    };

} // namespace RHI