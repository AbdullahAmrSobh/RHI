#pragma once

#include <RHI/FrameScheduler.hpp>

#include <vk_mem_alloc.h>

namespace Vulkan
{
    struct Allocation;

    struct Fence;

    class Context;
    class CommandList;
    class CommandListAllocator;

    class Pass final : public RHI::Pass
    {
        friend class CommandList;
        friend class FrameScheduler;

    public:
        Pass(Context* context, const char* name, RHI::QueueType queueType);
        ~Pass();

        VkResult Init();

        Context* m_context;
        VkSemaphore m_signalSemaphore;
    };

    class FrameScheduler final : public RHI::FrameScheduler
    {
    public:
        FrameScheduler(Context* context);
        ~FrameScheduler();

        VkResult Init();

        void OnBegin() override;
        void OnEnd() override;

        void DeviceWaitIdle() override;
        void QueuePassSubmit(RHI::Pass* pass, RHI::Fence* signalFence) override;
        void QueueImagePresent(RHI::ImageAttachment* attachments, RHI::Fence* signalFence) override;
        void QueueCommandsSubmit(RHI::QueueType queueType, RHI::TL::Span<RHI::CommandList*> commandLists, RHI::Fence* signalFence) override;

        static VkSemaphoreSubmitInfo GetPassSignalSemaphoreSubmitInfo(Pass* pass);
        static std::vector<VkSemaphoreSubmitInfo> GetPassWaitSemaphoresInfos(RHI::TL::Span<Pass*> passes);
        static std::vector<VkCommandBufferSubmitInfo> GetPassCommandBuffersSubmitInfos(RHI::TL::Span<CommandList*> commandLists);
    };

} // namespace Vulkan