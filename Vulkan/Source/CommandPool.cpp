#include "CommandPool.hpp"
#include "Context.hpp"
#include "Common.hpp"
#include "CommandList.hpp"

#include <RHI/Format.hpp>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{

    VkCommandPoolCreateFlags ConvertCommandPoolFlags(TL::Flags<CommandPoolFlags> flags)
    {
        VkCommandPoolCreateFlags result{};
        if (flags & CommandPoolFlags::Transient)
            result |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        if (flags & CommandPoolFlags::Reset)
            result |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        return result;
    }

    ICommandPool::ICommandPool(IContext* context)
        : m_context(context)
    {
    }

    ICommandPool::~ICommandPool()
    {
        for (auto commandPool : m_commandPools)
        {
            vkDestroyCommandPool(m_context->m_device, commandPool, nullptr);
        }
    }

    ResultCode ICommandPool::Init(CommandPoolFlags flags)
    {
        for (uint32_t queueType = 0; queueType < uint32_t(QueueType::Count); queueType++)
        {
            VkCommandPoolCreateInfo createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = ConvertCommandPoolFlags(flags);
            createInfo.queueFamilyIndex = m_context->m_queue[queueType].GetFamilyIndex();
            TryValidateVk(vkCreateCommandPool(m_context->m_device, &createInfo, nullptr, &m_commandPools[queueType]));
        }

        return ResultCode::Success;
    }

    void ICommandPool::Reset()
    {
        for (auto commandPool : m_commandPools)
        {
            vkResetCommandPool(m_context->m_device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        }
    }

    TL::Vector<CommandList*> ICommandPool::Allocate(QueueType queueType, CommandListLevel level, uint32_t count)
    {
        auto commandPool = m_commandPools[uint32_t(queueType)];
        auto commandBuffers = AllocateCommandBuffers(commandPool, count, level == CommandListLevel::Primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY);
        TL::Vector<CommandList*> commandLists;
        for (auto commandBuffer : commandBuffers)
        {
            commandLists.push_back(new ICommandList(m_context, commandBuffer));
        }
        return commandLists;
    }

    TL::Vector<VkCommandBuffer> ICommandPool::AllocateCommandBuffers(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level)
    {
        TL::Vector<VkCommandBuffer> commandBuffers;
        commandBuffers.resize(count);

        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.commandPool = pool;
        allocateInfo.level = level;
        allocateInfo.commandBufferCount = count;
        vkAllocateCommandBuffers(m_context->m_device, &allocateInfo, commandBuffers.data());
        return commandBuffers;
    }
} // namespace RHI::Vulkan
