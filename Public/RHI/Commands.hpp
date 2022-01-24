#pragma once
#include "RHI/Definitions.hpp"

#include "RHI/Buffer.hpp"
#include "RHI/Descriptor.hpp"
#include "RHI/Fence.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/RenderTarget.hpp"
#include "RHI/Texture.hpp"

namespace RHI
{

struct CopyCommand
{
    CopyCommand(size_t size, size_t srcOffset, IBuffer& src, size_t dstOffset, IBuffer& dst);
    CopyCommand(Extent3D size, ITexture& src, ITexture& dst);
};

struct DrawCommand
{
    DrawCommand() = default;
    
    void BindPipelineState(IPipelineState& pipelineState);
    
    void BindDescriptorSet(IDescriptorSet& descriptorSet);
    void BindIndexBuffer(IBuffer& buffer, uint32_t offset);
    void BindRenderTarget(IRenderTarget& renderTarget);
    
    void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void DrawIndirect(IBuffer& buffer, size_t offset, uint32_t drawCount, uint32_t stride);
    void DrawIndexedIndirect(IBuffer& buffer, size_t offset, uint32_t drawCount, uint32_t stride);
};

struct DispatchCommand
{
    void BindPipelineState(const IPipelineState& pipelineState);
    void BindDescriptorSet(const IDescriptorSet& descriptorSet);

    void Dispatch(uint32_t workGroupCountX, uint32_t workGroupCountY, uint32_t workGroupCountZ);
    void DispatchIndirect(IBuffer& buffer, size_t offset);
};

class ICommandList
{
public:
    void SetScissor(const Rect& scissor) { SetScissors(&scissor, 1); }
    void SetViewport(const Viewport& viewport) { SetViewports(&viewport, 1); }

    virtual void Reset() = 0;
    virtual void Begin() = 0;
    virtual void End()   = 0;

    virtual void SetViewports(const Viewport* viewports, uint32_t count) = 0;
    virtual void SetScissors(const Rect* scissors, uint32_t count)       = 0;
    

	// Clear color is a draw command
    // virtual void SetClearColor(const Color& color) = 0;

    // Dispatch commands
    virtual void Submit(const CopyCommand& command)     = 0;
    virtual void Submit(const DrawCommand& command)     = 0;
    virtual void Submit(const DispatchCommand& command) = 0;
};
using CommandListPtr = Unique<ICommandList>;

class ICommandsAllocator
{
public:
    virtual ~ICommandsAllocator() = default;

    virtual Expected<CommandListPtr> AllocateCommandList() = 0;
};
using CommandsAllocatorPtr = Unique<ICommandsAllocator>;

enum class ECommandQueueTypeFlagBits
{
    Graphics = 0x00000001,
    Compute  = 0x00000002,
    Transfer = 0x00000004,
    Present  = 0x00000008,
};
using CommandQueueTypeFlags = Flags<ECommandQueueTypeFlagBits>;

class ICommandQueue
{
public:
    virtual ~ICommandQueue() = default;

    virtual void Submit(ArrayView<ICommandList*> commandLists, IFence& signalFence) = 0;
};
using CommandQueuePtr = Unique<ICommandQueue>;

} // namespace RHI
