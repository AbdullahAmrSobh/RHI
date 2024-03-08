#pragma once
#include <RHI/Pass.hpp>

#include "Common.hpp"
#include "Resources.hpp"
#include "Context.hpp"
#include "TransientAllocator.hpp"

namespace RHI::Vulkan
{
    ITransientAllocator::~ITransientAllocator()
    {
    }

    void ITransientAllocator::Begin()
    {
    }

    void ITransientAllocator::End()
    {
    }

    void ITransientAllocator::Allocate(Attachment* attachment)
    {
        RHI_ASSERT(attachment->m_lifetime == Attachment::Lifetime::Transient);
        switch (attachment->m_type)
        {
        case Attachment::Type::Image:
            {
                auto imageAttachment = (ImageAttachment*)attachment;
                auto [handle, image] = m_context->m_imageOwner.InsertZerod();
                imageAttachment->SetHandle(handle);

                auto result = image.Init(m_context, imageAttachment->GetCreateInfo(), true);
                RHI_ASSERT(result == ResultCode::Success);

                auto memoryRequirements = image.GetMemoryRequirements(m_context->m_device);
                image.allocation = AllocateInternal(memoryRequirements);
                auto vkresult = vmaBindImageMemory2(m_context->m_allocator, image.allocation.handle, image.allocation.offset, image.handle, nullptr);
                VULKAN_ASSERT_SUCCESS(vkresult);

                break;
            }
        case Attachment::Type::Buffer:
            {
                auto bufferAttachment = (BufferAttachment*)attachment;
                auto [handle, buffer] = m_context->m_bufferOwner.InsertZerod();
                bufferAttachment->SetHandle(handle);

                auto result = buffer.Init(m_context, bufferAttachment->GetCreateInfo(), true);
                RHI_ASSERT(result == ResultCode::Success);

                auto memoryRequirements = buffer.GetMemoryRequirements(m_context->m_device);
                buffer.allocation = AllocateInternal(memoryRequirements);
                auto vkresult = vmaBindBufferMemory2(m_context->m_allocator, buffer.allocation.handle, buffer.allocation.offset, buffer.handle, nullptr);
                VULKAN_ASSERT_SUCCESS(vkresult);
                break;
            }
        default: RHI_UNREACHABLE();
        }
    }

    void ITransientAllocator::Release(Attachment* attachment)
    {
        RHI_ASSERT(attachment->m_lifetime == Attachment::Lifetime::Transient);

        switch (attachment->m_type)
        {
        case Attachment::Type::Image:
            {
                auto imageAttachment = (ImageAttachment*)attachment;
                auto image = m_context->m_imageOwner.Get(imageAttachment->GetHandle());
                vmaVirtualFree(image->allocation.virtualBlock, image->allocation.virtualHandle);
                break;
            }
        case Attachment::Type::Buffer:
            {
                auto bufferAttachment = (BufferAttachment*)attachment;
                auto buffer = m_context->m_bufferOwner.Get(bufferAttachment->GetHandle());
                vmaVirtualFree(buffer->allocation.virtualBlock, buffer->allocation.virtualHandle);
                break;
            }
        default: RHI_UNREACHABLE();
        }
    }

    void ITransientAllocator::Destroy(Attachment* attachment)
    {
        RHI_ASSERT(attachment->m_lifetime == Attachment::Lifetime::Transient);
        switch (attachment->m_type)
        {
        case Attachment::Type::Image:
            {
                auto imageAttachment = (ImageAttachment*)attachment;
                auto image = m_context->m_imageOwner.Get(imageAttachment->GetHandle());
                image->Shutdown(m_context);
                m_context->m_imageOwner.Remove(imageAttachment->GetHandle());
                break;
            }
        case Attachment::Type::Buffer:
            {
                auto bufferAttachment = (BufferAttachment*)attachment;
                auto buffer = m_context->m_bufferOwner.Get(bufferAttachment->GetHandle());
                buffer->Shutdown(m_context);
                m_context->m_bufferOwner.Remove(bufferAttachment->GetHandle());
                break;
            }
        default: RHI_UNREACHABLE();
        }
    }

    // private
    Allocation ITransientAllocator::AllocateInternal(VkMemoryRequirements requirements)
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
            vmaGetAllocationInfo(m_context->m_allocator, block.allocation, &allocation.info);
            allocation.offset = offset;
            allocation.virtualBlock = block.virtualBlock;
            allocation.virtualHandle = virtualHandle;
            return allocation;
        }

        const auto& block = m_blocks.emplace_back(CreateBlockNewBlock(requirements));

        VmaVirtualAllocationCreateInfo createInfo{};
        createInfo.size = block.info.size;
        createInfo.alignment = requirements.size;
        createInfo.flags = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        createInfo.pUserData = nullptr;

        auto result = vmaVirtualAllocate(block.virtualBlock, &createInfo, &virtualHandle, &offset);
        VULKAN_ASSERT_SUCCESS(result);

        Allocation allocation{};
        allocation.handle = block.allocation;
        vmaGetAllocationInfo(m_context->m_allocator, block.allocation, &allocation.info);
        allocation.offset = offset;
        allocation.virtualBlock = block.virtualBlock;
        allocation.virtualHandle = virtualHandle;
        return allocation;
    }

    size_t ITransientAllocator::CalculatePreferredBlockSize(uint32_t memTypeIndex)
    {
        constexpr size_t VMA_SMALL_HEAP_MAX_SIZE = (1024ull * 1024 * 1024);
        constexpr size_t VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE = (256ull * 1024 * 1024);

        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties(m_context->m_physicalDevice, &properties);

        auto heapIndex = properties.memoryTypes[memTypeIndex].heapIndex;
        auto heapSize = properties.memoryHeaps[heapIndex].size;
        auto isSmallHeap = heapSize <= VMA_SMALL_HEAP_MAX_SIZE;
        return AlignUp(isSmallHeap ? (heapSize / 8) : VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE, (size_t)32);
    }

    ITransientAllocator::Block ITransientAllocator::CreateBlockNewBlock(VkMemoryRequirements minRequirements)
    {
        // TODO: Hardcoded for my local machine, should probably handle this later.
        minRequirements.size = CalculatePreferredBlockSize(2);

        Block block{};

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        auto result = vmaAllocateMemory(m_context->m_allocator, &minRequirements, &allocationCreateInfo, &block.allocation, &block.info);
        VULKAN_ASSERT_SUCCESS(result);

        VmaVirtualBlockCreateInfo virtualBlockCreateInfo{};
        virtualBlockCreateInfo.size = block.info.size;
        virtualBlockCreateInfo.flags = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        result = vmaCreateVirtualBlock(&virtualBlockCreateInfo, &block.virtualBlock);
        VULKAN_ASSERT_SUCCESS(result);

        return block;
    }

} // namespace RHI::Vulkan