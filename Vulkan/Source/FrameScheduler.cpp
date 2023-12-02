#include "FrameScheduler.hpp"
#include "Conversion.hpp"
#include "CommandList.hpp"
#include "Common.hpp"
#include "Context.hpp"
#include "Resources.hpp"

#include <memory>

namespace Vulkan
{
    ///////////////////////////////////////////////////////////////////////////
    /// TransientAttachmentAllocator
    ///////////////////////////////////////////////////////////////////////////

    TransientAttachmentAllocator::TransientAttachmentAllocator(Context* context)
        : m_context(context)
    {
    }

    TransientAttachmentAllocator::~TransientAttachmentAllocator()
    {
        for (auto block : m_blocks)
        {
            vmaFreeMemory(m_context->m_allocator, block.allocation);
            vmaDestroyVirtualBlock(block.virtualBlock);
        }
    }

    void TransientAttachmentAllocator::Begin()
    {
    }

    void TransientAttachmentAllocator::End()
    {
        std::vector<VmaDetailedStatistics> statistics;
        statistics.resize(m_blocks.size());
        for (auto block : m_blocks)
        {
            vmaCalculateVirtualBlockStatistics(block.virtualBlock, &statistics.back());
        }
    }

    void TransientAttachmentAllocator::Allocate(RHI::ImageAttachment* attachment)
    {
        RHI_ASSERT(attachment->lifetime == RHI::AttachmentLifetime::Transient);

        auto [handle, image] = m_context->m_imageOwner.InsertZerod();
        attachment->handle = handle;

        auto result = image.Init(m_context, {}, attachment->info, nullptr, true);
        RHI_ASSERT(result == RHI::ResultCode::Success);

        auto requirements = image.GetMemoryRequirements(m_context->m_device);
        if (auto allocation = Allocate(requirements); allocation.has_value())
        {
            RHI_ASSERT(allocation->type == AllocationType::Aliasing);
            auto result = vmaBindImageMemory2(m_context->m_allocator, allocation->handle, allocation->virtualAllocation.offset, image.handle, nullptr);
            VULKAN_ASSERT_SUCCESS(result);
        }
    }

    void TransientAttachmentAllocator::Free(RHI::ImageAttachment* attachment)
    {
        auto image = m_context->m_imageOwner.Get(attachment->handle);
        RHI_ASSERT(image);

        Free(image->allocation);
    }

    void TransientAttachmentAllocator::Allocate(RHI::BufferAttachment* attachment)
    {
        RHI_ASSERT(attachment->lifetime == RHI::AttachmentLifetime::Transient);

        auto [handle, buffer] = m_context->m_bufferOwner.InsertZerod();
        attachment->handle = handle;

        auto result = buffer.Init(m_context, {}, attachment->info, nullptr, true);
        RHI_ASSERT(result == RHI::ResultCode::Success);

        auto requirements = buffer.GetMemoryRequirements(m_context->m_device);
        if (auto allocation = Allocate(requirements); allocation.has_value())
        {
            RHI_ASSERT(allocation->type == AllocationType::Aliasing);
            auto result = vmaBindBufferMemory2(m_context->m_allocator, allocation->handle, allocation->virtualAllocation.offset, buffer.handle, nullptr);
            VULKAN_ASSERT_SUCCESS(result);
        }
    }

    void TransientAttachmentAllocator::Free(RHI::BufferAttachment* attachment)
    {
        auto buffer = m_context->m_imageOwner.Get(attachment->handle);
        RHI_ASSERT(buffer);

        Free(buffer->allocation);
    }

    size_t TransientAttachmentAllocator::CalculatePreferredBlockSize(uint32_t memTypeIndex)
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

    TransientAttachmentAllocator::Block TransientAttachmentAllocator::CreateBlockNewBlock(VkMemoryRequirements minRequirements)
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

    std::optional<Allocation> TransientAttachmentAllocator::Allocate(VkMemoryRequirements requirements)
    {
        Allocation allocation;
        allocation.type = AllocationType::Aliasing;

        for (auto& block : m_blocks)
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
            auto result = vmaVirtualAllocate(block.virtualBlock, &createInfo, &allocation.virtualAllocation.handle, &allocation.virtualAllocation.offset);

            if (result == VK_SUCCESS)
            {
                allocation.handle = block.allocation;
                allocation.info = block.info;
                return allocation;
            }
        }

        Block block = m_blocks.emplace_back(CreateBlockNewBlock(requirements));
        allocation.handle = block.allocation;

        VmaVirtualAllocationCreateInfo createInfo{};
        createInfo.size = block.info.size;
        createInfo.alignment = requirements.size;
        createInfo.flags = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        createInfo.pUserData = nullptr;
        allocation.info = block.info;
        auto result = vmaVirtualAllocate(block.virtualBlock, &createInfo, &allocation.virtualAllocation.handle, &allocation.virtualAllocation.offset);

        if (result == VK_SUCCESS)
        {
            return allocation;
        }

