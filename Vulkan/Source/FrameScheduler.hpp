#pragma once

#include <RHI/FrameScheduler.hpp>

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    struct Allocation;

    struct IFence;

    class IContext;
    class ICommandList;

    class IPass final : public Pass
    {
        friend class ICommandList;
        friend class IFrameScheduler;

    public:
        IPass(IContext* context, const char* name, QueueType queueType);
        ~IPass();

        VkResult Init();

        IContext* m_context;
    };

    class IFrameScheduler final : public FrameScheduler
    {
    public:
        IFrameScheduler(IContext* context);
        ~IFrameScheduler();

        VkResult Init();

        void DeviceWaitIdle() override;
        void QueuePassSubmit(Pass* pass, Fence* fence) override;
        void QueueCommandsSubmit(QueueType queueType, TL::Span<CommandList*> commandLists, Fence& fence) override;
        void StreamResource(Handle<Image> dst, ImageOffset offset, ImageSize3D size, StagingBuffer::TempBuffer& tempBuffer) override;
        void StreamResource(Handle<Buffer> dst, size_t offset, size_t size, StagingBuffer::TempBuffer& tempBuffer) override;
    };

} // namespace RHI::Vulkan