#include "DeleteQueue.hpp"

#include <TL/Assert.hpp>

#include <tracy/Tracy.hpp>

#include "Device.hpp"

namespace RHI::Vulkan
{

    DeleteQueue::DeleteQueue()  = default;
    DeleteQueue::~DeleteQueue() = default;

    ResultCode DeleteQueue::Init(IDevice* device)
    {
        m_device = device;
        return ResultCode::Success;
    }

    void DeleteQueue::Shutdown()
    {
        vkDeviceWaitIdle(m_device->m_device);
        DestroyObjects(true);
        TL_ASSERT(m_destructionQueue.empty());
    }

    void DeleteQueue::Push(uint64_t frameIndex, DeleteFunc&& deleteFunc)
    {
        m_destructionQueue.push_back({frameIndex, std::move(deleteFunc)});
    }

    void DeleteQueue::DestroyObjects(bool force)
    {
        std::erase_if(
            m_destructionQueue,
            [&](const PendingDeletion& deletion)
            {
                if (force || m_device->m_frameIndex >= deletion.frameIndex)
                {
                    deletion.deleteFunc(m_device);
                    return true;
                }
                return false;
            });
    }
} // namespace RHI::Vulkan