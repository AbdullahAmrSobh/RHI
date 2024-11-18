#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>

#include "Common.hpp"

namespace RHI::Vulkan
{
    class IDevice;

    class DeleteQueue
    {
    public:
        using DeleteFunc = std::function<void(IDevice* device)>;

        DeleteQueue();
        ~DeleteQueue();

        ResultCode Init(IDevice* device);
        void       Shutdown();

        // Enqueue a resource destruction lambda with a frame index
        void Push(uint64_t frameIndex, DeleteFunc&& deleteFunc);

        // Executes and clears destruction functions when frameIndex conditions are met
        void DestroyObjects(bool force);

    private:
        struct PendingDeletion
        {
            uint64_t   frameIndex;
            DeleteFunc deleteFunc;
        };

        IDevice*                     m_device;
        std::vector<PendingDeletion> m_destructionQueue;
    };
} // namespace RHI::Vulkan