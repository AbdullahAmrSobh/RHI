#include "CommandList.hpp"
#include "Common.hpp"
#include "Device.hpp"
#include "Resources.hpp"

namespace RHI::WebGPU
{
    //////////////////////////////////////////////////////////////////////////////////////////
    /// ICommandPool
    //////////////////////////////////////////////////////////////////////////////////////////

    ResultCode ICommandPool::Init(IDevice* device, const CommandPoolCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        // TODO: implement WebGPU command pool
        return ResultCode::Success;
    }

    void ICommandPool::Shutdown(IDevice* device)
    {
        (void)device;
        // TODO: implement WebGPU command pool teardown
    }

    void commandPoolReset(ICommandPool* self)
    {
        (void)self;
        // TODO: implement
    }

    CommandList* commandPoolAllocate(ICommandPool* self)
    {
        (void)self;
        // TODO: implement
        return nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// ICommandList
    //////////////////////////////////////////////////////////////////////////////////////////

    void cmdBegin(ICommandList* self)
    {
        (void)self;
        // TODO: wgpuDeviceCreateCommandEncoder
    }

    void cmdEnd(ICommandList* self)
    {
        (void)self;
        // TODO: wgpuCommandEncoderFinish
    }

    void cmdPushDebugMarker(ICommandList* self, const char* name, uint32_t bgra)
    {
        (void)self;
        (void)name;
        (void)bgra;
        // TODO: wgpuCommandEncoderPushDebugGroup
    }

    void cmdPopDebugMarker(ICommandList* self)
    {
        (void)self;
        // TODO: wgpuCommandEncoderPopDebugGroup
    }

    void cmdInsertDebugMarker(ICommandList* self, const char* name, uint32_t bgra)
    {
        (void)self;
        (void)name;
        (void)bgra;
        // TODO: wgpuCommandEncoderInsertDebugMarker
    }

    void cmdAddPipelineBarrier(ICommandList* self, TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers)
    {
        (void)self;
        (void)barriers;
        (void)imageBarriers;
        (void)bufferBarriers;
        // TODO: WebGPU has no explicit barrier API; usage transitions are implicit.
    }

    void cmdBeginRenderPass(ICommandList* self, const RenderPassBeginInfo& beginInfo)
    {
        (void)self;
        (void)beginInfo;
        // TODO: wgpuCommandEncoderBeginRenderPass
    }

    void cmdEndRenderPass(ICommandList* self)
    {
        (void)self;
        // TODO: wgpuRenderPassEncoderEnd
    }

    void cmdBeginComputePass(ICommandList* self, const ComputePassBeginInfo& beginInfo)
    {
        (void)self;
        (void)beginInfo;
        // TODO: wgpuCommandEncoderBeginComputePass
    }

    void cmdEndComputePass(ICommandList* self)
    {
        (void)self;
        // TODO: wgpuComputePassEncoderEnd
    }

    void cmdBeginConditionalCommands(ICommandList* self, const BufferBindingInfo& conditionBuffer, bool inverted)
    {
        (void)self;
        (void)conditionBuffer;
        (void)inverted;
        // TODO: WebGPU has no conditional rendering extension; unsupported.
    }

    void cmdEndConditionalCommands(ICommandList* self)
    {
        (void)self;
        // TODO: unsupported on WebGPU
    }

    void cmdExecute(ICommandList* self, TL::Span<const CommandList*> commandLists)
    {
        (void)self;
        (void)commandLists;
        // TODO: WebGPU has no secondary command buffers; unsupported.
    }

    void cmdBindPipelineLayout(ICommandList* self, BindPoint bindPoint, const PipelineLayout* pipelineLayout)
    {
        (void)self;
        (void)bindPoint;
        (void)pipelineLayout;
        // TODO: implement
    }

    void cmdSetPushConstants(ICommandList* self, BindPoint bindPoint, uint32_t offset, TL::Block content)
    {
        (void)self;
        (void)bindPoint;
        (void)offset;
        (void)content;
        // TODO: WebGPU has no push constants; emulate via a uniform buffer.
    }

    void cmdPushBindGroup(ICommandList* self, BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos)
    {
        (void)self;
        (void)bindPoint;
        (void)firstGroup;
        (void)updateInfos;
        // TODO: WebGPU has no push-descriptor equivalent; requires a transient bind group.
    }

    void cmdSetBindGroups(ICommandList* self, BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        (void)self;
        (void)bindPoint;
        (void)bindGroups;
        // TODO: wgpuRenderPassEncoderSetBindGroup / wgpuComputePassEncoderSetBindGroup
    }

    void cmdBindGraphicsPipeline(ICommandList* self, const GraphicsPipeline* pipelineState)
    {
        (void)self;
        (void)pipelineState;
        // TODO: wgpuRenderPassEncoderSetPipeline
    }

    void cmdBindComputePipeline(ICommandList* self, const ComputePipeline* pipelineState)
    {
        (void)self;
        (void)pipelineState;
        // TODO: wgpuComputePassEncoderSetPipeline
    }

    void cmdBindRayTracingPipeline(ICommandList* self, const RayTracingPipeline* pipelineState)
    {
        (void)self;
        (void)pipelineState;
        // TODO: unsupported on WebGPU
    }

    void cmdSetViewport(ICommandList* self, float offsetX, float offsetY, float width, float height, float minDepth, float maxDepth)
    {
        (void)self;
        (void)offsetX;
        (void)offsetY;
        (void)width;
        (void)height;
        (void)minDepth;
        (void)maxDepth;
        // TODO: wgpuRenderPassEncoderSetViewport
    }

    void cmdSetScissor(ICommandList* self, int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height)
    {
        (void)self;
        (void)offsetX;
        (void)offsetY;
        (void)width;
        (void)height;
        // TODO: wgpuRenderPassEncoderSetScissorRect
    }

    void cmdBindVertexBuffers(ICommandList* self, uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers)
    {
        (void)self;
        (void)firstBinding;
        (void)vertexBuffers;
        // TODO: wgpuRenderPassEncoderSetVertexBuffer
    }

    void cmdBindIndexBuffer(ICommandList* self, const BufferBindingInfo& indexBuffer, IndexType indexType)
    {
        (void)self;
        (void)indexBuffer;
        (void)indexType;
        // TODO: wgpuRenderPassEncoderSetIndexBuffer
    }

    void cmdDraw(ICommandList* self, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        (void)self;
        (void)vertexCount;
        (void)instanceCount;
        (void)firstVertex;
        (void)firstInstance;
        // TODO: wgpuRenderPassEncoderDraw
    }

    void cmdDrawIndexed(ICommandList* self, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
    {
        (void)self;
        (void)indexCount;
        (void)instanceCount;
        (void)firstIndex;
        (void)vertexOffset;
        (void)firstInstance;
        // TODO: wgpuRenderPassEncoderDrawIndexed
    }

    void cmdDrawMeshTasks(ICommandList* self, uint32_t x, uint32_t y, uint32_t z)
    {
        (void)self;
        (void)x;
        (void)y;
        (void)z;
        // TODO: unsupported on WebGPU
    }

    void cmdDrawIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        (void)self;
        (void)argumentBuffer;
        (void)countBuffer;
        (void)maxDrawCount;
        (void)stride;
        // TODO: wgpuRenderPassEncoderDrawIndirect (no built-in count-buffer support)
    }

    void cmdDrawIndexedIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        (void)self;
        (void)argumentBuffer;
        (void)countBuffer;
        (void)maxDrawCount;
        (void)stride;
        // TODO: wgpuRenderPassEncoderDrawIndexedIndirect (no built-in count-buffer support)
    }

    void cmdDrawMeshTasksIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride)
    {
        (void)self;
        (void)argumentBuffer;
        (void)countBuffer;
        (void)drawNum;
        (void)stride;
        // TODO: unsupported on WebGPU
    }

    void cmdDispatch(ICommandList* self, uint32_t x, uint32_t y, uint32_t z)
    {
        (void)self;
        (void)x;
        (void)y;
        (void)z;
        // TODO: wgpuComputePassEncoderDispatchWorkgroups
    }

    void cmdDispatchIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer)
    {
        (void)self;
        (void)argumentBuffer;
        // TODO: wgpuComputePassEncoderDispatchWorkgroupsIndirect
    }

    void cmdDispatchRays(ICommandList* self, const DispatchRaysInfo& dispatchRaysDesc)
    {
        (void)self;
        (void)dispatchRaysDesc;
        // TODO: unsupported on WebGPU
    }

    void cmdDispatchRaysIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer)
    {
        (void)self;
        (void)argumentBuffer;
        // TODO: unsupported on WebGPU
    }

    void cmdCopyBuffer(ICommandList* self, const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size)
    {
        (void)self;
        (void)srcBuffer;
        (void)srcOffset;
        (void)dstBuffer;
        (void)dstOffset;
        (void)size;
        // TODO: wgpuCommandEncoderCopyBufferToBuffer
    }

    void cmdCopyImage(ICommandList* self, const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size)
    {
        (void)self;
        (void)srcImage;
        (void)dstImage;
        (void)size;
        // TODO: wgpuCommandEncoderCopyTextureToTexture
    }

    void cmdCopyImageToBuffer(ICommandList* self, const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer)
    {
        (void)self;
        (void)srcImage;
        (void)layout;
        (void)dstBuffer;
        // TODO: wgpuCommandEncoderCopyTextureToBuffer
    }

    void cmdCopyBufferToImage(ICommandList* self, const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout)
    {
        (void)self;
        (void)srcBuffer;
        (void)dstImage;
        (void)layout;
        // TODO: wgpuCommandEncoderCopyBufferToTexture
    }

    void cmdCopyAccelerationStructure(ICommandList* self, AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode)
    {
        (void)self;
        (void)dst;
        (void)src;
        (void)copyMode;
        // TODO: unsupported on WebGPU
    }

    void cmdCopyMicromap(ICommandList* self, Micromap* dst, const Micromap* src, CopyMode copyMode)
    {
        (void)self;
        (void)dst;
        (void)src;
        (void)copyMode;
        // TODO: unsupported on WebGPU
    }

    void cmdClearBuffer(ICommandList* self, Buffer* dst, size_t offset, size_t size)
    {
        (void)self;
        (void)dst;
        (void)offset;
        (void)size;
        // TODO: wgpuCommandEncoderClearBuffer
    }

    void cmdBuildTlas(ICommandList* self, TL::Span<const TlasBuildInfo> buildInfos)
    {
        (void)self;
        (void)buildInfos;
        // TODO: unsupported on WebGPU
    }

    void cmdBuildBlas(ICommandList* self, TL::Span<const BlasBuildInfo> buildInfos)
    {
        (void)self;
        (void)buildInfos;
        // TODO: unsupported on WebGPU
    }

    void cmdBuildMicromaps(ICommandList* self, TL::Span<const MicromapBuildInfo> buildInfos)
    {
        (void)self;
        (void)buildInfos;
        // TODO: unsupported on WebGPU
    }

    void cmdWriteAccelerationStructuresSizes(ICommandList* self, TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset)
    {
        (void)self;
        (void)accelerationStructures;
        (void)queryPool;
        (void)queryPoolOffset;
        // TODO: unsupported on WebGPU
    }

    void cmdWriteMicromapsSizes(ICommandList* self, TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset)
    {
        (void)self;
        (void)micromaps;
        (void)queryPool;
        (void)queryPoolOffset;
        // TODO: unsupported on WebGPU
    }
} // namespace RHI::WebGPU
