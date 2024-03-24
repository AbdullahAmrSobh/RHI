#pragma once

#include <RHI/FrameScheduler.hpp>
#include <RHI/CommandList.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;
    class ICommandList;

    class ICommandListAllocator final : public CommandListAllocator
    {
    public:
        ICommandListAllocator(IContext* context);

        ~ICommandListAllocator();

        VkResult Init();

        // clang-format off
        void                     Reset()                                       override;
        CommandList*             Allocate(QueueType queueType)                 override;
        TL::Vector<CommandList*> Allocate(QueueType queueType, uint32_t count) override;
        void                     Release(TL::Span<CommandList*> commandLists)  override;
        // clang-format on

    private:
        TL::Vector<VkCommandBuffer> AllocateCommandBuffers(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level);
        void ReleaseCommandBuffers(VkCommandPool pool, TL::Span<VkCommandBuffer> commandBuffers);

    private:
        IContext* m_context;
        TL::Vector<VkCommandPool> m_commandPools[uint32_t(QueueType::Count)];
    };

    class ICommandList final : public CommandList
    {
    public:
        ICommandList(IContext* context, VkCommandPool commandPool, VkCommandBuffer commandBuffer);
        ~ICommandList() = default;

        // clang-format off
        void Begin()                                                                       override;
        void Begin(Pass& pass)                                                             override;
        void End()                                                                         override;
        void DebugMarkerPush(const char* name, const struct ColorValue& color)             override;
        void DebugMarkerPop()                                                              override;
        void BeginConditionalCommands(Handle<Buffer> buffer, size_t offset, bool inverted) override;
        void EndConditionalCommands()                                                      override;
        // TODO: add indirect commands here
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

        void PipelineBarrier(
            TL::Span<const VkMemoryBarrier2> memoryBarriers,
            TL::Span<const VkBufferMemoryBarrier2> bufferBarriers,
            TL::Span<const VkImageMemoryBarrier2> imageBarriers);

        void RenderingBegin(
            TL::Span<const VkRenderingAttachmentInfo> attachmentInfos,
            const VkRenderingAttachmentInfo* depthAttachmentInfo,
            ImageSize2D extent);

        void RenderingEnd();

        VkCommandBuffer m_commandBuffer;
        VkCommandPool m_commandPool;

        TL::Vector<VkSemaphoreSubmitInfo> m_signalSemaphores;
        TL::Vector<VkSemaphoreSubmitInfo> m_waitSemaphores;

    private:
        void BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<Handle<BindGroup>> bindGroups);

    private:
        IContext* m_context;
        Pass* m_pass;
    };

} // namespace RHI::Vulkan