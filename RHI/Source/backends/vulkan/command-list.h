#pragma once

#include <RHI/RHI.h>

#include <TL/Allocator/Arena.hpp>
#include <TL/Containers/InlineVector.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    struct IDevice;
    struct IQueue;
    struct ICommandList;

    struct ICommandPool : RHI::CommandPool
    {
        TL::Arena arena;
        IDevice* device;
        IQueue* queue;
        VkCommandPool commandPool;
        TL::InlineVector<ICommandList*, 64> commandList;

        ResultCode Init(IDevice* device, const CommandPoolCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };

    struct ICommandList : RHI::CommandList
    {
        IDevice* device = nullptr;
        ICommandPool* commandPool = nullptr;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        TL::InlineVector<void*, 64> tracyScopes;
        PipelineLayout* pipelineLayout = nullptr;
        VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;
        bool hasVertexBuffer         : 1;
        bool hasIndexBuffer          : 1;
        bool isGraphicsPipelineBound : 1;
        bool isComputePipelineBound  : 1;
        bool hasViewportSet          : 1;
        bool hasScissorSet           : 1;
    };

    void commandPoolReset(ICommandPool* self);
    CommandList* commandPoolAllocate(ICommandPool* self);

    void cmdBegin(ICommandList* self);
    void cmdEnd(ICommandList* self);
    void cmdPushDebugMarker(ICommandList* self, const char* name, uint32_t bgra);
    void cmdPopDebugMarker(ICommandList* self);
    void cmdInsertDebugMarker(ICommandList* self, const char* name, uint32_t bgra);
    void cmdAddPipelineBarrier(ICommandList* self, TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers);
    void cmdBeginRenderPass(ICommandList* self, const RenderPassBeginInfo& beginInfo);
    void cmdEndRenderPass(ICommandList* self);
    void cmdBeginComputePass(ICommandList* self, const ComputePassBeginInfo& beginInfo);
    void cmdEndComputePass(ICommandList* self);
    void cmdBeginConditionalCommands(ICommandList* self, const BufferBindingInfo& conditionBuffer, bool inverted);
    void cmdEndConditionalCommands(ICommandList* self);
    void cmdExecute(ICommandList* self, TL::Span<const CommandList*> commandLists);
    void cmdBindPipelineLayout(ICommandList* self, BindPoint bindPoint, const PipelineLayout* pipelineLayout);
    void cmdSetPushConstants(ICommandList* self, BindPoint bindPoint, uint32_t offset, TL::Block content);
    void cmdPushBindGroup(ICommandList* self, BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos);
    void cmdSetBindGroups(ICommandList* self, BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups);
    void cmdBindGraphicsPipeline(ICommandList* self, const GraphicsPipeline* pipelineState);
    void cmdBindComputePipeline(ICommandList* self, const ComputePipeline* pipelineState);
    void cmdBindRayTracingPipeline(ICommandList* self, const RayTracingPipeline* pipelineState);
    void cmdSetViewport(ICommandList* self, float offsetX, float offsetY, float width, float height, float minDepth, float maxDepth);
    void cmdSetScissor(ICommandList* self, int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height);
    void cmdBindVertexBuffers(ICommandList* self, uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers);
    void cmdBindIndexBuffer(ICommandList* self, const BufferBindingInfo& indexBuffer, IndexType indexType);
    void cmdDraw(ICommandList* self, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    void cmdDrawIndexed(ICommandList* self, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void cmdDrawMeshTasks(ICommandList* self, uint32_t x, uint32_t y, uint32_t z);
    void cmdDrawIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride);
    void cmdDrawIndexedIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride);
    void cmdDrawMeshTasksIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride);
    void cmdDispatch(ICommandList* self, uint32_t x, uint32_t y, uint32_t z);
    void cmdDispatchIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer);
    void cmdDispatchRays(ICommandList* self, const DispatchRaysInfo& dispatchRaysDesc);
    void cmdDispatchRaysIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer);
    void cmdCopyBuffer(ICommandList* self, const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size);
    void cmdCopyImage(ICommandList* self, const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size);
    void cmdCopyImageToBuffer(ICommandList* self, const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer);
    void cmdCopyBufferToImage(ICommandList* self, const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout);
    void cmdCopyAccelerationStructure(ICommandList* self, AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode);
    void cmdCopyMicromap(ICommandList* self, Micromap* dst, const Micromap* src, CopyMode copyMode);
    void cmdClearBuffer(ICommandList* self, Buffer* dst, size_t offset, size_t size);
    void cmdBuildTlas(ICommandList* self, TL::Span<const TlasBuildInfo> buildInfos);
    void cmdBuildBlas(ICommandList* self, TL::Span<const BlasBuildInfo> buildInfos);
    void cmdBuildMicromaps(ICommandList* self, TL::Span<const MicromapBuildInfo> buildInfos);
    void cmdWriteAccelerationStructuresSizes(ICommandList* self, TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset);
    void cmdWriteMicromapsSizes(ICommandList* self, TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset);
} // namespace RHI::Vulkan
