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

    struct LinearDrawDesc
    {
        LinearDrawDesc() = default;

        LinearDrawDesc(uint32_t instanceCount, uint32_t instanceOffset, uint32_t vertexCount, uint32_t vertexOffset)
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

    struct IndexedDrawDesc
    {
        IndexedDrawDesc() = default;

        IndexedDrawDesc(uint32_t instanceCount, uint32_t instanceOffset, uint32_t vertexOffset, uint32_t indexCount, uint32_t indexOffset)
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
    
    DrawCommand(const LinearDrawDesc& desc, IBuffer& vertexBuffer)
		: type(EDrawType::Linear)
		, linearDrawDesc(desc)
		, pVertexBuffer(&vertexBuffer)
	{
	}
    
    DrawCommand(const IndexedDrawDesc& desc, IBuffer& vertexBuffer, IBuffer& indexBuffer)
		: type(EDrawType::Indexed)
		, indexedDrawDesc(desc)
		, pVertexBuffer(&vertexBuffer)
		, pIndexBuffer(&indexBuffer)
	{
	}
    
    IPipelineLayout*             pPipelineLayout   = nullptr;
    IPipelineState*              pPipelineState    = nullptr;
    std::vector<IDescriptorSet*> descriptorSetPtrs = {};
    IBuffer*                     pVertexBuffer     = nullptr;
    IBuffer*                     pIndexBuffer      = nullptr;
    IBuffer*                     pInstanceBuffer   = nullptr;
    
    const EDrawType type;
    union
    {
        const LinearDrawDesc  linearDrawDesc;
        const IndexedDrawDesc indexedDrawDesc;
    };
};

} // namespace RHI
