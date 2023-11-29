#pragma once
#include <RHI/FrameGraph.hpp>
#include <optional>
#include <vk_mem_alloc.h>

namespace Vulkan
{
    struct Allocation;

    struct Fence;

    class Context;
    class CommandList;
    class CommandListAllocator;

    VkAttachmentLoadOp ConvertLoadOp(RHI::ImageLoadOperation op);

    VkAttachmentStoreOp ConvertStoreOp(RHI::ImageStoreOperation op);

    VkImageLayout ConvertImageLayout(RHI::AttachmentUsage usage, RHI::AttachmentAccess access);

    /// @brief Memory allocator use for allocating transient resource backing memory.
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

        RHI::CommandList& BeginCommandList(uint32_t commandsCount = 1) override;

        void EndCommandList() override;

        RHI::PassQueueState GetPassQueueStateInternal() const override;

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

        std::vector<VkSemaphoreSubmitInfo> GetSemaphores(const std::vector<RHI::Pass*>& passes) const;

        void ExecutePass(RHI::Pass& pass) override;

        void ResetPass(RHI::Pass& pass) override;

        RHI::CommandList* GetCommandList(uint32_t frameIndex) override;

        void OnFrameEnd() override;

        uint32_t m_currentFrameIndex;

        std::unique_ptr<CommandListAllocator> m_graphicsCommandlistAllocator;

        std::vector<VkFence> m_framesInflightFences;
    };

} // namespace Vulkan