#pragma once

#include <RHI/CommandList.hpp>
#include <RHI/Common/Result.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

    class ICommandPool final : public CommandPool
    {
    public:
        ICommandPool(IContext* context);
        ~ICommandPool();

        ResultCode Init(CommandPoolFlags flags);

        void Reset() override;
        TL::Vector<CommandList*> Allocate(QueueType queueType, CommandListLevel level, uint32_t count) override;

    private:
        TL::Vector<VkCommandBuffer> AllocateCommandBuffers(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level);

    private:
        IContext* m_context;
        VkCommandPool m_commandPools[uint32_t(QueueType::Count)];
    };

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

        void BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> bindingInfos);

        void BindIndexBuffer(const BufferBindingInfo& bindingInfo, VkIndexType indexType);

        // Interface implementation
        void Begin() override;
        void Begin(const CommandListBeginInfo& beginInfo) override;
        void End() override;
        void DebugMarkerPush(const char* name, ColorValue<float> color) override;
        void DebugMarkerPop() override;
        void BeginConditionalCommands(Handle<Buffer> buffer, size_t offset, bool inverted) override;
        void EndConditionalCommands() override;
        void Execute(TL::Span<const CommandList*> commandLists) override;
        void SetViewport(const Viewport& viewport) override;
        void SetSicssor(const Scissor& sicssor) override;
        void Draw(const DrawInfo& drawInfo) override;
        void Dispatch(const DispatchInfo& dispatchInfo) override;
        void CopyBuffer(const BufferCopyInfo& copyInfo) override;
        void CopyImage(const ImageCopyInfo& copyInfo) override;
        void CopyImageToBuffer(const BufferImageCopyInfo& copyInfo) override;
        void CopyBufferToImage(const BufferImageCopyInfo& copyInfo) override;

        VkCommandBuffer m_commandBuffer;

        class IPassSubmitData* m_executeContext;

    private:
        IContext* m_context;
    };
} // namespace RHI::Vulkan