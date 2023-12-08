#pragma once
#include <RHI/FrameScheduler.hpp>
#include <optional>
#include <vk_mem_alloc.h>

namespace Vulkan
{
    struct Allocation;

    struct Fence;

    class Context;
    class CommandList;
    class CommandListAllocator;

    class TransientAttachmentAllocator final : public RHI::TransientAttachmentAllocator
    {
    public:
        struct Block
        {
            VmaAllocation allocation;
            VmaAllocationInfo info;
            VmaVirtualBlock virtualBlock;
            VkMemoryHeapFlags memoryProperties;
        };

        TransientAttachmentAllocator(Context* context);
        ~TransientAttachmentAllocator();

        void Begin() override;
        void End() override;

        void Allocate(RHI::ImageAttachment* attachment) override;

        void Free(RHI::ImageAttachment* attachment) override;

        void Allocate(RHI::BufferAttachment* attachment) override;

        void Free(RHI::BufferAttachment* attachment) override;

        std::optional<Allocation> Allocate(VkMemoryRequirements requirements);

        void Free(Allocation allocation);

        size_t CalculatePreferredBlockSize(uint32_t memTypeIndex);

        Block CreateBlockNewBlock(VkMemoryRequirements minRequirements);

        Context* m_context;

        std::vector<Block> m_blocks;
    };

    class Pass final : public RHI::Pass
    {
        friend class FrameScheduler;
        friend class CommandList;

    public:
        Pass(Context* context);
        ~Pass();

        VkResult Init(const RHI::PassCreateInfo& createInfo);

        std::vector<VkSemaphoreSubmitInfo> GetWaitSemaphoreSubmitInfos() const;

        std::vector<VkSemaphoreSubmitInfo> GetSignalSemaphoreSubmitInfos() const;

        uint32_t m_queueFamilyIndex;

        VkSemaphore m_signalSemaphore;

        VkFence m_signalFence;
    };

    class FrameScheduler final : public RHI::FrameScheduler
    {
    public:
        FrameScheduler(Context* context);
        ~FrameScheduler();

        VkResult Init();

        std::unique_ptr<RHI::Pass> CreatePass(const RHI::PassCreateInfo& createInfo) override;

        bool WaitIdle(uint64_t waitTimeNano) override;

        void ExecutePass(RHI::Pass& pass) override;

        void OnFrameBegin() override;
        void OnFrameEnd() override;

        VkFence GetCurrentFrameFence() const;

        std::vector<VkFence> m_framesInflightFences;
    };

} // namespace Vulkan