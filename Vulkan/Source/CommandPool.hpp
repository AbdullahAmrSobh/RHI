#pragma once

#include <RHI/Queue.hpp>

#include <TL/Flags.hpp>
#include <TL/UniquePtr.hpp>

#include <vulkan/vulkan.h>

#include <mutex>
#include <thread>
#include <unordered_map>
#include <array>

namespace RHI::Vulkan
{
    class IDevice;
    class ICommandList;

    class CommandAllocator
    {
    public:
        CommandAllocator() = default;
        ~CommandAllocator();

        void Init(IDevice* device);
        void Shutdown();

        /// @brief Allocates a command list for the specified queue type.
        /// @param queueType The type of queue (Graphics, Compute, Transfer) for the command list.
        /// @return A pointer to the allocated CommandList.
        TL::Ptr<ICommandList> AllocateCommandList(QueueType queueType);

        /// @brief Resets all command lists used within the current frame.
        void Reset();

    private:
        using CommandPoolPerQueue = std::array<VkCommandPool, (int)QueueType::Count>;

        IDevice* m_device = nullptr;

        std::mutex m_poolMutex;

        std::unordered_map<std::thread::id, CommandPoolPerQueue> m_pools;

        // Helper functions to create and destroy command pools
        VkCommandPool CreateCommandPool(QueueType queueType);
        void          DestroyCommandPools();
    };

} // namespace RHI::Vulkan
