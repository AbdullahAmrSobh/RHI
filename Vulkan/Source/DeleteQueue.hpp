#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>

#include <vk_mem_alloc.h>

#include "Device.hpp"

namespace RHI::Vulkan
{
    class IDevice;

    class DeleteQueue
    {
        IDevice*                                         m_device;
        TL::Deque<std::pair<uint64_t, VmaAllocation>>    m_allocation;
        TL::Deque<std::pair<uint64_t, VkBuffer>>         m_buffer;
        TL::Deque<std::pair<uint64_t, VkBufferView>>     m_bufferView;
        TL::Deque<std::pair<uint64_t, VkImage>>          m_image;
        TL::Deque<std::pair<uint64_t, VkImageView>>      m_imageView;
        TL::Deque<std::pair<uint64_t, VkSampler>>        m_sampler;
        TL::Deque<std::pair<uint64_t, VkPipeline>>       m_pipeline;
        TL::Deque<std::pair<uint64_t, VkDescriptorPool>> m_descriptorPool;

    public:
        ~DeleteQueue()
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

        TL_MAYBE_UNUSED ResultCode Init(IDevice* device)
        {
            m_device = device;
            return ResultCode::Success;
        }

        void Shutdown() { Flush(UINT64_MAX); }

        void Push(uint64_t timeline, VmaAllocation h) { m_allocation.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkBuffer h) { m_buffer.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkBufferView h) { m_bufferView.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkImage h) { m_image.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkImageView h) { m_imageView.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkSampler h) { m_sampler.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkPipeline h) { m_pipeline.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkDescriptorPool h) { m_descriptorPool.emplace_back(timeline, h); }

        void Flush(uint64_t timeline)
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

    private:
        template<typename VkHandleType>
        void FlushQueue(IDevice& device, TL::Deque<std::pair<uint64_t, VkHandleType>>& queue, uint64_t timeline)
        {
            while (queue.empty() == false)
            {
                auto [currentTimeline, handle] = queue.front();
                if (currentTimeline > timeline)
                    return;
                DestroyObject(device, handle);
                queue.pop_front();
            }
        }

        template<typename VkHandleType>
        static void DestroyObject(IDevice& device, VkHandleType handle)
        {
            // clang-format off
            if      constexpr (std::is_same_v<VkHandleType, VmaAllocation>)    vmaFreeMemory(device.m_deviceAllocator, handle);
            else if constexpr (std::is_same_v<VkHandleType, VkBuffer>)         vkDestroyBuffer(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkBufferView>)     vkDestroyBufferView(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkImage>)          vkDestroyImage(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkImageView>)      vkDestroyImageView(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkSampler>)        vkDestroySampler(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkPipeline>)       vkDestroyPipeline(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkDescriptorPool>) vkDestroyDescriptorPool(device.m_device, handle, nullptr);
            // clang-format on
        }
    };
} // namespace RHI::Vulkan