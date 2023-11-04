#pragma once

#include <RHI/CommandList.hpp>

#include <RHI/Span.hpp>

#include <vulkan/vulkan.h>

#include <memory>

namespace Vulkan
{

    class Pass;
    class Context;
    class FrameScheduler;
    class CommandList;

    class CommandListAllocator final
    {
    public:
        enum CommandLevel
        {
            Primary,
            Secondary,
        };

        CommandListAllocator(Context* context);
        ~CommandListAllocator();

        std::vector<CommandList> AllocateCommandLists(CommandLevel level, uint32_t count) const;
        std::unique_ptr<CommandList> AllocateCommandList(CommandLevel level) const;
    };

    class CommandList final : public RHI::CommandList
    {
    public:
        CommandList(Context* context)
            : m_context(context)
        {
        }

        CommandList(const CommandList& other) = delete;
        CommandList(CommandList&& other);
        ~CommandList();

        VkResult Init(const VkCommandBufferAllocateInfo& allocateInfo);

        void Begin();
        void End();

        void TransitionPassAttachments(FrameScheduler* scheduler, RHI::TL::Span<RHI::ImagePassAttachment*> passAttachments);
        void TransitionPassAttachments(FrameScheduler* scheduler, RHI::TL::Span<RHI::BufferPassAttachment*> passAttachments);

        void RenderingBegin(FrameScheduler& scheduler, Pass& pass);
        void RenderingEnd(FrameScheduler& scheduler, Pass& pass);

        void PushDebugMarker(const char* name);
        void PopDebugMarker();

        void SetViewport(const RHI::Viewport& viewport) override;

        void SetSicssor(const RHI::Scissor& sicssor) override;

        void Submit(const RHI::CommandDraw& command) override;

        void Submit(const RHI::CommandCopy& command) override;

        void Submit(const RHI::CommandCompute& command) override;

    private:
        VkImageMemoryBarrier2 TransitionResource(FrameScheduler* scheduler, RHI::ImagePassAttachment* resourceBefore, RHI::ImagePassAttachment* resourceAfter);
        VkBufferMemoryBarrier2 TransitionResource(FrameScheduler* scheduler, RHI::BufferPassAttachment* resourceBefore, RHI::BufferPassAttachment* resourceAfter);

    public:
        Context* m_context;

        VkCommandPool m_commandPool = VK_NULL_HANDLE;
        VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    };

} // namespace Vulkan