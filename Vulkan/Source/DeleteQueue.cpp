#include "DeleteQueue.hpp"
#include "Device.hpp"

#include <TL/Assert.hpp>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    DeleteQueue::~DeleteQueue()
    {
        TL_ASSERT(m_allocation.empty());
        TL_ASSERT(m_buffer.empty());
        TL_ASSERT(m_bufferView.empty());
        TL_ASSERT(m_image.empty());
        TL_ASSERT(m_imageView.empty());
        TL_ASSERT(m_sampler.empty());
        TL_ASSERT(m_pipeline.empty());
        TL_ASSERT(m_descriptorPool.empty());
    }

    ResultCode DeleteQueue::Init(IDevice* device)
    {
        m_device = device;
        return ResultCode::Success;
    }

    void DeleteQueue::Shutdown()
    {
        Flush(UINT64_MAX);
    }

    void DeleteQueue::Push(uint64_t timeline, VmaAllocation h)
    {
        m_allocation.emplace_back(timeline, h);
    }

    void DeleteQueue::Push(uint64_t timeline, VkBuffer h)
    {
        m_buffer.emplace_back(timeline, h);
    }

    void DeleteQueue::Push(uint64_t timeline, VkBufferView h)
    {
        m_bufferView.emplace_back(timeline, h);
    }

    void DeleteQueue::Push(uint64_t timeline, VkImage h)
    {
        m_image.emplace_back(timeline, h);
    }

    void DeleteQueue::Push(uint64_t timeline, VkImageView h)
    {
        m_imageView.emplace_back(timeline, h);
    }

    void DeleteQueue::Push(uint64_t timeline, VkSampler h)
    {
        m_sampler.emplace_back(timeline, h);
    }

    void DeleteQueue::Push(uint64_t timeline, VkPipeline h)
    {
        m_pipeline.emplace_back(timeline, h);
    }

    void DeleteQueue::Push(uint64_t timeline, VkDescriptorPool h)
    {
        m_descriptorPool.emplace_back(timeline, h);
    }

    void DeleteQueue::Flush(uint64_t timeline)
    {
        // NOTE: Order is important
        FlushQueue(*m_device, m_bufferView, timeline);
        FlushQueue(*m_device, m_buffer, timeline);
        FlushQueue(*m_device, m_imageView, timeline);
        FlushQueue(*m_device, m_image, timeline);
        FlushQueue(*m_device, m_allocation, timeline);
        FlushQueue(*m_device, m_sampler, timeline);
        FlushQueue(*m_device, m_pipeline, timeline);
        FlushQueue(*m_device, m_descriptorPool, timeline);
    }
} // namespace RHI::Vulkan