#pragma once

#include <RHI/CommandList.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;

    enum BarrierSlot
    {
        Priloge,
        Epiloge,
        // Resolve,
        Count,
    };

    struct PipelineBarriers
    {
        TL::Vector<VkMemoryBarrier2>       memoryBarriers = {};
        TL::Vector<VkBufferMemoryBarrier2> bufferBarriers = {};
        TL::Vector<VkImageMemoryBarrier2>  imageBarriers  = {};
    };

    // TODO: remove
    struct PipelineBarriers2
    {
        TL::Span<const VkMemoryBarrier2>       memoryBarriers = {};
        TL::Span<const VkBufferMemoryBarrier2> bufferBarriers = {};
        TL::Span<const VkImageMemoryBarrier2>  imageBarriers  = {};
    };

    VkImageSubresourceLayers ConvertSubresourceLayer(const ImageSubresourceLayers& subresource);

    class ICommandList final : public CommandList
    {
    public:
        ICommandList(IDevice* device, VkCommandBuffer commandBuffer);
        ~ICommandList();

        void PipelineBarrier(
            TL::Span<const VkMemoryBarrier2>       memoryBarriers,
            TL::Span<const VkBufferMemoryBarrier2> bufferBarriers,
            TL::Span<const VkImageMemoryBarrier2>  imageBarriers);

        void PipelineBarrier(const PipelineBarriers& barriers)
        {
            PipelineBarrier(barriers.memoryBarriers, barriers.bufferBarriers, barriers.imageBarriers);
        }

        void BindShaderBindGroups(
            VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<const BindGroupBindingInfo> bindGroups);

        // Interface implementation
        void Begin() override;
        void End() override;
        void BeginRenderPass(const RenderPassBeginInfo& beginInfo) override;
        void EndRenderPass() override;
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
        void Draw(const DrawParameters& parameters) override;
        void Dispatch(const DispatchParameters& parameters) override;
        void CopyBuffer(const BufferCopyInfo& copyInfo) override;
        void CopyImage(const ImageCopyInfo& copyInfo) override;
        void CopyImageToBuffer(const BufferImageCopyInfo& copyInfo) override;
        void CopyBufferToImage(const BufferImageCopyInfo& copyInfo) override;
        void BlitImage(const ImageBlitInfo& blitInfo) override;

        VkCommandBuffer GetHandle() { return m_commandBuffer; }

        /// @todo: optimize pipeline stages
        VkPipelineStageFlags2 GetPipelineStages() const { return VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT; }

    private:
        IDevice*              m_device;
        VkCommandBuffer       m_commandBuffer;
        PipelineBarriers      m_barriers[BarrierSlot::Count];
        VkPipelineStageFlags2 m_signalPipelineStages;

        struct State
        {
            bool hasVertexBuffer         : 1;
            bool hasIndexBuffer          : 1;
            bool isGraphicsPipelineBound : 1;
            bool isComputePipelineBound  : 1;
            bool hasViewportSet          : 1;
            bool hasScissorSet           : 1;
        } m_state;
    };
} // namespace RHI::Vulkan