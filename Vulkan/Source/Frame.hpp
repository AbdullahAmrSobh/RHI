#pragma once

#include "Device.hpp"
#include "CommandList.hpp"
#include "Resources.hpp"

#include <TL/UniquePtr.hpp>
#include <TL/Allocator/Arena.hpp>

#include <tracy/TracyVulkan.hpp>

namespace RHI::Vulkan
{
    class CommandAllocator;
    class ICommandList;
    class StagingBuffer;
    class ISwapchain;

    ////////////////////////////////////////////////////////////////
    /// Frame
    ////////////////////////////////////////////////////////////////

    struct StagingBufferBlock;

    class IFrame final : public Frame
    {
    public:
        IFrame() = default;

        ~IFrame()
        {
            Shutdown();
        }

        ResultCode Init(IDevice* device);
        void       Shutdown();

        ICommandList*      GetActiveTransferCommandList();
        StagingBufferBlock AllocateStaging(TL::Block block);

        void            CaptureNextFrame() override;
        void            Begin(TL::Span<SwapchainImageAcquireInfo> swapchainToAcquire) override;
        void            End() override;
        CommandList*    CreateCommandList(const CommandListCreateInfo& createInfo) override;
        uint64_t        QueueSubmit(QueueType queueType, const QueueSubmitInfo& submitInfo) override;
        void            BufferWrite(Buffer* buffer, size_t offset, TL::Block block) override;
        void            ImageWrite(Image* image, ImageOffset3D offset, ImageSize3D size, uint32_t mipLevel, uint32_t arrayLayer, TL::Block block) override;
        TL::IAllocator& GetAllocator() override;
        uint64_t        GetTimelineValue() const;

    private:
        IDevice*                  m_device;
        TL::Ptr<StagingBuffer>    m_stagingPool;
        TL::Ptr<CommandAllocator> m_commandListAllocator;

        std::atomic_uint64_t m_timeline;

        ICommandList* m_activeTransferCommandList;

        VkSemaphore m_presentFrameSemaphore;

        // transient states
        TL::Arena              m_arena;                       ///< Arena allocator used for per-frame allocations.
        TL::Vector<Swapchain*> m_acquiredSwapchains{m_arena}; ///< List of swapchains acquired by this frame.
        // TL::Vector<struct BufferStreamRequest> m_bufferStreamRequests{m_arena}; ///<
        // TL::Vector<struct BufferStreamRequest> m_imageStreamRequests{m_arena};  ///<

        bool m_renderdocPendingCapture : 1;
    };

    ////////////////////////////////////////////////////////////////
    /// Staging buffer allocator
    ////////////////////////////////////////////////////////////////

    struct StagingBufferBlock
    {
        Buffer*         buffer;
        BufferSubregion subregion;
    };

    class StagingBuffer
    {
    public:
        StagingBuffer();
        ~StagingBuffer();

        ResultCode         Init(IDevice* device);
        void               Shutdown();
        StagingBufferBlock Allocate(size_t size);
        StagingBufferBlock Allocate(TL::Block block);
        void               Reset();

    private:
        struct Page
        {
            Buffer*      buffer;
            size_t       offset;
            const size_t size;

            size_t GetRemainingSize() const { return size - offset; }
        };

        IDevice*         m_device;
        TL::Vector<Page> m_pages;
    };

    ////////////////////////////////////////////////////////////////
    /// Release Queue
    ////////////////////////////////////////////////////////////////

    class DeleteQueue
    {
    public:
        using VmaImageAllocation  = std::pair<VkImage, VmaAllocation>;
        using VmaBufferAllocation = std::pair<VkBuffer, VmaAllocation>;

        ~DeleteQueue();

        void Init(IDevice* device);

        void Shutdown();

        void Push(uint64_t timeline, VmaAllocation h) { PushImpl(m_allocation, timeline, h); }

        void Push(uint64_t timeline, VkBuffer h) { PushImpl(m_buffer, timeline, h); }

        // void Push(uint64_t timeline, VmaBufferAllocation h) { PushImpl(m_vmaBuffer, timeline, h); }

        void Push(uint64_t timeline, VkBufferView h) { PushImpl(m_bufferView, timeline, h); }

        void Push(uint64_t timeline, VkImage h) { PushImpl(m_image, timeline, h); }

        // void Push(uint64_t timeline, VmaImageAllocation h) { PushImpl(m_vmaImage, timeline, h); }

        void Push(uint64_t timeline, VkImageView h) { PushImpl(m_imageView, timeline, h); }

        void Push(uint64_t timeline, VkSampler h) { PushImpl(m_sampler, timeline, h); }

        void Push(uint64_t timeline, VkPipeline h) { PushImpl(m_pipeline, timeline, h); }

