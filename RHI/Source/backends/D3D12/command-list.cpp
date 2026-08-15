#include <backends/D3D12/command-list.h>

namespace RHI::D3D12
{
    void ICommandPool::Init(IDevice*, const CommandPoolCreateInfo&) {}

    void ICommandPool::Shutdown(IDevice*) {}

    void ICommandList::Init(IDevice*, ICommandPool*) {}

    void ICommandList::Shutdown(IDevice*) {}

    void commandPoolReset(ICommandPool*) {}

    CommandList* commandPoolAllocate(ICommandPool*)
    {
        return nullptr;
    }

    void cmdBegin(ICommandList*) {}

    void cmdEnd(ICommandList*) {}

    void cmdPushDebugMarker(ICommandList*, const char*, uint32_t) {}

    void cmdPopDebugMarker(ICommandList*) {}

    void cmdInsertDebugMarker(ICommandList*, const char*, uint32_t) {}

    void cmdAddPipelineBarrier(ICommandList*, TL::Span<const BarrierInfo>, TL::Span<const ImageBarrierInfo>, TL::Span<const BufferBarrierInfo>) {}

    void cmdBeginRenderPass(ICommandList*, const RenderPassBeginInfo&) {}

    void cmdEndRenderPass(ICommandList*) {}

    void cmdBeginComputePass(ICommandList*, const ComputePassBeginInfo&) {}

    void cmdEndComputePass(ICommandList*) {}

    void cmdBeginConditionalCommands(ICommandList*, const BufferBindingInfo&, bool) {}

    void cmdEndConditionalCommands(ICommandList*) {}

    void cmdExecute(ICommandList*, TL::Span<const CommandList*>) {}

    void cmdBindPipelineLayout(ICommandList*, BindPoint, const PipelineLayout*) {}

    void cmdSetPushConstants(ICommandList*, BindPoint, uint32_t, TL::Block) {}

    void cmdPushBindGroup(ICommandList*, BindPoint, uint32_t, TL::Span<const BindGroupUpdateInfo>) {}

    void cmdSetBindGroups(ICommandList*, BindPoint, TL::Span<const BindGroupBindingInfo>) {}

    void cmdBindGraphicsPipeline(ICommandList*, const GraphicsPipeline*) {}

    void cmdBindComputePipeline(ICommandList*, const ComputePipeline*) {}

    void cmdBindRayTracingPipeline(ICommandList*, const RayTracingPipeline*) {}

    void cmdSetViewport(ICommandList*, float, float, float, float, float, float) {}

    void cmdSetScissor(ICommandList*, int32_t, int32_t, uint32_t, uint32_t) {}

    void cmdBindVertexBuffers(ICommandList*, uint32_t, TL::Span<const BufferBindingInfo>) {}

    void cmdBindIndexBuffer(ICommandList*, const BufferBindingInfo&, IndexType) {}

    void cmdDraw(ICommandList*, uint32_t, uint32_t, uint32_t, uint32_t) {}

    void cmdDrawIndexed(ICommandList*, uint32_t, uint32_t, uint32_t, int32_t, uint32_t) {}

    void cmdDrawMeshTasks(ICommandList*, uint32_t, uint32_t, uint32_t) {}

    void cmdDrawIndirect(ICommandList*, const BufferBindingInfo&, const BufferBindingInfo&, uint32_t, uint32_t) {}

    void cmdDrawIndexedIndirect(ICommandList*, const BufferBindingInfo&, const BufferBindingInfo&, uint32_t, uint32_t) {}

    void cmdDrawMeshTasksIndirect(ICommandList*, const BufferBindingInfo&, const BufferBindingInfo&, uint32_t, uint32_t) {}

    void cmdDispatch(ICommandList*, uint32_t, uint32_t, uint32_t) {}

    void cmdDispatchIndirect(ICommandList*, const BufferBindingInfo&) {}

    void cmdDispatchRays(ICommandList*, const DispatchRaysInfo&) {}

    void cmdDispatchRaysIndirect(ICommandList*, const BufferBindingInfo&) {}

    void cmdCopyBuffer(ICommandList*, const Buffer*, uint64_t, const Buffer*, uint64_t, uint64_t) {}

    void cmdCopyImage(ICommandList*, const ImageCopyInfo&, const ImageCopyInfo&, const ImageSize3D&) {}

    void cmdCopyImageToBuffer(ICommandList*, const ImageCopyInfo&, const ImageMemoryLayout&, const Buffer*) {}

    void cmdCopyBufferToImage(ICommandList*, const Buffer*, const ImageCopyInfo&, const ImageMemoryLayout&) {}

    void cmdCopyAccelerationStructure(ICommandList*, AccelerationStructure*, const AccelerationStructure*, CopyMode) {}

    void cmdCopyMicromap(ICommandList*, Micromap*, const Micromap*, CopyMode) {}

    void cmdClearBuffer(ICommandList*, Buffer*, size_t, size_t) {}

    void cmdBuildTlas(ICommandList*, TL::Span<const TlasBuildInfo>) {}

    void cmdBuildBlas(ICommandList*, TL::Span<const BlasBuildInfo>) {}

    void cmdBuildMicromaps(ICommandList*, TL::Span<const MicromapBuildInfo>) {}

    void cmdWriteAccelerationStructuresSizes(ICommandList*, TL::Span<const AccelerationStructure*>, QueryPool*, uint32_t) {}

    void cmdWriteMicromapsSizes(ICommandList*, TL::Span<const Micromap*>, QueryPool*, uint32_t) {}
} // namespace RHI::D3D12
