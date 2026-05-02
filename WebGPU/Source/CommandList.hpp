#pragma once

#include <RHI/CommandList.hpp>
#include <RHI/Result.hpp>

#include <dawn/webgpu.h>

namespace RHI::WebGPU
{
    class IDevice;

    class ICommandList final : public CommandList
    {
    public:
        ICommandList();
        ~ICommandList();

        ResultCode Init(IDevice* device, const CommandListCreateInfo& createInfo);
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
        void SetScissor(const Scissor& sicssor) override;
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
        void CopyBuffer(const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size) override;
        void CopyImage(const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size) override;
        void CopyImageToBuffer(const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer) override;
        void CopyBufferToImage(const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout) override;
        void CopyMicromap(Micromap* dst, const Micromap* src, CopyMode copyMode) override;
        void CopyAccelerationStructure(AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode) override;
        void BuildMicromaps(TL::Span<const MicromapBuildInfo> buildInfos) override;
        void WriteMicromapsSizes(TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset) override;
        void BuildTopLevelAccelerationStructures(TL::Span<const TopLevelAccelerationStructureBuildInfo> buildInfos) override;
        void BuildBottomLevelAccelerationStructures(TL::Span<const BottomLevelAccelerationStructureBuildInfo> buildInfos) override;
        void WriteAccelerationStructuresSizes(TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset) override;

    public:
        enum class State
        {
            CommandBuffer,
            CommandEncoder,
            RenderBundle,
            RenderPassEncoder,
            ComputePassEncoder,
        };

        IDevice* m_device = nullptr;

        State m_state = State::CommandEncoder;

        union
        {
            WGPUCommandEncoder      m_cmdEncoder = nullptr;
            WGPURenderBundleEncoder m_bundleEncoder;
        };

        union
        {
            WGPUCommandBuffer m_cmdBuffer = nullptr;
            WGPURenderBundle  m_bundle;
        };

        union
        {
            WGPURenderPassEncoder  m_renderPassEncoder = nullptr;
            WGPUComputePassEncoder m_computePassEncoder;
        };
    };
} // namespace RHI::WebGPU