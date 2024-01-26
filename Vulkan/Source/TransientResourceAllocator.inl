#pragma once
#include "Context.hpp"
#include "Resources.hpp"
#include "Common.hpp"

#include <RHI/FrameScheduler.hpp>

#include <vk_mem_alloc.h>

namespace Vulkan
{

    class TransientResourceAllocator final : public RHI::TransientResourceAllocator
    {
        struct Block
        {
            VmaAllocation allocation;
            VmaAllocationInfo info;
            VmaVirtualBlock virtualBlock;
            VkMemoryHeapFlags memoryProperties;
        };

        std::vector<Block> m_blocks;

    public:
        static std::unique_ptr<TransientResourceAllocator> Create();

        TransientResourceAllocator() = default;
        ~TransientResourceAllocator() = default;

        void Begin(RHI::Context* context) override;
        void End(RHI::Context* context) override;

        void Reset(RHI::Context* context) override;

        void Allocate(RHI::Context* context, RHI::Attachment* attachment) override;
        void Release(RHI::Context* context, RHI::Attachment* attachment) override;
        void Destroy(RHI::Context* context, RHI::Attachment* attachment) override;

    private:
        Allocation AllocateInternal(Context* context, VkMemoryRequirements requirements);

        size_t CalculatePreferredBlockSize(Context* context, uint32_t memTypeIndex);

        Block CreateBlockNewBlock(Context* context, VkMemoryRequirements minRequirements);
    };

    ///////////////////////////////////////////////////////////////////////////
    /// TransientResourceAllocator
    ///////////////////////////////////////////////////////////////////////////

    inline std::unique_ptr<TransientResourceAllocator> TransientResourceAllocator::Create()
    {
        return std::make_unique<TransientResourceAllocator>();
    }

    inline void TransientResourceAllocator::Begin(RHI::Context* _context)
    {
        auto context = (Context*)_context;
        (void)context;
    }

    inline void TransientResourceAllocator::End(RHI::Context* _context)
    {
        auto context = (Context*)_context;
        (void)context;
    }

    inline void TransientResourceAllocator::Reset(RHI::Context* _context)
    {
        auto context = (Context*)_context;
        (void)context;
    }

    inline void TransientResourceAllocator::Allocate(RHI::Context* _context, RHI::Attachment* attachment)
    {
        auto context = (Context*)_context;

        RHI_ASSERT(attachment->lifetime == RHI::Attachment::Lifetime::Transient);

        switch (attachment->type)
        {
        case RHI::Attachment::Type::Image:
            {
                auto imageAttachment = (RHI::ImageAttachment*)attachment;
                auto [handle, image] = context->m_imageOwner.InsertZerod();
                imageAttachment->handle = handle;

                {
                    auto result = image.Init(context, {}, imageAttachment->info, nullptr, true);
                    RHI_ASSERT(result == RHI::ResultCode::Success);
                }

                auto memoryRequirements = image.GetMemoryRequirements(context->m_device);
                image.allocation = AllocateInternal(context, memoryRequirements);
                {
                    auto result = vmaBindImageMemory2(context->m_allocator, image.allocation.handle, image.allocation.offset, image.handle, nullptr);
                    VULKAN_ASSERT_SUCCESS(result);
                }

                break;
            }
        case RHI::Attachment::Type::Buffer:
            {
                auto bufferAttachment = (RHI::BufferAttachment*)attachment;
                auto [handle, buffer] = context->m_bufferOwner.InsertZerod();
                bufferAttachment->handle = handle;

                {
                    auto result = buffer.Init(context, {}, bufferAttachment->info, nullptr, true);
                    RHI_ASSERT(result == RHI::ResultCode::Success);
                }

                auto memoryRequirements = buffer.GetMemoryRequirements(context->m_device);
                buffer.allocation = AllocateInternal(context, memoryRequirements);
                {
                    auto result = vmaBindBufferMemory2(context->m_allocator, buffer.allocation.handle, buffer.allocation.offset, buffer.handle, nullptr);
                    VULKAN_ASSERT_SUCCESS(result);
                }
                break;
            }
        default: RHI_UNREACHABLE();
        }
    }

    inline void TransientResourceAllocator::Release(RHI::Context* _context, RHI::Attachment* attachment)
    {
        auto context = (Context*)_context;

        RHI_ASSERT(attachment->lifetime == RHI::Attachment::Lifetime::Transient);

        Allocation allocation{};

        switch (attachment->type)
        {
        case RHI::Attachment::Type::Image:
            {
                auto imageAttachment = (RHI::ImageAttachment*)attachment;
                auto image = context->m_imageOwner.Get(imageAttachment->handle);
                allocation = image->allocation;
                break;
            }
        case RHI::Attachment::Type::Buffer:
            {
                auto bufferAttachment = (RHI::BufferAttachment*)attachment;
                auto buffer = context->m_bufferOwner.Get(bufferAttachment->handle);
                allocation = buffer->allocation;
                break;
            }
        default: RHI_UNREACHABLE();
        }

        vmaVirtualFree(allocation.virtualBlock, allocation.virtualHandle);
    }

