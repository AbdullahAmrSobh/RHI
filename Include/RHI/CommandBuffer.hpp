#pragma once
#include "RHI/Common.hpp"

namespace RHI
{

class IShaderResourceGroup;
class IPipelineState;
class IBuffer;

struct Rect
{
    int32_t  x;
    int32_t  y;
    uint32_t sizeX;
    uint32_t sizeY;
};

struct Viewport
{
    Rect  drawingArea;
    float minDepth;
    float maxDepth;
};

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

    void BindShaderResourceGroup(uint32_t index, IShaderResourceGroup& resourceGroup);

    IPipelineState*       pipelineState            = nullptr;
    IBuffer*              vertexBuffer             = nullptr;
    IBuffer*              indexBuffer              = nullptr;
    IShaderResourceGroup* shaderResourceGroup      = nullptr;
    uint32_t              shaderResourceGroupIndex = 0;
    DrawData              drawData;
};

class ICommandBuffer
{
public:
    virtual ~ICommandBuffer() = default;

    void SetViewport(const Viewport& viewports);
    void SetScissor(const Rect& scissors);

    virtual void SetViewports(std::span<const Viewport> viewports) = 0;
    virtual void SetScissors(std::span<const Rect> scissors)       = 0;

    virtual void Submit(const DrawCommand& drawCommand) = 0;
};

}  // namespace RHI
