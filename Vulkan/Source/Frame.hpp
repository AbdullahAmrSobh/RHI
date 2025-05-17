#pragma once

#include "Device.hpp"
#include "CommandList.hpp"
#include "Resources.hpp"

#include <TL/UniquePtr.hpp>
#include <TL/Allocator/Arena.hpp>

namespace RHI::Vulkan
{
    class CommandPool;
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

        void         Begin(TL::Span<Swapchain* const> swapchains) override;
        uint64_t     End() override;
        CommandList* CreateCommandList(const CommandListCreateInfo& createInfo) override;
        uint64_t     QueueSubmit(const QueueSubmitInfo& submitInfo) override;
        void         BufferWrite(Handle<Buffer> buffer, size_t offset, TL::Block block) override;
        void         ImageWrite(Handle<Image> image, ImageOffset3D offset, ImageSize3D size, uint32_t mipLevel, uint32_t arrayLayer, TL::Block block) override;

        IDevice*               m_device;
        TL::Arena              m_tempAllocator;
        TL::Ptr<CommandPool>   m_commandListAllocator;
        TL::Ptr<StagingBuffer> m_stagingPool;
        std::atomic_uint64_t   m_timeline;
        ICommandList*          m_activeTransferCommandList;

        TL::Vector<ISwapchain*> m_swapchains;
    };

    ////////////////////////////////////////////////////////////////
    /// Staging buffer allocator
    ////////////////////////////////////////////////////////////////

    struct StagingBufferBlock
    {
        Handle<Buffer>  buffer;
        BufferSubregion subregion;
    };

    class StagingBuffer
    {
    public:
        StagingBuffer();
        ~StagingBuffer();

        ResultCode Init(IDevice* device);
        void       Shutdown();

        StagingBufferBlock Allocate(size_t size);
        StagingBufferBlock Allocate(TL::Block block);

        void Reset();

    private:
        struct Page
        {
            DeviceMemoryPtr ptr;
            Handle<Buffer>  buffer;
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

    public:
        ~DeleteQueue();

        TL_MAYBE_UNUSED ResultCode Init(IDevice* device);

        void Shutdown();

        void Push(uint64_t timeline, VmaAllocation h);
        void Push(uint64_t timeline, VkBuffer h);
        void Push(uint64_t timeline, VkBufferView h);
        void Push(uint64_t timeline, VkImage h);
        void Push(uint64_t timeline, VkImageView h);
        void Push(uint64_t timeline, VkSampler h);
        void Push(uint64_t timeline, VkPipeline h);
        void Push(uint64_t timeline, VkDescriptorPool h);
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