        void Push(uint64_t timeline, VkDescriptorPool h) { PushImpl(m_descriptorPool, timeline, h); }

        void Push(uint64_t timeline, VkSwapchainKHR h) { PushImpl(m_swapchain, timeline, h); }

        void Push(uint64_t timeline, VkSurfaceKHR h) { PushImpl(m_surface, timeline, h); }

        void Push(uint64_t timeline, VkSemaphore h) { PushImpl(m_semaphore, timeline, h); }

        void Flush(uint64_t timeline);

    private:
        template<typename VkHandleType>
        void PushImpl(TL::Vector<std::pair<uint64_t, VkHandleType>>& queue, uint64_t timeline, VkHandleType h)
        {
            if (auto it = m_pending.find((uint64_t)h); it != m_pending.end())
            {
                auto st = TL::ReportStacktrace(it->second);
                TL_LOG_ERROR("Object was already requested for deletion at {}", st);
                TL_UNREACHABLE();
            }
            else
            {
                m_pending[(uint64_t)h] = TL::CaptureStacktrace();
            }
            queue.emplace_back(timeline, h);
        }

        // template<>
        // void PushImpl(TL::Vector<std::pair<uint64_t, VmaBufferAllocation>>& queue, uint64_t timeline, VmaBufferAllocation h)
        // {
        //     queue.emplace_back(timeline, h);
        // }

        // template<>
        // void PushImpl(TL::Vector<std::pair<uint64_t, VmaImageAllocation>>& queue, uint64_t timeline, VmaImageAllocation h)
        // {
        //     queue.emplace_back(timeline, h);
        // }

        template<typename VkHandleType>
        void FlushQueue(IDevice& device, TL::Vector<std::pair<uint64_t, VkHandleType>>& queue, uint64_t timeline)
        {
            uint32_t deleteCount = 0;
            for (auto [currentTimeline, handle] : queue)
            {
                if (currentTimeline > timeline)
                    break;
                DestroyObject(device, handle);
                TL_ASSERT(m_pending.erase((uint64_t)handle));
                deleteCount++;
            }
            queue.erase(queue.begin(), queue.begin() + deleteCount);
        }

        template<typename VkHandleType>
        static constexpr void DestroyObject(IDevice& device, VkHandleType handle)
        {
            if constexpr (std::is_same_v<VkHandleType, VmaAllocation>) vmaFreeMemory(device.m_deviceAllocator, handle);
            else if constexpr (std::is_same_v<VkHandleType, VkBuffer>) vkDestroyBuffer(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkBufferView>) vkDestroyBufferView(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkImage>) vkDestroyImage(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkImageView>) vkDestroyImageView(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkSampler>) vkDestroySampler(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkPipeline>) vkDestroyPipeline(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkDescriptorPool>) vkDestroyDescriptorPool(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkSemaphore>) vkDestroySemaphore(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkSwapchainKHR>) vkDestroySwapchainKHR(device.m_device, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VkSurfaceKHR>) vkDestroySurfaceKHR(device.m_instance, handle, nullptr);
            else if constexpr (std::is_same_v<VkHandleType, VmaBufferAllocation>)
            {
                vmaDestroyBuffer(device.m_deviceAllocator, handle.first, handle.second);
            }
            else if constexpr (std::is_same_v<VkHandleType, VmaBufferAllocation>)
            {
                vmaDestroyImage(device.m_deviceAllocator, handle.first, handle.second);
            }
        }

    private:
        IDevice*                                          m_device;
        TL::Vector<std::pair<uint64_t, VmaAllocation>>    m_allocation;
        TL::Vector<std::pair<uint64_t, VkBuffer>>         m_buffer;
        TL::Vector<std::pair<uint64_t, VkBufferView>>     m_bufferView;
        TL::Vector<std::pair<uint64_t, VkImage>>          m_image;
        TL::Vector<std::pair<uint64_t, VkImageView>>      m_imageView;
        TL::Vector<std::pair<uint64_t, VkSampler>>        m_sampler;
        TL::Vector<std::pair<uint64_t, VkPipeline>>       m_pipeline;
        TL::Vector<std::pair<uint64_t, VkDescriptorPool>> m_descriptorPool;
        TL::Vector<std::pair<uint64_t, VkSwapchainKHR>>   m_swapchain;
        TL::Vector<std::pair<uint64_t, VkSurfaceKHR>>     m_surface;
        TL::Vector<std::pair<uint64_t, VkSemaphore>>      m_semaphore;

        TL::Vector<std::pair<uint64_t, VmaBufferAllocation>> m_vmaBuffer;
        TL::Vector<std::pair<uint64_t, VmaImageAllocation>>  m_vmaImage;

        TL::Map<uint64_t, TL::Stacktrace> m_pending;
    };
} // namespace RHI::Vulkan