        return std::nullopt;
    }

    void TransientAttachmentAllocator::Free(Allocation allocation)
    {
        vmaVirtualFree(allocation.virtualAllocation.blockHandle, allocation.virtualAllocation.handle);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Pass
    ///////////////////////////////////////////////////////////////////////////

    Pass::Pass(Context* context)
        : RHI::Pass(context)
    {
    }

    Pass::~Pass()
    {
        auto context = static_cast<Context*>(m_context);
        vkDeviceWaitIdle(context->m_device);

        if (m_signalSemaphore)
        {
            vkDestroySemaphore(context->m_device, m_signalSemaphore, nullptr);
        }

        if (m_signalFence)
        {
            vkDestroyFence(context->m_device, m_signalFence, nullptr);
        }
    }

    VkResult Pass::Init(const RHI::PassCreateInfo& createInfo)
    {
        m_name = createInfo.name;
        m_queueType = createInfo.type;
        m_queueFamilyIndex = static_cast<Context*>(m_context)->GetQueueFamilyIndex(m_queueType);

        auto context = static_cast<Context*>(m_context);

        m_signalSemaphore = context->CreateSemaphore();
        m_signalFence = context->CreateFence();

        return VK_SUCCESS;
    }

    std::vector<VkSemaphoreSubmitInfo> Pass::GetWaitSemaphoreSubmitInfos() const
    {
        std::vector<VkSemaphoreSubmitInfo> result;

        VkSemaphoreSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;

        for (auto& imageAttachments : m_imagePassAttachments)
        {
            if (auto swapchainBase = imageAttachments.attachment->swapchain)
            {
                auto swapchain = static_cast<Swapchain*>(swapchainBase);
                submitInfo.stageMask = ConvertPipelineStageFlags(imageAttachments.info.usage, imageAttachments.stage);
                submitInfo.semaphore = swapchain->m_imageReadySemaphore;
                result.push_back(submitInfo);
            }
        }
        return result;
    }

    std::vector<VkSemaphoreSubmitInfo> Pass::GetSignalSemaphoreSubmitInfos() const
    {
        VkSemaphoreSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        submitInfo.semaphore = m_signalSemaphore;

        return std::vector<VkSemaphoreSubmitInfo>{ submitInfo };
    }

    ///////////////////////////////////////////////////////////////////////////
    /// FrameScheduler
    ///////////////////////////////////////////////////////////////////////////

    FrameScheduler::FrameScheduler(Context* context)
        : RHI::FrameScheduler(static_cast<RHI::Context*>(context))
    {
        m_transientAttachmentAllocator = std::make_unique<TransientAttachmentAllocator>(context);
    }

    FrameScheduler::~FrameScheduler()
    {
    }

    VkResult FrameScheduler::Init()
    {
        auto context = static_cast<Context*>(m_context);

        for (uint32_t i = 0; i < m_frameBufferingMaxCount; i++)
        {
            m_framesInflightFences.push_back(context->CreateFence());
        }

        return VK_SUCCESS;
    }

    std::unique_ptr<RHI::Pass> FrameScheduler::CreatePass(const RHI::PassCreateInfo& createInfo)
    {
        std::unique_ptr<Pass> pass = std::make_unique<Pass>(static_cast<Context*>(m_context));
        pass->m_scheduler = this;
        auto result = pass->Init(createInfo);
        RHI_ASSERT(result == VK_SUCCESS);
        return pass;
    }

    void FrameScheduler::ExecutePass(RHI::Pass& passBase)
    {
        auto context = static_cast<Context*>(m_context);
        auto& pass = static_cast<Pass&>(passBase);

        std::vector<VkCommandBufferSubmitInfo> commandBuffers = {};

        for (auto commandList : pass.m_commandLists)
        {
            VkCommandBufferSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            submitInfo.commandBuffer = static_cast<CommandList*>(commandList)->m_commandBuffer;
            commandBuffers.push_back(submitInfo);
        }

        auto waitSemaphores = pass.GetWaitSemaphoreSubmitInfos();
        // auto signalSemaphores = pass.GetSignalSemaphoreSubmitInfos();

        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.pNext = nullptr;
        submitInfo.flags = 0;
        submitInfo.waitSemaphoreInfoCount = waitSemaphores.size();
        submitInfo.pWaitSemaphoreInfos = waitSemaphores.data();
        submitInfo.commandBufferInfoCount = commandBuffers.size();
        submitInfo.pCommandBufferInfos = commandBuffers.data();
        // submitInfo.signalSemaphoreInfoCount = signalSemaphores.size();
        // submitInfo.pSignalSemaphoreInfos = signalSemaphores.data();

        auto fence = GetCurrentFrameFence();
        // auto fence = VK_NULL_HANDLE;

        auto result = vkQueueSubmit2(context->m_graphicsQueue, 1, &submitInfo, fence);
        VULKAN_ASSERT_SUCCESS(result);

        pass.m_commandLists.clear();
    }

    void FrameScheduler::OnFrameBegin()
    {
        auto context = static_cast<Context*>(m_context);
        auto fence = GetCurrentFrameFence();
        vkWaitForFences(context->m_device, 1, &fence, VK_TRUE, 1e+9);
        vkResetFences(context->m_device, 1, &fence);
    }

    void FrameScheduler::OnFrameEnd()
    {
    }

    VkFence FrameScheduler::GetCurrentFrameFence() const
    {
        auto fence = m_framesInflightFences[m_frameIndex % m_frameBufferingMaxCount];
        return fence;
    }

} // namespace Vulkan