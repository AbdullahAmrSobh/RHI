#pragma once

#include <RHI/CommandList.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;

    struct PipelineBarriers
    {
        TL::Span<const VkMemoryBarrier2>       memoryBarriers = {};
        TL::Span<const VkImageMemoryBarrier2>  imageBarriers  = {};
        TL::Span<const VkBufferMemoryBarrier2> bufferBarriers = {};
    };

    VkImageSubresourceLayers ConvertSubresourceLayer(const ImageSubresourceLayers& subresource);

    VkResolveModeFlagBits ConvertResolveMode(ResolveMode resolveMode);

    class ICommandList final : public CommandList
    {
    public:
        ICommandList(IDevice* device, VkCommandBuffer commandBuffer);
        ~ICommandList();

        void Begin();
        void End();

        void AddPipelineBarriers(const PipelineBarriers& barriers);

        void BeginPass(const Pass& pass);
        void EndPass();

        void BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<const BindGroupBindingInfo> bindGroups);

        // Interface implementation
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

        VkCommandBuffer GetHandle() const { return m_commandBuffer; }

    private:
        IDevice*        m_device                      = nullptr;
        VkCommandBuffer m_commandBuffer               = VK_NULL_HANDLE;
        bool            m_isInsideRenderPass      : 1 = false;
        bool            m_hasVertexBuffer         : 1 = false;
        bool            m_hasIndexBuffer          : 1 = false;
        bool            m_isGraphicsPipelineBound : 1 = false;
        bool            m_isComputePipelineBound  : 1 = false;
        bool            m_hasViewportSet          : 1 = false;
        bool            m_hasScissorSet           : 1 = false;
    };
} // namespace RHI::Vulkan