#pragma once

#include <RHI/CommandList.hpp>

#include <TL/Containers/Vector.hpp>

#include <d3d12.h>

namespace RHI::D3D12
{
    class IDevice;
    class ICommandPool;

    class ICommandList final : public CommandList
    {
    public:
        ICommandList();
        ~ICommandList();

        ID3D12GraphicsCommandList7* GetHandle() const { return m_cmdLst; }

        ResultCode Init(IDevice* device, ICommandPool* pool, const CommandListCreateInfo& createInfo);
        void       Shutdown();

        // Interface implementation
        void Begin() override;
        void End() override;
        void AddPipelineBarrier(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers) override;
        void BeginRenderPass(const RenderPassBeginInfo& beginInfo) override;
        void EndRenderPass() override;
        void BeginComputePass(const ComputePassBeginInfo& beginInfo) override;
        void EndComputePass() override;
        void PushDebugMarker(const char* name, uint32_t bgra) override;
        void PopDebugMarker() override;
        void InsertDebugMarker(const char* name, uint32_t bgra) override;
        void BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted) override;
        void EndConditionalCommands() override;
        void Execute(TL::Span<const CommandList*> commandLists) override;
        void BindPipelineLayout(BindPoint bindPoint, const PipelineLayout* pipelineLayout) override;
        void SetPushConstants(BindPoint bindPoint, uint32_t offset, TL::Block content) override;
        void PushBindGroup(BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos) override;
        void SetBindGroups(BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups) override;
        void BindGraphicsPipeline(const GraphicsPipeline* pipelineState) override;
        void BindComputePipeline(const ComputePipeline* pipelineState) override;
        void SetViewport(const Viewport& viewport) override;
        void SetScissor(const Scissor& scissor) override;
        void BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers) override;
        void BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType) override;
        void Draw(const DrawParameters& parameters) override;
        void DrawIndexed(const DrawIndexedParameters& parameters) override;
        void DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) override;
        void DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) override;
        void DrawMeshTasks(const DispatchParameters drawMeshTasksDesc) override;
        void DrawMeshTasksIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride) override;
        void DispatchRays(const DispatchRaysInfo& dispatchRaysDesc) override;
        void DispatchRaysIndirect(const BufferBindingInfo& argumentBuffer) override;
        void Dispatch(const DispatchParameters& parameters) override;
        void DispatchIndirect(const BufferBindingInfo& argumentBuffer) override;
        void CopyBuffer(const BufferCopyInfo& copyInfo) override;
        void CopyImage(const ImageCopyInfo& copyInfo) override;
        void CopyImageToBuffer(const BufferImageCopyInfo& copyInfo) override;
        void CopyBufferToImage(const BufferImageCopyInfo& copyInfo) override;

        void BuildMicromaps(TL::Span<const BuildMicromapDesc> buildMicromapDescs) override;
        void WriteMicromapsSizes(TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset) override;
        void CopyMicromap(Micromap* dst, const Micromap* src, CopyMode copyMode) override;
        void BuildTopLevelAccelerationStructures(TL::Span<const BuildTopLevelAccelerationStructureDesc> buildTopLevelAccelerationStructureDescs) override;
        void BuildBottomLevelAccelerationStructures(TL::Span<const BuildBottomLevelAccelerationStructureDesc> buildBottomLevelAccelerationStructureDescs) override;
        void WriteAccelerationStructuresSizes(TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset) override;
        void CopyAccelerationStructure(AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode) override;

        UINT                    GetVertexBufferStrideFromBoundPipeline(uint32_t binding);
        ID3D12CommandSignature* GetCommandSignatureFromBoundPipeline(D3D12_INDIRECT_ARGUMENT_TYPE type, UINT stride = 0);

        IDevice*                    m_device  = nullptr;
        ICommandPool*               m_pool    = nullptr;
        ID3D12GraphicsCommandList7* m_cmdLst  = nullptr;
    };
} // namespace RHI::D3D12
