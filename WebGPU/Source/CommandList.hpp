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
        void PushDebugMarker(const char* name, ClearValue color) override;
        void PopDebugMarker() override;
        void BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted) override;
        void EndConditionalCommands() override;
        void Execute(TL::Span<const CommandList*> commandLists) override;
        void BindGraphicsPipeline(GraphicsPipeline* pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups) override;
        void BindComputePipeline(ComputePipeline* pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups) override;
        void SetViewport(const Viewport& viewport) override;
        void SetScissor(const Scissor& sicssor) override;
        void BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers) override;
        void BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType) override;
        void Draw(const DrawParameters& parameters) override;
        void DrawIndexed(const DrawIndexedParameters& parameters) override;
        void DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) override;
        void DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) override;
        void Dispatch(const DispatchParameters& parameters) override;
        void DispatchIndirect(const BufferBindingInfo& argumentBuffer) override;
        void CopyBuffer(const BufferCopyInfo& copyInfo) override;
        void CopyImage(const ImageCopyInfo& copyInfo) override;
        void CopyImageToBuffer(const BufferImageCopyInfo& copyInfo) override;
        void CopyBufferToImage(const BufferImageCopyInfo& copyInfo) override;

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