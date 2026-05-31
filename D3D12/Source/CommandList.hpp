#pragma once

#include <RHI/CommandList.hpp>
#include <RHI/Result.hpp>

#include <TL/Containers/Vector.hpp>

#include <d3d12.h>

namespace RHI::D3D12
{
    class IDevice;
    class ICommandList;

    class ICommandPool final : public RHI::CommandPool
    {
    public:
        ResultCode Init(IDevice* device, const CommandPoolCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        void         Reset() override;
        CommandList* Allocate() override;

        IDevice*                  m_device           = nullptr;
        ID3D12CommandAllocator*   m_commandAllocator = nullptr;
        D3D12_COMMAND_LIST_TYPE   m_type             = D3D12_COMMAND_LIST_TYPE_DIRECT;
        TL::Vector<ICommandList*> m_commandLists;
    };

    class ICommandList final : public RHI::CommandList
    {
    public:
        ICommandList();
        ~ICommandList();

        ResultCode Init(IDevice* device, CommandPool* pool, const CommandListCreateInfo& createInfo);
        void       Shutdown();

        // Interface implementation
        void Begin() override;
        void End() override;
        void PushDebugMarker(const char* name, uint32_t bgra) override;
        void PopDebugMarker() override;
        void InsertDebugMarker(const char* name, uint32_t bgra) override;
        void AddPipelineBarrier(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers) override;
        void BeginRenderPass(const RenderPassBeginInfo& beginInfo) override;
        void EndRenderPass() override;
        void BeginComputePass(const ComputePassBeginInfo& beginInfo) override;
        void EndComputePass() override;
        void BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted) override;
        void EndConditionalCommands() override;
        void Execute(TL::Span<const CommandList*> commandLists) override;
        void BindPipelineLayout(BindPoint bindPoint, const PipelineLayout* pipelineLayout) override;
        void SetPushConstants(BindPoint bindPoint, uint32_t offset, TL::Block content) override;
        void PushBindGroup(BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos) override;
        void SetBindGroups(BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups) override;
        void BindGraphicsPipeline(const GraphicsPipeline* pipelineState) override;
        void BindComputePipeline(const ComputePipeline* pipelineState) override;
        void BindRayTracingPipeline(const RayTracingPipeline* pipelineState) override;
        void SetViewport(const Viewport& viewport) override;
        void SetScissor(const Scissor& sicssor) override;
        void BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers) override;
        void BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType) override;
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;
        void DrawMeshTasks(uint32_t x = 1, uint32_t y = 1, uint32_t z = 1) override;
        void DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) override;
        void DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) override;
        void DrawMeshTasksIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride) override;
        void Dispatch(uint32_t x = 1, uint32_t y = 1, uint32_t z = 1) override;
        void DispatchIndirect(const BufferBindingInfo& argumentBuffer) override;
        void DispatchRays(const DispatchRaysInfo& dispatchRaysDesc) override;
        void DispatchRaysIndirect(const BufferBindingInfo& argumentBuffer) override;
        void CopyBuffer(const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size) override;
        void CopyImage(const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size) override;
        void CopyImageToBuffer(const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer) override;
        void CopyBufferToImage(const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout) override;
        void CopyAccelerationStructure(AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode) override;
        void CopyMicromap(Micromap* dst, const Micromap* src, CopyMode copyMode) override;
        void BuildTlas(TL::Span<const TlasBuildInfo> buildInfos) override;
        void BuildBlas(TL::Span<const BlasBuildInfo> buildInfos) override;
        void BuildMicromaps(TL::Span<const MicromapBuildInfo> buildInfos) override;
        void WriteAccelerationStructuresSizes(TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset) override;
        void WriteMicromapsSizes(TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset) override;

    public:
        ID3D12GraphicsCommandList7* GetHandle() const { return m_commandList; }

        IDevice*                    m_device         = nullptr;
        ICommandPool*               m_pool           = nullptr;
        ID3D12GraphicsCommandList7* m_commandList    = nullptr;
        const PipelineLayout*       m_pipelineLayout = nullptr;
    };
} // namespace RHI::D3D12
