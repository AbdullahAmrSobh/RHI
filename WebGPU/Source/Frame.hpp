#pragma once

#include "Device.hpp"
#include "CommandList.hpp"
#include "Resources.hpp"

#include <TL/Ptr.hpp>
#include <TL/Literals.hpp>

namespace RHI::WebGPU
{
    class ICommandList;
    class ISwapchain;

    ////////////////////////////////////////////////////////////////
    /// Frame
    ////////////////////////////////////////////////////////////////

    class IFrame final : public Frame
    {
    public:
        IFrame() = default;
        ~IFrame() = default;

        ResultCode Init(IDevice* device);
        void       Shutdown();

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
        IDevice* m_device;

        std::atomic_uint64_t m_timeline;

        // transient states
        TL::Arena              m_arena;                       ///< Arena allocator used for per-frame allocations.
        TL::Vector<Swapchain*> m_acquiredSwapchains{m_arena}; ///< List of swapchains acquired by this frame.
    };
} // namespace RHI::WebGPU

