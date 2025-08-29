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
            DeviceMemoryPtr ptr;
            Buffer*         buffer;
            size_t          offset;
            const size_t    size;

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
        IDevice*                                         m_device;
        TL::Deque<std::pair<uint64_t, VmaAllocation>>    m_allocation;
        TL::Deque<std::pair<uint64_t, VkBuffer>>         m_buffer;
        TL::Deque<std::pair<uint64_t, VkBufferView>>     m_bufferView;
        TL::Deque<std::pair<uint64_t, VkImage>>          m_image;
        TL::Deque<std::pair<uint64_t, VkImageView>>      m_imageView;
        TL::Deque<std::pair<uint64_t, VkSampler>>        m_sampler;
        TL::Deque<std::pair<uint64_t, VkPipeline>>       m_pipeline;
        TL::Deque<std::pair<uint64_t, VkDescriptorPool>> m_descriptorPool;
        TL::Deque<std::pair<uint64_t, VkSwapchainKHR>>   m_swapchain;
        TL::Deque<std::pair<uint64_t, VkSurfaceKHR>>     m_surface;
        TL::Deque<std::pair<uint64_t, VkSemaphore>>      m_semaphore;

    public:
        ~DeleteQueue();

        void Init(IDevice* device);

        void Shutdown();

        void Push(uint64_t timeline, VmaAllocation h) { m_allocation.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkBuffer h) { m_buffer.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkBufferView h) { m_bufferView.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkImage h) { m_image.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkImageView h) { m_imageView.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkSampler h) { m_sampler.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkPipeline h) { m_pipeline.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkDescriptorPool h) { m_descriptorPool.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkSwapchainKHR h) { m_swapchain.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkSurfaceKHR h) { m_surface.emplace_back(timeline, h); }

        void Push(uint64_t timeline, VkSemaphore h) { m_semaphore.emplace_back(timeline, h); }

        void Flush(uint64_t timeline);

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
        }
    };
} // namespace RHI::Vulkan