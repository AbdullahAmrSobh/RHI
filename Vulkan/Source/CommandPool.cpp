#include "CommandList.hpp"
#include "CommandPool.hpp"
#include "Context.hpp"

namespace RHI::Vulkan
{
    class IContext;
    class ICommandList;

    ICommandListAllocator::ICommandListAllocator(IContext* context)
        : m_context(context)
    {
    }

    ICommandListAllocator::~ICommandListAllocator()
    {
        for (auto queueCommandPool : m_commandPools)
        {
            for (auto commandPool : queueCommandPool)
            {
                vkDestroyCommandPool(m_context->m_device, commandPool, nullptr);
            }
        }
    }

    VkResult ICommandListAllocator::Init()
    {
        for (uint32_t queueType = 0; queueType < uint32_t(QueueType::Count); queueType++)
        {
            m_commandPools[(queueType)].resize(2);
            for (auto& commandPool : m_commandPools[(queueType)])
            {
                VkCommandPoolCreateInfo createInfo;
                createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                createInfo.pNext = nullptr;
                createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                createInfo.queueFamilyIndex = m_context->GetQueueFamilyIndex(QueueType(queueType));
                auto result = vkCreateCommandPool(m_context->m_device, &createInfo, nullptr, &commandPool);
                RHI_ASSERT(result == VK_SUCCESS);
                if (result != VK_SUCCESS)
                {
                    return result;
                }
            }
        }

        return VK_SUCCESS;
    }

    void ICommandListAllocator::Reset()
    {
        for (auto queueCommandPool : m_commandPools)
        {
            for (auto commandPool : queueCommandPool)
            {
                vkTrimCommandPool(m_context->m_device, commandPool, 0);
                vkResetCommandPool(m_context->m_device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
            }
        }
    }

    CommandList* ICommandListAllocator::Allocate(QueueType queueType)
    {
        return Allocate(queueType, 1).front();
    }

    TL::Vector<CommandList*> ICommandListAllocator::Allocate(QueueType queueType, uint32_t count)
    {
        auto commandPool = m_commandPools[uint32_t(queueType)][0];
        auto commandBuffers = AllocateCommandBuffers(commandPool, count, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        TL::Vector<CommandList*> commandLists;
        commandLists.reserve(count);
        for (auto commandBuffer : commandBuffers)
        {
            commandLists.push_back(new ICommandList(m_context, commandPool, commandBuffer));
        }
        return commandLists;
    }

    void ICommandListAllocator::Release(TL::Span<CommandList*> _commandLists)
    {
        (void)_commandLists;
        // auto commandLists = TL::Span((ICommandList*)_commandLists.data(), _commandLists.size());
        // TL::Vector<VkCommandBuffer> commandBuffers;
        // commandBuffers.reserve(_commandLists.size());
        // auto commandPool = commandLists[0].m_commandPool;
        // auto device = m_context->m_device;

        // for (auto& commandList : commandLists)
        // {
        //     commandBuffers.push_back(commandList.m_commandBuffer);
        // }

        // m_context->m_deferDeleteQueue.push_back([=](){
        //     vkFreeCommandBuffers(device, commandPool, uint32_t(commandBuffers.size()), commandBuffers.data());
        // });
    }

    TL::Vector<VkCommandBuffer> ICommandListAllocator::AllocateCommandBuffers(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level)
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

    void ICommandListAllocator::ReleaseCommandBuffers(VkCommandPool pool, TL::Span<VkCommandBuffer> commandBuffers)
    {
        auto device = m_context->m_device;
        m_context->m_deferDeleteQueue.push_back([=]()
        {
            vkFreeCommandBuffers(device, pool, uint32_t(commandBuffers.size()), commandBuffers.data());
        });
    }

} // namespace RHI::Vulkan