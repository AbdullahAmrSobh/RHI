#pragma once
#include "RHI/Common.hpp"
#include "RHI/Resource.hpp"

namespace RHI
{

enum class EResourceType;

struct ImageViewRange;
struct BufferViewRange;

class IShaderResourceGroup;
class IPipelineState;
class IBuffer;
class IBufferView;
class IImage;
class IImageView;

struct PipelineCommand
{
    PipelineCommand(const IPipelineState& pipelineState);

    void BindShaderResourceGroup(std::string_view groupName, const IShaderResourceGroup& group);

    std::vector<IShaderResourceGroup> resourceGroup;
    IPipelineState*                   pPipelineState;
};

struct DispatchCommand : PipelineCommand
{
    IPipelineState* pPipelineState;
    uint32_t        countX  = 0;
    uint32_t        countY  = 0;
    uint32_t        countZ  = 0;
    uint32_t        offsetX = 0;
    uint32_t        offsetY = 0;
    uint32_t        offsetZ = 0;
};

struct DrawCommand : PipelineCommand
{
    enum class EType
    {
        Linear,
        Indexed
    };

    struct LinearData
    {
        uint32_t instanceCount  = 1;
        uint32_t instanceOffset = 0;
        uint32_t vertexCount    = 0;
        uint32_t vertexOffset   = 0;
    };

    struct IndexedData
    {
        uint32_t instanceCount  = 1;
        uint32_t instanceOffset = 0;
        uint32_t vertexOffset   = 0;
        uint32_t indexCount     = 0;
        uint32_t indexOffset    = 0;
    };

    DrawCommand(const IPipelineState& pso, const LinearData& desc)
        : PipelineCommand(pso)
        , type(EType::Linear)
        , linearDrawDesc(desc)
    {
    }

    DrawCommand(const IPipelineState& pso, const IndexedData& desc)
        : PipelineCommand(pso)
        , type(EType::Indexed)
        , indexedDrawDesc(desc)
    {
    }

    inline void SetVertexBuffer(IBuffer& vertexBuffer)
    {
        pVertexBuffer = &vertexBuffer;
    }

    inline void SetIndexBuffer(IBuffer& indexBuffer)
    {
        pIndexBuffer = &indexBuffer;
    }

    inline void SetInstanceBuffer(IBuffer& instanceBuffer)
    {
        pInstanceBuffer = &instanceBuffer;
    }

    inline void SetPipelineState(IPipelineState& pipelineState)
    {
        pPipelineState = &pipelineState;
    }

    const EType type;
    union
    {
        const LinearData  linearDrawDesc;
        const IndexedData indexedDrawDesc;
    };

    IPipelineState* pPipelineState  = nullptr;
    IBuffer*        pVertexBuffer   = nullptr;
    IBuffer*        pIndexBuffer    = nullptr;
    IBuffer*        pInstanceBuffer = nullptr;
};

struct CopyCommand
{
    CopyCommand();

    inline void SetSource(const IImage& image, const ImageViewRange& range);
    inline void SetSource(const IBuffer& buffer, const BufferViewRange& range);
    inline void SetDestination(const IImage& image, const ImageViewRange& range);
    inline void SetDestination(const IBuffer& buffer, const BufferViewRange& range);

    EResourceType sourceResourceType;
    EResourceType destinationResourceType;

    struct CopyImage
    {
        IImage*        pImage;
        ImageViewRange imageRange;
    };

    struct CopyBuffer
    {
        IBuffer*        pBuffer;
        BufferViewRange bufferRange;
    };

    union
    {
        CopyImage  srcImage;
        CopyBuffer srcBuffer;
    };

    union
    {
        CopyImage  dstImage;
        CopyBuffer dstBuffer;
    };
};

class ICommandBuffer
{
public:
    virtual ~ICommandBuffer();

    virtual void Begin() = 0;
    virtual void End()   = 0;

    virtual void SetViewports(const std::vector<Viewport>& viewports) = 0;
    virtual void SetScissors(const std::vector<Rect>& scissors)       = 0;

    virtual void Submit(const DrawCommand& drawCommand)         = 0;
    virtual void Submit(const CopyCommand& copyCommand)         = 0;
    virtual void Submit(const DispatchCommand& dispatchCommand) = 0;
};

} // namespace RHI
