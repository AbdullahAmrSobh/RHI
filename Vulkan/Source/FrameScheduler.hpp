#pragma once

#include <RHI/FrameScheduler.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

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