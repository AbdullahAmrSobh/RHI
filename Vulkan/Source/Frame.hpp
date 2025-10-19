#pragma once

#include "Device.hpp"
#include "CommandList.hpp"
#include "Resources.hpp"

#include <TL/Ptr.hpp>
#include <TL/Literals.hpp>

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

    struct StagingBufferBlock
    {
        IBuffer*        buffer;
        BufferSubregion subregion;
    };

    class StagingBuffer
    {
    public:
        static constexpr size_t MinPageSize = 64_mb;

        StagingBuffer()  = default;
        ~StagingBuffer() = default;

        ResultCode         init(IDevice* device);
        void               shutdown(IDevice* device);
        StagingBufferBlock allocate(IDevice* device, size_t size);
        StagingBufferBlock allocate(IDevice* device, TL::Block block);
        void               reset(IDevice* device);

    private:
        struct Page
        {
            IBuffer*        buffer    = nullptr;
            DeviceMemoryPtr mappedPtr = nullptr;
            const size_t    size      = 0;
            size_t          offset    = 0;
            bool            isMapped  = false;

            size_t getRemainingSize() const { return size - offset; }
        };

        TL::Vector<Page> m_pages;
    };

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
        void            DestroyCommandList(CommandList* commandList) override;
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
} // namespace RHI::Vulkan