#pragma once

#include <RHI/FrameScheduler.hpp>
#include <RHI/CommandList.hpp>

#include <memory>
#include <array>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IPass;
    class IContext;
    class ICommandList;
    class IFence;

    enum class BarrierType
    {
        PrePass,
        PostPass,
        Transition,
    };

    class CommandPool
    {
    public:
        VkResult Init(IContext* context, uint32_t queueFamilyIndex);

        void Shutdown(IContext* context);

        void Reset(IContext* context);

        ICommandList* Allocate(IContext* context);

        void Release(ICommandList* commandList);

    private:
        VkCommandPool m_commandPool;
        std::vector<std::unique_ptr<ICommandList>> m_commandLists;
        std::vector<ICommandList*> m_availableCommandLists;
    };

    class ICommandListAllocator final : public CommandListAllocator
    {
    public:
        ICommandListAllocator(IContext* context);
        ~ICommandListAllocator();

        VkResult Init(QueueType queueType);

        CommandList* Allocate() override;

    private:
        IContext* m_context;
        uint32_t m_maxFrameBufferingCount;
        uint32_t m_currentFrameIndex;
        std::array<CommandPool, 3> m_commandPools;
    };

    class ICommandList final : public CommandList
    {
    public:
        ICommandList(IContext* context, VkCommandBuffer commandBuffer);

        void Begin() override;
        void Begin(Pass& pass) override;
        void End() override;
        void SetViewport(const Viewport& viewport) override;
        void SetSicssor(const Scissor& sicssor) override;
        void Draw(const DrawInfo& command) override;
        void Dispatch(const DispatchInfo& command) override;
        void Copy(const BufferCopyInfo& command) override;
        void Copy(const ImageCopyInfo& command) override;
        void Copy(const BufferToImageCopyInfo& command) override;
        void Copy(const ImageToBufferCopyInfo& command) override;

        VkRenderingAttachmentInfo GetAttachmentInfo(const ImagePassAttachment& passAttachment) const;

        void RenderingBegin(IPass& pass);

        void RenderingEnd(IPass& pass);

        void PushDebugMarker(const char* name);

        void PopDebugMarker();

        void BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<Handle<BindGroup>> bindGroups);

        void TransitionPassAttachments(BarrierType barrierType, TL::Span<ImagePassAttachment*> passAttachments);

        void TransitionPassAttachments(BarrierType barrierType, TL::Span<BufferPassAttachment*> passAttachments);

        VkImageMemoryBarrier2 TransitionPassAttachment(ImagePassAttachment& passAttachment);

        VkImageMemoryBarrier2 TransitionPassAttachment(BufferPassAttachment& passAttachment);

        VkImageMemoryBarrier2 TransitionToPresent(ImagePassAttachment& passAttachment);

        void PipelineBarrier(TL::Span<VkMemoryBarrier2> memoryBarriers, TL::Span<VkBufferMemoryBarrier2> bufferBarriers, TL::Span<VkImageMemoryBarrier2> imageBarriers);

        void PipelineBarrier(TL::Span<VkMemoryBarrier2> memoryBarriers);

        void PipelineBarrier(TL::Span<VkBufferMemoryBarrier2> bufferBarriers);

        void PipelineBarrier(TL::Span<VkImageMemoryBarrier2> imageBarriers);

        IContext* m_context;

        IPass* m_pass;

        VkCommandBuffer m_commandBuffer;
    };

} // namespace RHI::Vulkan