#pragma once

#include <span>

#include "RHI/Export.hpp"
#include "RHI/ResourcePool.hpp"

namespace RHI
{

class PipelineState;
class ShaderResourceGroup;

enum class TransferCommandType
{
    Buffer,
    Image,
    BufferToImage,
    ImageToBuffer,
    Invalid
};

struct DrawArea
{
    int32_t  offsetX;
    int32_t  offsetY;
    uint32_t width;
    uint32_t height;
    float    minDepth = 0.0f;
    float    maxDepth = 1.0f;
};

struct DrawIndexedData
{
    uint32_t indexCount;
    uint32_t vertexOffset;
    uint32_t indexOffset;
};

struct DrawUnindexedData
{
    uint32_t vertexCount;
    uint32_t vertexOffset;
};

// Describe a draw command
struct Draw
{
    PipelineState* pipelineState;
    Buffer*        indexBuffer;
    uint32_t       vertexBuffersCount;
    Buffer**       vertexBuffers;
    uint32_t       instanceCount  = 1;
    uint32_t       instanceOffset = 0;

    union
    {
        DrawUnindexedData linearData;
        DrawIndexedData   indexedData;
    };
};

// Describe a compute command
struct Compute
{
    PipelineState* pipelineState;

    uint32_t offsetX = 0;
    uint32_t offsetY = 0;
    uint32_t offsetZ = 0;
    uint32_t countX  = 32;
    uint32_t countY  = 32;
    uint32_t countZ  = 32;
};

struct ImageSubresourceView
{
    Flags<ImageAspect> aspects;
    uint32_t           mipLevel    = 1;
    uint32_t           arrayCount  = 1;
    uint32_t           arrayOffset = 0;
};

struct CopyBufferDescriptor
{
    CopyBufferDescriptor() = default;

    const Buffer* srcBuffer = nullptr;
    uint32_t      srcOffset = 0;
    const Buffer* dstBuffer = nullptr;
    uint32_t      dstOffset = 0;
    uint32_t      size      = 0;
};

struct CopyImageDescriptor
{
    CopyImageDescriptor() = default;

    const Image*         srcImage = nullptr;
    ImageSubresourceView srcSubresource;
    ImageOffset          srcOrigin;
    ImageSize            srcSize;
    const Image*         dstImage = nullptr;
    ImageSubresourceView dstSubresource;
    ImageOffset          dstOrigin;
};

struct CopyBufferToImageDescriptor
{
    CopyBufferToImageDescriptor() = default;

    const Buffer*        srcBuffer        = nullptr;
    uint32_t             srcOffset        = 0;
    uint32_t             srcBytesPerRow   = 0;
    uint32_t             srcBytesPerImage = 0;
    ImageSize            srcSize;
    const Image*         dstImage = nullptr;
    ImageSubresourceView dstSubresource;
    ImageOffset          dstOrigin;
};

struct CopyImageToBufferDescriptor
{
    CopyImageToBufferDescriptor() = default;

    const Image*         srcImage = nullptr;
    ImageSubresourceView srcSubresource;
    ImageOffset          srcOrigin;
    ImageSize            srcSize;
    const Buffer*        dstBuffer        = nullptr;
    uint32_t             dstOffset        = 0;
    uint32_t             dstBytesPerRow   = 0;
    uint32_t             dstBytesPerImage = 0;
    // The destination format is usually same as sourceImage's format. When source image contains more than one aspect,
    // the format should be compatiable with the aspect of the source image's subresource
    Format destinationFormat;
};

// Describe a transfer command
struct Copy
{
    Copy()
        : type {TransferCommandType::Buffer}
        , buffer {}
    {
    }

    Copy(const CopyBufferDescriptor& descriptor)
        : type {TransferCommandType::Buffer}
        , buffer {descriptor}
    {
    }

    Copy(const CopyImageDescriptor& descriptor)
        : type {TransferCommandType::Image}
        , image {descriptor}
    {
    }

    Copy(const CopyBufferToImageDescriptor& descriptor)
        : type {TransferCommandType::BufferToImage}
        , bufferToImage {descriptor}
    {
    }

    Copy(const CopyImageToBufferDescriptor& descriptor)
        : type {TransferCommandType::ImageToBuffer}
        , imageToBuffer {descriptor}
    {
    }

    TransferCommandType type;

    union
    {
        CopyBufferDescriptor        buffer;
        CopyImageDescriptor         image;
        CopyBufferToImageDescriptor bufferToImage;
        CopyImageToBufferDescriptor imageToBuffer;
    };
};

/// @brief Encapsulates a list of commands for rendering, compute, or transfer.
class RHI_EXPORT CommandList
{
public:
    virtual ~CommandList() = default;

    // Set the sub-region of the frame buffer which will be rendered to.
    // Can only be used with graphics commands.
    virtual void SetRenderArea(const DrawArea& region) = 0;

    // Submits a command for rendering
    virtual void Submit(const Draw& command) = 0;

    // Submits a command for dispatching
    virtual void Submit(const Compute& command) = 0;

    // Submits a command for transfer
    virtual void Submit(const Copy& command) = 0;
};

}  // namespace RHI