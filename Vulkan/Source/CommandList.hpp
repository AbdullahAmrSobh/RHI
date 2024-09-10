#pragma once

#include <RHI/CommandList.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

    enum BarrierSlot
    {
        Priloge,
        Epiloge,
        // Resolve,
        Count,
    };

    struct PipelineBarriers
    {
        TL::Vector<VkMemoryBarrier2> memoryBarriers;
        TL::Vector<VkBufferMemoryBarrier2> bufferBarriers;
        TL::Vector<VkImageMemoryBarrier2> imageBarriers;
    };

    VkImageSubresourceLayers ConvertSubresourceLayer(const ImageSubresourceLayers& subresource);

    class ICommandList final : public CommandList
    {
    public:
        ICommandList(IContext* context, VkCommandBuffer commandBuffer);
        ~ICommandList();

        void BeginRendering(
            VkRect2D renderingArea,
            TL::Span<const VkRenderingAttachmentInfo> colorAttachments,
            VkRenderingAttachmentInfo* depthAttachment,
            VkRenderingAttachmentInfo* stencilAttachment);

        void EndRendedring();

        void PipelineBarrier(
            TL::Span<const VkMemoryBarrier2> memoryBarriers,
            TL::Span<const VkBufferMemoryBarrier2> bufferBarriers,
            TL::Span<const VkImageMemoryBarrier2> imageBarriers);

        void BindShaderBindGroups(
            VkPipelineBindPoint bindPoint,
            VkPipelineLayout pipelineLayout,
            TL::Span<const BindGroupBindingInfo> bindGroups);

        // Interface implementation
        void Begin() override;
        void Begin(const CommandListBeginInfo& beginInfo) override;
        void End() override;
        void DebugMarkerPush(const char* name, ColorValue<float> color) override;
        void DebugMarkerPop() override;
        void BeginConditionalCommands(Handle<Buffer> buffer, size_t offset, bool inverted) override;
        void EndConditionalCommands() override;
        void Execute(TL::Span<const CommandList*> commandLists) override;
        void BindGraphicsPipeline(Handle<GraphicsPipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups) override;
        void BindComputePipeline(Handle<ComputePipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups) override;
        void SetViewport(const Viewport& viewport) override;
        void SetSicssor(const Scissor& sicssor) override;
        void BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers) override;
        void BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType) override;
        void Draw(const DrawInfo& drawInfo) override;
        void Dispatch(const DispatchInfo& dispatchInfo) override;
        void CopyBuffer(const BufferCopyInfo& copyInfo) override;
        void CopyImage(const ImageCopyInfo& copyInfo) override;
        void CopyImageToBuffer(const BufferImageCopyInfo& copyInfo) override;
        void CopyBufferToImage(const BufferImageCopyInfo& copyInfo) override;
        void BlitImage(Handle<ImageView> srcImage, Handle<ImageView> dstImage, TL::Span<ImageBlitInfo> regions, SamplerFilter filter) override;

        IContext* m_context;
        VkCommandBuffer m_commandBuffer;
        PipelineBarriers m_barriers[BarrierSlot::Count];
        TL::Vector<VkSemaphoreSubmitInfo> m_waitSemaphores;
        TL::Vector<VkSemaphoreSubmitInfo> m_signalSemaphores;
        bool m_isRenderPassStarted;
    };
} // namespace RHI::Vulkan