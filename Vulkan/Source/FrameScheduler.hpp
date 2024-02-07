#pragma once

#include <RHI/FrameScheduler.hpp>

#include <vk_mem_alloc.h>

namespace Vulkan
{
    struct Allocation;

    struct IFence;

    class IContext;
    class ICommandList;

    class IPass final : public RHI::Pass
    {
        friend class ICommandList;
        friend class IFrameScheduler;

    public:
        IPass(IContext* context, const char* name, RHI::QueueType queueType);
        ~IPass();

        VkResult Init();

        IContext* m_context;
    };

    class IFrameScheduler final : public RHI::FrameScheduler
    {
    public:
        IFrameScheduler(IContext* context);
        ~IFrameScheduler();

        VkResult Init();

        void DeviceWaitIdle() override;
        void QueuePassSubmit(RHI::Pass* pass, RHI::Fence* fence) override;
        void QueueCommandsSubmit(RHI::QueueType queueType, RHI::TL::Span<RHI::CommandList*> commandLists, RHI::Fence& fence) override;
    };

} // namespace Vulkan