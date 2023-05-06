#pragma once

#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

class PipelineState;
class ShaderResourceGroup;

enum class PredicationOp
{
    None,
    EqualZero,
    NotEqualZero,
};

struct ImageCopyInfo
{
    Image*           image;
    ImageSize        offset;
    ImageSubresource subresource;
};

struct BufferCopyInfo
{
    size_t byteSize;
    size_t byteOffset;
};

struct DrawArea
{
    int32_t  offsetX;
    int32_t  offsetY;
    int32_t  offsetZ;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
};

struct DrawIndexedData
{
    Buffer*  instanceBuffer;
    Buffer*  vertexBuffer;
    Buffer*  indexBuffer;
    uint32_t instanceCount;
    uint32_t indexCount;
    uint32_t firstIndexOffset;
};

struct DrawUnindexedData
{
    Buffer*  instanceBuffer;
    Buffer*  vertexBuffer;
    uint32_t instanceCount;
    uint32_t verticesCount;
    uint32_t firstVertexOffset;
};

struct DispatchRaysData
{
    /** TODO **/
};

class RHI_EXPORT CommandList
{
public:
    virtual ~CommandList() = default;

    virtual void CopyImage(const ImageSize& imageSize, const ImageCopyInfo& srcImage, ImageCopyInfo& dstImage) = 0;

    virtual void CopyBuffer(const BufferCopyInfo& srcBuffer, ImageCopyInfo& dstBuffer) = 0;

    virtual void CopyBufferToImage(const BufferCopyInfo& srcBuffer, ImageCopyInfo& dstImage) = 0;

    virtual void CopyImageToBuffer(const ImageCopyInfo& srcImage, BufferCopyInfo& dstBuffer) = 0;

    virtual void BeginPredication(const Buffer& buffer, uint64_t offset, PredicationOp operation) = 0;

    virtual void EndPredication() = 0;

    virtual void SetPipelineState(const PipelineState& pso) = 0;

    virtual void BindShaderResourceGroup(const ShaderResourceGroup& group) = 0;

    virtual void SetRenderArea(const DrawArea& drawArea) = 0;

    virtual void DrawIndexed(const DrawIndexedData& indexedData) = 0;

    virtual void DrawUnindexed(const DrawUnindexedData& unindexedData) = 0;

    virtual void DispatchRay(const DispatchRaysData& raysData) = 0;

    virtual void Dispatch(uint32_t offset[3], uint32_t groupCount[3]) = 0;

protected:
    PassState* m_passState; // the Pass this command list
};

}  // namespace RHI