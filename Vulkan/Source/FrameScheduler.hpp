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
        void Activate(RHI::Image* resource) override;
        void Deactivate(RHI::Image* resource) override;
        void Activate(RHI::Buffer* resource) override;
        void Deactivate(RHI::Buffer* resource) override;
        RHI::Handle<RHI::Image> CreateTransientImage(const RHI::ImageCreateInfo& createInfo) override;
        RHI::Handle<RHI::Buffer> CreateTransientBuffer(const RHI::BufferCreateInfo& createInfo) override;

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

        RHI::CommandList& BeginCommandList(uint32_t commandsCount) override;
        void EndCommandList() override;

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

        std::vector<VkSemaphoreSubmitInfo> GetSemaphores(const std::vector<RHI::Pass*>& passes) const;

        std::unique_ptr<RHI::Pass> CreatePass(const RHI::PassCreateInfo& createInfo) override;
        void ExecutePass(RHI::Pass& pass) override;
        void ResetPass(RHI::Pass& pass) override;
        RHI::CommandList* GetCommandList(uint32_t frameIndex) override;
        void OnFrameEnd() override;

        uint64_t m_frameNumber;

        std::unique_ptr<CommandListAllocator> m_graphicsCommandlistAllocator;

        std::vector<VkFence> m_framesInflightFences;
    };

} // namespace Vulkan