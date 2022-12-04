#pragma once
#include "RHI/Common.hpp"
#include "RHI/Resource.hpp"

namespace RHI
{

struct DrawCommand
{
    struct LinearDrawData
    {
        uint32_t instanceCount  = 1;
        uint32_t instanceOffset = 0;
        uint32_t vertexCount    = 0;
        uint32_t vertexOffset   = 0;
    };

    struct IndexedDrawData
    {
        uint32_t instanceCount  = 1;
        uint32_t instanceOffset = 0;
        uint32_t vertexOffset   = 0;
        uint32_t indexCount     = 0;
        uint32_t indexOffset    = 0;
    };

    enum class EType
    {
        Linear,
        Indexed
    };

    IPipelineState* pPipelineState = nullptr;
    IBuffer*        pVertexBuffer  = nullptr;
    IBuffer*        pIndexBuffer   = nullptr;
    EType           type;
    union
    {
        LinearDrawData  linearData;
        IndexedDrawData indexedData;
    };
};

class ICommandBuffer
{
public:
    virtual ~ICommandBuffer() = default;

    virtual void Begin() = 0;
    virtual void End()   = 0;

    virtual void SetViewports(const std::vector<Viewport>& viewports) = 0;
    virtual void SetScissors(const std::vector<Rect>& scissors)       = 0;

    virtual void Submit(const DrawCommand& drawCommand) = 0;
};

}  // namespace RHI
