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

        void Begin();
        void End();

        // Interface implementation
        void DebugMarkerPush(const char* name, ColorValue<float> color) override;
        void DebugMarkerPop() override;
        void BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted) override;
        void EndConditionalCommands() override;
        void Execute(TL::Span<const CommandList*> commandLists) override;
        void BindGraphicsPipeline(Handle<GraphicsPipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups) override;
        void BindComputePipeline(Handle<ComputePipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups) override;
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
        void BlitImage(const ImageBlitInfo& blitInfo) override;

    public:
        IDevice* m_device = nullptr;
        bool     m_isRenderPass;

        WGPUCommandBuffer  m_cmdBuffer  = nullptr;
        WGPUCommandEncoder m_cmdEncoder = nullptr;
        WGPURenderBundle   m_bundle     = nullptr;
        union
        {
            WGPURenderPassEncoder  m_renderPassEncoder = {};
            WGPUComputePassEncoder m_computePassEncoder;
        };
    };
} // namespace RHI::WebGPU