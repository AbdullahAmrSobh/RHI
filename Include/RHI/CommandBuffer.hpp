#pragma once
#include "RHI/Common.hpp"

namespace RHI
{

class IPipelineState;
class IBuffer;

struct DrawCommand
{
    DrawCommand() = default;

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

    using DrawData = std::variant<LinearDrawData, IndexedDrawData>;

    IPipelineState* pPipelineState = nullptr;
    IBuffer*        pVertexBuffer  = nullptr;
    IBuffer*        pIndexBuffer   = nullptr;
    DrawData        drawData;
};

class ICommandBuffer
{
public:
    virtual ~ICommandBuffer() = default;

    virtual void SetViewports(std::span<const Viewport> viewports) = 0;
    virtual void SetScissors(std::span<const Rect> scissors)       = 0;

    virtual void Submit(const DrawCommand& drawCommand) = 0;
};

}  // namespace RHI
