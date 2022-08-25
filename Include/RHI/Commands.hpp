#pragma once
#include "RHI/Resource.hpp"

namespace RHI
{

class IDescriptorSet;
class IPipelineLayout;
class IPipelineState;

struct DispatchCommand
{
    DispatchCommand() = default;

    inline void SetPipelineState(IPipelineState& pipelineState)
    {
        pPipelineState = &pipelineState;
    }

    inline void SetDescriptorSets(IPipelineLayout& layout, const std::vector<IDescriptorSet*>& descriptorSets)
    {
        pPipelineLayout   = &layout;
        descriptorSetPtrs = descriptorSets;
    }

    IPipelineState*              pPipelineState;
    IPipelineLayout*             pPipelineLayout;
    std::vector<IDescriptorSet*> descriptorSetPtrs;

    uint32_t countX  = 0;
    uint32_t countY  = 0;
    uint32_t countZ  = 0;
    uint32_t offsetX = 0;
    uint32_t offsetY = 0;
    uint32_t offsetZ = 0;
};

struct DrawCommand
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

    DrawCommand(const LinearData& desc)
        : type(EType::Linear)
        , linearDrawDesc(desc)
    {
    }

    DrawCommand(const IndexedData& desc)
        : type(EType::Indexed)
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

    inline void SetDescriptorSets(IPipelineLayout& layout, const std::vector<IDescriptorSet*>& descriptorSets)
    {
        pPipelineLayout   = &layout;
        descriptorSetPtrs = descriptorSets;
    }

    const EType type;
    union
    {
        const LinearData  linearDrawDesc;
        const IndexedData indexedDrawDesc;
    };
    std::vector<IDescriptorSet*> descriptorSetPtrs = {};
    IPipelineLayout*             pPipelineLayout   = nullptr;
    IPipelineState*              pPipelineState    = nullptr;
    IBuffer*                     pVertexBuffer     = nullptr;
    IBuffer*                     pIndexBuffer      = nullptr;
    IBuffer*                     pInstanceBuffer   = nullptr;
};

struct CopyCommand
{
    CopyCommand();

    inline void SetSource(const IImage& image, const ImageViewRange& range);
    inline void SetSource(const IBuffer& buffer, const BufferViewRange& range);
    inline void SetDestination(const IImage& image, const ImageViewRange& range);
    inline void SetDestination(const IBuffer& buffer, const BufferViewRange& range);

    EResourceType sourceResourceType;
    union
    {
        IImage*        pImage;
        ImageViewRange imageRange;
    };

    EResourceType destinationResourceType;
    union
    {
        IBuffer*        pBuffer;
        BufferViewRange bufferRange;
    };
};

class ICommandBuffer
{
public:
    virtual void Begin() = 0;
    virtual void End()   = 0;

    virtual void SetViewports(const std::vector<Viewport>& viewports) = 0;
    virtual void SetScissors(const std::vector<Rect>& scissors)       = 0;

    virtual void Submit(const DrawCommand& drawCommand)         = 0;
    virtual void Submit(const CopyCommand& copyCommand)         = 0;
    virtual void Submit(const DispatchCommand& dispatchCommand) = 0;
};

} // namespace RHI
