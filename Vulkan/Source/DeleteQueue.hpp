#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>

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
        void DestroyObjects();

    private:
        struct PendingDeletion
        {
            uint64_t   frameIndex;
            DeleteFunc deleteFunc;
        };

        IDevice*                    m_device;
        TL::Vector<PendingDeletion> m_destructionQueue;
        uint64_t                    m_completedFrameIndex = 0;
        uint32_t                    m_readIndex           = 0; // Head (read) index
        uint32_t                    m_writeIndex          = 0; // Tail (write) index
    };
} // namespace RHI::Vulkan