    inline void TransientResourceAllocator::Destroy(RHI::Context* _context, RHI::Attachment* attachment)
    {
        auto context = (Context*)_context;
        switch (attachment->type)
        {
        case RHI::Attachment::Type::Image:
            {
                auto imageAttachment = (RHI::ImageAttachment*)attachment;
                auto image = context->m_imageOwner.Get(imageAttachment->handle);
                image->Shutdown(context);
                context->m_imageOwner.Remove(imageAttachment->handle);
                break;
            }
        case RHI::Attachment::Type::Buffer:
            {
                auto bufferAttachment = (RHI::BufferAttachment*)attachment;
                auto buffer = context->m_bufferOwner.Get(bufferAttachment->handle);
                buffer->Shutdown(context);
                context->m_bufferOwner.Remove(bufferAttachment->handle);
                break;
            }
        default: RHI_UNREACHABLE();
        }
    }

    inline Allocation TransientResourceAllocator::AllocateInternal(Context* context, VkMemoryRequirements requirements)
    {
        size_t offset = SIZE_MAX;
        VmaVirtualAllocation virtualHandle = VK_NULL_HANDLE;

        for (const auto& block : m_blocks)
        {
            if ((block.memoryProperties & requirements.memoryTypeBits) != requirements.memoryTypeBits)
            {
                continue;
            }

            VmaVirtualAllocationCreateInfo createInfo{};
            createInfo.size = block.info.size;
            createInfo.alignment = requirements.size;
            createInfo.flags = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
            createInfo.pUserData = nullptr;
            auto result = vmaVirtualAllocate(block.virtualBlock, &createInfo, &virtualHandle, &offset);
            VULKAN_ASSERT_SUCCESS(result);

            Allocation allocation{};
            allocation.handle = block.allocation;
            vmaGetAllocationInfo(context->m_allocator, block.allocation, &allocation.info);
            allocation.offset = offset;
            allocation.virtualBlock = block.virtualBlock;
            allocation.virtualHandle = virtualHandle;
            return allocation;
        }

        const auto& block = m_blocks.emplace_back(CreateBlockNewBlock(context, requirements));

        VmaVirtualAllocationCreateInfo createInfo{};
        createInfo.size = block.info.size;
        createInfo.alignment = requirements.size;
        createInfo.flags = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        createInfo.pUserData = nullptr;

        auto result = vmaVirtualAllocate(block.virtualBlock, &createInfo, &virtualHandle, &offset);
        VULKAN_ASSERT_SUCCESS(result);

        Allocation allocation{};
        allocation.handle = block.allocation;
        vmaGetAllocationInfo(context->m_allocator, block.allocation, &allocation.info);
        allocation.offset = offset;
        allocation.virtualBlock = block.virtualBlock;
        allocation.virtualHandle = virtualHandle;
        return allocation;
    }

    inline size_t TransientResourceAllocator::CalculatePreferredBlockSize(Context* context, uint32_t memTypeIndex)
    {
        constexpr size_t VMA_SMALL_HEAP_MAX_SIZE = (1024ull * 1024 * 1024);
        constexpr size_t VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE = (256ull * 1024 * 1024);

        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties(context->m_physicalDevice, &properties);

        auto heapIndex = properties.memoryTypes[memTypeIndex].heapIndex;
        auto heapSize = properties.memoryHeaps[heapIndex].size;
        auto isSmallHeap = heapSize <= VMA_SMALL_HEAP_MAX_SIZE;
        return AlignUp(isSmallHeap ? (heapSize / 8) : VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE, (size_t)32);
    }

    inline TransientResourceAllocator::Block TransientResourceAllocator::CreateBlockNewBlock(Context* context, VkMemoryRequirements minRequirements)
    {
        // TODO: Hardcoded for my local machine, should probably handle this later.
        minRequirements.size = CalculatePreferredBlockSize(context, 2);

        Block block{};

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        auto result = vmaAllocateMemory(context->m_allocator, &minRequirements, &allocationCreateInfo, &block.allocation, &block.info);
        VULKAN_ASSERT_SUCCESS(result);

        VmaVirtualBlockCreateInfo virtualBlockCreateInfo{};
        virtualBlockCreateInfo.size = block.info.size;
        virtualBlockCreateInfo.flags = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        result = vmaCreateVirtualBlock(&virtualBlockCreateInfo, &block.virtualBlock);
        VULKAN_ASSERT_SUCCESS(result);

        return block;
    }

} // namespace Vulkan