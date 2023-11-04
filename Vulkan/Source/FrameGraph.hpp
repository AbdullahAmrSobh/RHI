#pragma once
#include <RHI/FrameGraph.hpp>

#include <vk_mem_alloc.h>

#include <optional>

namespace Vulkan
{
    struct Allocation;

    struct Fence;

    class Context;
    class CommandList;
    class Allocation;

    VkAttachmentLoadOp ConvertToVkLoadOp(RHI::ImageLoadOperation op);

    VkAttachmentStoreOp ConvertToVkStoreOp(RHI::ImageStoreOperation op);

    VkImageLayout ConvertToVkImageLayout(RHI::AttachmentUsage usage, RHI::AttachmentAccess access);

    template<typename T>
    inline static bool IsPow2(T x)
    {
        return (x & (x - 1)) == 0;
    }

    template<typename T>
    inline static T AlignUp(T val, T alignment)
    {
        RHI_ASSERT(IsPow2(alignment));
        return (val + alignment - 1) & ~(alignment - 1);
    }

    /// @brief Memory allocator use for allocating transient resource backing memory.
    class TransientResourceAllocator
    {
    public:
        TransientResourceAllocator(Context* context)
            : m_context(context)
        {
        }
        ~TransientResourceAllocator();

        /// @brief bind the given resource to a memory allocation (may alias).
        bool Activate(RHI::Handle<RHI::Image> image);

        /// @brief bind the given resource to a memory allocation (may alias).
        bool Activate(RHI::Handle<RHI::Buffer> buffer);

        /// @brief returns the memory used by this resource to allocator, to be reused.
        /// @note this means that this resource wont be used in any subsequent operations.
        void Shutdown(RHI::Handle<RHI::Image> image);

        /// @brief returns the memory used by this resource to allocator, to be reused.
        /// @note this means that this resource wont be used in any subsequent operations.
        void Shutdown(RHI::Handle<RHI::Buffer> buffer);

    private:
        std::optional<Allocation> Allocate(VkMemoryRequirements requirements);

        void Free(Allocation allocation);

        struct Block
        {
            VmaAllocation allocation;
            VmaAllocationInfo info;
            VmaVirtualBlock virtualBlock;
            VkMemoryHeapFlags memoryProperties;
        };

        size_t CalculatePreferredBlockSize(uint32_t memTypeIndex);

        Block CreateBlockNewBlock(VkMemoryRequirements minRequirements);

    private:
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

        void OnBegin() override;

        void OnEnd() override;

        RHI::PassQueueState GetPassQueueStateInternal() const override;

        uint32_t m_queueFamilyIndex;

        VkCommandPool m_commandPool;

        VkSemaphore m_signalSemaphore;

        RHI::Handle<Fence> m_signalFence;

        std::unique_ptr<CommandList> m_commandList;
    };

    class FrameScheduler final : public RHI::FrameScheduler
    {
    public:
        FrameScheduler(Context* context);
        ~FrameScheduler();

        VkResult Init();

        std::unique_ptr<RHI::Pass> CreatePass(const RHI::PassCreateInfo& createInfo) override;

        void BeginInternal() override;
        void EndInternal() override;

        void ExecutePass(RHI::Pass* pass) override;

        void Allocate(RHI::Handle<RHI::Attachment> handle) override;
        void Release(RHI::Handle<RHI::Attachment> handle) override;

        RHI::Handle<RHI::Image> CreateTransientImageResource(const RHI::ImageCreateInfo& createInfo) override;
        RHI::Handle<RHI::Buffer> CreateTransientBufferResource(const RHI::BufferCreateInfo& createInfo) override;

        RHI::Handle<RHI::ImageView> CreateImageView(RHI::Attachment* attachment, const RHI::ImageAttachmentUseInfo& useInfo) override;
        RHI::Handle<RHI::BufferView> CreateBufferView(RHI::Attachment* attachment, const RHI::BufferAttachmentUseInfo& useInfo) override;

        void FreeTransientBufferResource(RHI::Handle<RHI::Buffer> handle) override;
        void FreeTransientImageResource(RHI::Handle<RHI::Image> handle) override;

        void FreeImageView(RHI::Handle<RHI::ImageView> handle) override;
        void FreeBufferView(RHI::Handle<RHI::BufferView> handle) override;

    private:
        std::vector<VkSemaphoreSubmitInfo> GetSemaphores(const std::vector<RHI::Pass*>& passes) const;

        std::unique_ptr<TransientResourceAllocator> m_transientAllocator;
    };

} // namespace Vulkan