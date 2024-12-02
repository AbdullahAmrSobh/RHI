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
        uint64_t currentTimelineValue = m_device->m_frameIndex;
        auto     objectIt             = m_destructionQueue.begin();
        for (; objectIt != m_destructionQueue.end(); objectIt++)
        {
            if (objectIt->frameIndex < currentTimelineValue || force)
            {
                objectIt->deleteFunc(m_device);
            }
            else
            {
                break;
            }
        }
        m_destructionQueue.erase(m_destructionQueue.begin(), objectIt);
    }
} // namespace RHI::Vulkan