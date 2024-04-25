#pragma once

#include <RHI/FrameScheduler.hpp>
#include <RHI/CommandList.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;
    class ICommandList;

    class ICommandList final : public CommandList
    {
    public:
        ICommandList(IContext* context, VkCommandPool commandPool, VkCommandBuffer commandBuffer);
        ~ICommandList() = default;

        // clang-format off
        void Begin()                                                                       override;
        void Begin(Pass& pass)                                                             override;
        void End()                                                                         override;
        void DebugMarkerPush(const char* name, ColorValue<float> color)                    override;
        void DebugMarkerPop()                                                              override;
        void BeginConditionalCommands(Handle<Buffer> buffer, size_t offset, bool inverted) override;
        void EndConditionalCommands()                                                      override;
        void Execute(TL::Span<const CommandList*> commandLists)                            override;
        void SetViewport(const Viewport& viewport)                                         override;
        void SetSicssor(const Scissor& sicssor)                                            override;
        void Draw(const DrawInfo& drawInfo)                                                override;
        void Dispatch(const DispatchInfo& dispatchInfo)                                    override;
        void Copy(const BufferCopyInfo& copyInfo)                                          override;
        void Copy(const ImageCopyInfo& copyInfo)                                           override;
        void Copy(const BufferToImageCopyInfo& copyInfo)                                   override;
        void Copy(const ImageToBufferCopyInfo& copyInfo)                                   override;
        // clang-format on

        void PipelineBarrier(TL::Span<const VkMemoryBarrier2> memoryBarriers,
                             TL::Span<const VkBufferMemoryBarrier2> bufferBarriers,
                             TL::Span<const VkImageMemoryBarrier2> imageBarriers);

        void RenderingBegin(TL::Span<const VkRenderingAttachmentInfo> attachmentInfos,
                            const VkRenderingAttachmentInfo* depthAttachmentInfo,
                            ImageSize2D extent);

        void RenderingEnd();

        VkCommandBuffer m_commandBuffer;
        VkCommandPool m_commandPool;

        // TODO: Move to pass
        TL::Vector<VkSemaphoreSubmitInfo> m_signalSemaphores;
        TL::Vector<VkSemaphoreSubmitInfo> m_waitSemaphores;

    private:
        void BindShaderBindGroups(
            VkPipelineBindPoint bindPoint,
            VkPipelineLayout pipelineLayout,
            TL::Span<Handle<BindGroup>> bindGroups);

    private:
        IContext* m_context;
        Pass* m_pass;
    };

} // namespace RHI::Vulkan