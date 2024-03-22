#pragma once
#include <RHI/TransientAllocator.hpp>

#include <RHI/Common/Containers.h>

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    struct Allocation;
    class IContext;

    class ITransientAllocator final : public TransientAllocator
    {
    public:
        ITransientAllocator(IContext* context)
            : m_context(context){};
        ~ITransientAllocator();

        void Begin() override;
        void End() override;
        void Allocate(Attachment* attachment) override;
        void Release(Attachment* attachment) override;
        void Destroy(Attachment* attachment) override;

    private:
        struct Block
        {
            VmaAllocation allocation;
            VmaAllocationInfo info;
            VmaVirtualBlock virtualBlock;
            VkMemoryHeapFlags memoryProperties;
        };

        Allocation AllocateInternal(VkMemoryRequirements requirements);

        size_t CalculatePreferredBlockSize(uint32_t memTypeIndex);

        Block CreateBlockNewBlock(VkMemoryRequirements minRequirements);

    private:
        IContext* m_context;
        TL::Vector<Block> m_blocks;
    };
} // namespace RHI::Vulkan