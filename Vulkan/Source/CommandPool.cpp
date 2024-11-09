
#include "CommandPool.hpp"
#include "Device.hpp"
#include "Common.hpp"
#include "CommandList.hpp"

#include <RHI/Format.hpp>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{

    VkCommandPoolCreateFlags ConvertCommandPoolFlags(TL::Flags<CommandPoolFlags> flags)
    {
        VkCommandPoolCreateFlags result{};
        if (flags & CommandPoolFlags::Transient) result |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        if (flags & CommandPoolFlags::Reset) result |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        return result;
    }

    ICommandPool::ICommandPool(IDevice* device)
        : m_device(device)
    {
    }

    ICommandPool::~ICommandPool()
    {
        for (auto commandPool : m_commandPools)
        {
            // vkDestroyCommandPool(m_device->m_device, commandPool, nullptr);
            m_device->m_deleteQueue.DestroyObject(commandPool);
        }
    }

    ResultCode ICommandPool::Init(CommandPoolFlags flags)
    {
        for (uint32_t queueType = 0; queueType < uint32_t(QueueType::Count); queueType++)
        {
            VkCommandPoolCreateInfo createInfo{
                .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext            = nullptr,
                .flags            = ConvertCommandPoolFlags(flags),
                .queueFamilyIndex = m_device->m_queue[queueType].GetFamilyIndex(),
            };
            TRY_OR_RETURN(vkCreateCommandPool(m_device->m_device, &createInfo, nullptr, &m_commandPools[queueType]));
        }

        return ResultCode::Success;
    }

    void ICommandPool::Reset()
    {
        for (auto commandPool : m_commandPools)
        {
            vkResetCommandPool(m_device->m_device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        }
    }

    TL::Vector<TL::Ptr<CommandList>> ICommandPool::Allocate(QueueType queueType, CommandListLevel level, uint32_t count)
    {
        auto commandPool    = m_commandPools[uint32_t(queueType)];
        auto commandBuffers = AllocateCommandBuffers(
            commandPool, count, level == CommandListLevel::Primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY);
        TL::Vector<TL::Ptr<CommandList>> commandLists;
        for (auto commandBuffer : commandBuffers)
        {
            commandLists.push_back(TL::CreatePtr<ICommandList>(m_device, commandBuffer));
        }
        return commandLists;
    }

    TL::Vector<VkCommandBuffer> ICommandPool::AllocateCommandBuffers(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level)
    {
        TL::Vector<VkCommandBuffer> commandBuffers;
        commandBuffers.resize(count);

        VkCommandBufferAllocateInfo allocateInfo{
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext              = nullptr,
            .commandPool        = pool,
            .level              = level,
            .commandBufferCount = count,
        };
        vkAllocateCommandBuffers(m_device->m_device, &allocateInfo, commandBuffers.data());
        return commandBuffers;
    }
} // namespace RHI::Vulkan
