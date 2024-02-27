#pragma once

#include <RHI/FrameScheduler.hpp>

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    struct Allocation;

    struct IFence;

    class IContext;
    class ICommandList;

    class IFrameScheduler final : public FrameScheduler
    {
    public:
        IFrameScheduler(IContext* context);
        ~IFrameScheduler();

        VkResult Init();

        void DeviceWaitIdle() override;
        void QueuePassSubmit(Pass* pass, Fence* fence) override;
        void QueueCommandsSubmit(QueueType queueType, TL::Span<CommandList*> commandLists, Fence& fence) override;
    };

} // namespace RHI::Vulkan