#pragma once

#include <RHI/CommandPool.hpp>
#include <RHI/Result.hpp>

#include <TL/Flags.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

    VkCommandPoolCreateFlags ConvertCommandPoolFlags(TL::Flags<CommandPoolFlags> flags);

    class ICommandPool final : public CommandPool
    {
    public:
        ICommandPool(IContext* context);
        ~ICommandPool();

        ResultCode Init(CommandPoolFlags flags);

        void Reset() override;
        TL::Vector<CommandList*> Allocate(QueueType queueType, CommandListLevel level, uint32_t count) override;

    private:
        TL::Vector<VkCommandBuffer> AllocateCommandBuffers(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level);

    private:
        IContext* m_context;
        VkCommandPool m_commandPools[uint32_t(QueueType::Count)];
    };

} // namespace RHI::Vulkan