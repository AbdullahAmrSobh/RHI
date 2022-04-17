#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

class IDescriptorSet;
class IPipelineLayout;
class IPipelineState;

struct DrawCommand
{
    enum class EDrawType
    {
        Linear,
        Indexed
    };

    struct DrawLinear
    {
        DrawLinear() = default;

        DrawLinear(uint32_t instanceCount, uint32_t instanceOffset, uint32_t vertexCount, uint32_t vertexOffset)
            : instanceCount(instanceCount)
            , instanceOffset(instanceOffset)
            , vertexCount(vertexCount)
            , vertexOffset(vertexOffset)
        {
        }

        uint32_t instanceCount  = 1;
        uint32_t instanceOffset = 0;
        uint32_t vertexCount    = 0;
        uint32_t vertexOffset   = 0;
    };

    struct DrawIndexed
    {
        DrawIndexed() = default;

        DrawIndexed(uint32_t instanceCount, uint32_t instanceOffset, uint32_t vertexOffset, uint32_t indexCount, uint32_t indexOffset)
            : instanceCount(instanceCount)
            , instanceOffset(instanceOffset)
            , vertexOffset(vertexOffset)
            , indexCount(indexCount)
            , indexOffset(indexOffset)
        {
        }

        uint32_t instanceCount  = 1;
        uint32_t instanceOffset = 0;
        uint32_t vertexOffset   = 0;
        uint32_t indexCount     = 0;
        uint32_t indexOffset    = 0;
    };

    DrawCommand(const DrawLinear& desc)
        : type(EDrawType::Linear)
        , linearDrawDesc(desc)
    {
    }

    DrawCommand(const DrawIndexed& desc)
        : type(EDrawType::Indexed)
        , indexedDrawDesc(desc)
    {
    }

    const EDrawType type;
    union
    {
        const DrawLinear  linearDrawDesc;
        const DrawIndexed indexedDrawDesc;
    };

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

    std::vector<IDescriptorSet*> descriptorSetPtrs = {};
    IPipelineLayout*             pPipelineLayout   = nullptr;
    IPipelineState*              pPipelineState    = nullptr;
    IBuffer*                     pVertexBuffer     = nullptr;
    IBuffer*                     pIndexBuffer      = nullptr;
    IBuffer*                     pInstanceBuffer   = nullptr;
};

} // namespace RHI
