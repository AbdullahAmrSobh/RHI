#include "CommandPool.hpp"

#include <vulkan/vulkan.h>

#include "CommandList.hpp"
#include "Device.hpp"

namespace RHI::Vulkan
{
    CommandAllocator::CommandAllocator()  = default;
    CommandAllocator::~CommandAllocator() = default;

    ResultCode CommandAllocator::Init(IDevice* device)
    {
        m_device = device;

        // Lock mutex while initializing pools
        std::lock_guard<std::mutex> lock(m_poolMutex);

        // Initialize command pools for each queue type per thread
        auto& poolMap = m_pools[std::this_thread::get_id()];

        for (size_t i = 0; i < (int)QueueType::Count; ++i)
        {
            poolMap[i] = CreateCommandPool(static_cast<QueueType>(i));
        }

        return ResultCode::Success;
    }

    void CommandAllocator::Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        DestroyCommandPools();
    }

    ICommandList* CommandAllocator::AllocateCommandList(QueueType queueType, TL::Arena& arena)
    {
        // Lock mutex for thread-safe pool access
        std::lock_guard<std::mutex> lock(m_poolMutex);

        // Get command pool for this queue type and create a command buffer
        VkCommandPool pool = m_pools[std::this_thread::get_id()][(uint32_t)queueType];

        VkCommandBufferAllocateInfo allocateInfo = {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = pool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkResult        result        = vkAllocateCommandBuffers(m_device->m_device, &allocateInfo, &commandBuffer);

        if (result != VK_SUCCESS)
        {
            TL_UNREACHABLE_MSG("Failed to allocate Vulkan command buffer");
            return nullptr;
        }

        return arena.Construct<ICommandList>(m_device, commandBuffer);
    }

    void CommandAllocator::Reset()
    {
        // Lock mutex for thread-safe reset
        std::lock_guard<std::mutex> lock(m_poolMutex);

        // Reset command pools for the current thread
        auto& poolMap = m_pools[std::this_thread::get_id()];

        for (size_t i = 0; i < (int)QueueType::Count; ++i)
        {
            VkCommandPool pool = poolMap[i];
            vkResetCommandPool(m_device->m_device, pool, 0);
        }
    }

    VkCommandPool CommandAllocator::CreateCommandPool(QueueType queueType)
    {
        VkCommandPoolCreateInfo poolCreateInfo = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = m_device->m_queue[(uint32_t)queueType].GetFamilyIndex(),
        };

        VkCommandPool pool;
        VkResult      result = vkCreateCommandPool(m_device->m_device, &poolCreateInfo, nullptr, &pool);

        if (result != VK_SUCCESS)
        {
            // Handle error
            TL_UNREACHABLE_MSG("Failed to create Vulkan command pool");
        }

        return pool;
    }

    void CommandAllocator::DestroyCommandPools()
    {
        // Destroy command pools for the current thread
        auto& poolMap = m_pools[std::this_thread::get_id()];

        for (size_t i = 0; i < (int)QueueType::Count; ++i)
        {
            VkCommandPool pool = poolMap[i];
            vkDestroyCommandPool(m_device->m_device, pool, nullptr);
        }

        m_pools.erase(std::this_thread::get_id()); // Clean up the pools for this thread
    }
} // namespace RHI::Vulkan
