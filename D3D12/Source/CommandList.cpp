#include "CommandList.hpp"

#include "Device.hpp"
#include "Resources.hpp"

namespace RHI::D3D12
{
    ///////////////////////////////////////////////////////////
    // ICommandPool
    ///////////////////////////////////////////////////////////

    ResultCode ICommandPool::Init(IDevice* device, const CommandPoolCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void ICommandPool::Shutdown(IDevice* device)
    {
        (void)device;
    }

    void ICommandPool::Reset()
    {
    }

    CommandList* ICommandPool::Allocate()
    {
        return nullptr;
    }

    ///////////////////////////////////////////////////////////
    // ICommandList
    ///////////////////////////////////////////////////////////

    ICommandList::ICommandList()  = default;
    ICommandList::~ICommandList() = default;

    ResultCode ICommandList::Init(IDevice* device, CommandPool* pool, const CommandListCreateInfo& createInfo)
    {
        (void)device;
        (void)pool;
        (void)createInfo;
        return ResultCode::Success;
    }

    void ICommandList::Shutdown()
    {
    }

    void ICommandList::Begin()
    {
    }

    void ICommandList::End()
    {
    }

    void ICommandList::PushDebugMarker(const char* name, uint32_t bgra)
    {
        (void)name;
        (void)bgra;
    }

    void ICommandList::PopDebugMarker()
    {
    }

    void ICommandList::InsertDebugMarker(const char* name, uint32_t bgra)
    {
        (void)name;
        (void)bgra;
    }

    void ICommandList::AddPipelineBarrier(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers)
    {
        (void)barriers;
        (void)imageBarriers;
        (void)bufferBarriers;
    }

    void ICommandList::BeginRenderPass(const RenderPassBeginInfo& beginInfo)
    {
        (void)beginInfo;
    }

    void ICommandList::EndRenderPass()
    {
    }

    void ICommandList::BeginComputePass(const ComputePassBeginInfo& beginInfo)
    {
        (void)beginInfo;
    }

    void ICommandList::EndComputePass()
    {
    }

    void ICommandList::BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted)
    {
        (void)conditionBuffer;
        (void)inverted;
    }

    void ICommandList::EndConditionalCommands()
    {
    }

    void ICommandList::Execute(TL::Span<const CommandList*> commandLists)
    {
        (void)commandLists;
    }

    void ICommandList::BindPipelineLayout(BindPoint bindPoint, const PipelineLayout* pipelineLayout)
    {
        (void)bindPoint;
        (void)pipelineLayout;
    }

    void ICommandList::SetPushConstants(BindPoint bindPoint, uint32_t offset, TL::Block content)
    {
        (void)bindPoint;
        (void)offset;
        (void)content;
    }

    void ICommandList::PushBindGroup(BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos)
    {
        (void)bindPoint;
        (void)firstGroup;
        (void)updateInfos;
    }

    void ICommandList::SetBindGroups(BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        (void)bindPoint;
        (void)bindGroups;
    }

    void ICommandList::BindGraphicsPipeline(const GraphicsPipeline* pipelineState)
    {
        (void)pipelineState;
    }

    void ICommandList::BindComputePipeline(const ComputePipeline* pipelineState)
    {
        (void)pipelineState;
    }

    void ICommandList::BindRayTracingPipeline(const RayTracingPipeline* pipelineState)
    {
        (void)pipelineState;
    }

    void ICommandList::SetViewport(const Viewport& viewport)
    {
        (void)viewport;
    }

    void ICommandList::SetScissor(const Scissor& sicssor)
    {
        (void)sicssor;
    }

    void ICommandList::BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers)
    {
        (void)firstBinding;
        (void)vertexBuffers;
    }

    void ICommandList::BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType)
    {
        (void)indexBuffer;
        (void)indexType;
    }

    void ICommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        (void)vertexCount;
        (void)instanceCount;
        (void)firstVertex;
        (void)firstInstance;
    }

    void ICommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
    {
        (void)indexCount;
        (void)instanceCount;
        (void)firstIndex;
        (void)vertexOffset;
        (void)firstInstance;
    }

    void ICommandList::DrawMeshTasks(uint32_t x, uint32_t y, uint32_t z)
    {
        (void)x;
        (void)y;
        (void)z;
    }

    void ICommandList::DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        (void)argumentBuffer;
        (void)countBuffer;
        (void)maxDrawCount;
        (void)stride;
    }

    void ICommandList::DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        (void)argumentBuffer;
        (void)countBuffer;
        (void)maxDrawCount;
        (void)stride;
    }

    void ICommandList::DrawMeshTasksIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride)
    {
        (void)argumentBuffer;
        (void)countBuffer;
        (void)drawNum;
        (void)stride;
    }

    void ICommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z)
    {
        (void)x;
        (void)y;
        (void)z;
    }

    void ICommandList::DispatchIndirect(const BufferBindingInfo& argumentBuffer)
    {
        (void)argumentBuffer;
    }

    void ICommandList::DispatchRays(const DispatchRaysInfo& dispatchRaysDesc)
    {
        (void)dispatchRaysDesc;
    }

    void ICommandList::DispatchRaysIndirect(const BufferBindingInfo& argumentBuffer)
    {
        (void)argumentBuffer;
    }

    void ICommandList::CopyBuffer(const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size)
    {
        (void)srcBuffer;
        (void)srcOffset;
        (void)dstBuffer;
        (void)dstOffset;
        (void)size;
    }

    void ICommandList::CopyImage(const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size)
    {
        (void)srcImage;
        (void)dstImage;
        (void)size;
    }

    void ICommandList::CopyImageToBuffer(const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer)
    {
        (void)srcImage;
        (void)layout;
        (void)dstBuffer;
    }

    void ICommandList::CopyBufferToImage(const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout)
    {
        (void)srcBuffer;
        (void)dstImage;
        (void)layout;
    }

    void ICommandList::CopyAccelerationStructure(AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode)
    {
        (void)dst;
        (void)src;
        (void)copyMode;
    }

    void ICommandList::CopyMicromap(Micromap* dst, const Micromap* src, CopyMode copyMode)
    {
        (void)dst;
        (void)src;
        (void)copyMode;
    }

    void ICommandList::BuildTlas(TL::Span<const TlasBuildInfo> buildInfos)
    {
        (void)buildInfos;
    }

    void ICommandList::BuildBlas(TL::Span<const BlasBuildInfo> buildInfos)
    {
        (void)buildInfos;
    }

    void ICommandList::BuildMicromaps(TL::Span<const MicromapBuildInfo> buildInfos)
    {
        (void)buildInfos;
    }

    void ICommandList::WriteAccelerationStructuresSizes(TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset)
    {
        (void)accelerationStructures;
        (void)queryPool;
        (void)queryPoolOffset;
    }

    void ICommandList::WriteMicromapsSizes(TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset)
    {
        (void)micromaps;
        (void)queryPool;
        (void)queryPoolOffset;
    }
} // namespace RHI::D3D12
