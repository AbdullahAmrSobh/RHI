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
        vkDeviceWaitIdle(m_device->m_device);
        for (auto& [frameIndex, deleteFunc] : m_destructionQueue)
        {
            deleteFunc(m_device);
        }
        m_destructionQueue.clear();

        // auto frameIndex = m_device->m_frameIndex.load();
        // // To destroy an object swap with last element in the queue
        // // increase delete count
        // static uint32_t startIndex = 0;
        // uint64_t deleteCount = 0;

        // auto it = m_destructionQueue.begin() + startIndex;
        // for ( ;it != m_destructionQueue.end(); it++)
        // {
        //     if (it->frameIndex < frameIndex)
        //     {
        //         it->deleteFunc(m_device);
        //         std::swap(*it, m_destructionQueue.back());
        //         deleteCount++;
        //     }
        //     else
        //     {
        //         break;
        //     }
        // }

        // // Remove remaining elements
        // m_destructionQueue.erase(it, it + deleteCount);
    }
} // namespace RHI::Vulkan