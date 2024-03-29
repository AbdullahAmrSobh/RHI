#pragma once

#include <RHI/CommandList.hpp>
#include <RHI/Pass.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

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

} // namespace RHI::Vulkan