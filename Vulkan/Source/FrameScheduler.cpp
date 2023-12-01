#include "FrameScheduler.hpp"

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

    void TransientAttachmentAllocator::Activate(RHI::Image* resourceBase)
    {
        auto image = static_cast<Image*>(resourceBase);
        auto requirements = image->GetMemoryRequirements(m_context->m_device);

        if (auto allocation = Allocate(requirements); allocation.has_value())
        {
            RHI_ASSERT(allocation->type == AllocationType::Aliasing);
            auto result = vmaBindImageMemory2(m_context->m_allocator, allocation->handle, allocation->virtualAllocation.offset, image->handle, nullptr);
            VULKAN_ASSERT_SUCCESS(result);
        }
    }

    void TransientAttachmentAllocator::Deactivate(RHI::Image* resourceBase)
    {
        auto image = static_cast<Image*>(resourceBase);
        Free(image->allocation);
    }

    void TransientAttachmentAllocator::Activate(RHI::Buffer* resourceBase)
    {
        auto buffer = static_cast<Buffer*>(resourceBase);
        auto requirements = buffer->GetMemoryRequirements(m_context->m_device);

        if (auto allocation = Allocate(requirements); allocation.has_value())
        {
            RHI_ASSERT(allocation->type == AllocationType::Aliasing);
            auto result = vmaBindBufferMemory2(m_context->m_allocator, allocation->handle, allocation->virtualAllocation.offset, buffer->handle, nullptr);
            VULKAN_ASSERT_SUCCESS(result);
        }
    }

    void TransientAttachmentAllocator::Deactivate(RHI::Buffer* resourceBase)
    {
        auto buffer = static_cast<Buffer*>(resourceBase);
        Free(buffer->allocation);
    }

    RHI::Handle<RHI::Image> TransientAttachmentAllocator::CreateTransientImage(const RHI::ImageCreateInfo& createInfo)
    {
        auto [handle, image] = m_context->m_imageOwner.InsertZerod();
        auto result = image.Init(m_context, {}, createInfo, nullptr);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return handle;
    }

    RHI::Handle<RHI::Buffer> TransientAttachmentAllocator::CreateTransientBuffer(const RHI::BufferCreateInfo& createInfo)
    {
        auto [handle, buffer] = m_context->m_bufferOwner.InsertZerod();
        auto result = buffer.Init(m_context, {}, createInfo, nullptr);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return handle;
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

    RHI::CommandList& Pass::BeginCommandList(uint32_t commandsCount)
    {
        (void)commandsCount;
        auto commandList = static_cast<CommandList*>(m_commandList);

        commandList->Begin();
        if (m_queueType == RHI::QueueType::Graphics)
        {
            commandList->RenderingBegin(*this);
        }

        return *commandList;
    }

    void Pass::EndCommandList()
    {
        auto commandList = static_cast<CommandList*>(m_commandList);

        if (m_queueType == RHI::QueueType::Graphics)
        {
            commandList->RenderingEnd(*this);
        }
        commandList->End();
    }

    ///////////////////////////////////////////////////////////////////////////
    /// FrameScheduler
    ///////////////////////////////////////////////////////////////////////////

    FrameScheduler::FrameScheduler(Context* context)
        : RHI::FrameScheduler(static_cast<RHI::Context*>(context))
    {
        m_transientAttachmentAllocator = std::make_unique<TransientAttachmentAllocator>(context);
        m_graphicsCommandlistAllocator = std::make_unique<CommandListAllocator>(context);
    }

    FrameScheduler::~FrameScheduler()
    {
    }

    VkResult FrameScheduler::Init()
    {
        auto context = static_cast<Context*>(m_context);

        auto result = m_graphicsCommandlistAllocator->Init(context->m_graphicsQueueFamilyIndex, 3);
        VULKAN_RETURN_VKERR_CODE(result);

        for (uint32_t i = 0; i < 3; i++)
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

    std::vector<VkSemaphoreSubmitInfo> FrameScheduler::GetSemaphores(const std::vector<RHI::Pass*>& passes) const
    {
        std::vector<VkSemaphoreSubmitInfo> submitInfos{};
        for (auto passBase : passes)
        {
            auto pass = static_cast<Pass*>(passBase);

            VkSemaphoreSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            submitInfo.pNext = nullptr;
            submitInfo.semaphore = pass->m_signalSemaphore;
            // submitInfo.value;
            submitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR; // ConvertShaderStage(pass);
            // submitInfo.deviceIndex;
            submitInfos.push_back(submitInfo);
        }
        return submitInfos;
    }

    void FrameScheduler::ExecutePass(RHI::Pass& passBase)
    {
        auto context = static_cast<Context*>(m_context);
        auto& pass = static_cast<Pass&>(passBase);
        auto commandList = static_cast<CommandList*>(pass.m_commandList);
        auto waitSemaphores = GetSemaphores(pass.m_producers);
        auto signalSemaphores = GetSemaphores(pass.m_consumers);

        if (pass.m_swapchain)
        {
            auto imageReadySemaphore = static_cast<Swapchain*>(pass.m_swapchain)->m_imageReadySemaphore;

            auto submitInfo = VkSemaphoreSubmitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            submitInfo.pNext = nullptr;
            submitInfo.semaphore = imageReadySemaphore;
            submitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR; // ConvertShaderStage(pass);
            waitSemaphores.push_back(submitInfo);
        }

        auto signalSemaphore = VkSemaphoreSubmitInfo{};
        signalSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signalSemaphore.pNext = nullptr;
        signalSemaphore.semaphore = pass.m_signalSemaphore;
        signalSemaphore.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR; // ConvertShaderStage(pass);

        auto commandSubmitInfo = VkCommandBufferSubmitInfo{};
        commandSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandSubmitInfo.pNext = nullptr;
        commandSubmitInfo.commandBuffer = commandList->m_commandBuffer;

        auto submitInfo = VkSubmitInfo2{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.pNext = nullptr;
        submitInfo.flags = 0;
        submitInfo.waitSemaphoreInfoCount = waitSemaphores.size();
        submitInfo.pWaitSemaphoreInfos = waitSemaphores.data();
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &commandSubmitInfo;
        submitInfo.signalSemaphoreInfoCount = 1;
        submitInfo.pSignalSemaphoreInfos = &signalSemaphore;

        auto currentFrameFence = m_framesInflightFences[m_frameNumber % 3];

        auto result = vkQueueSubmit2(context->m_graphicsQueue, 1, &submitInfo, currentFrameFence);
        VULKAN_ASSERT_SUCCESS(result);
    }

    void FrameScheduler::ResetPass(RHI::Pass& passBase)
    {
        auto& pass = static_cast<Pass&>(passBase);
        auto context = static_cast<Context*>(m_context);
        auto result = vkResetFences(context->m_device, 1, &pass.m_signalFence);
        VULKAN_ASSERT_SUCCESS(result);
    }

    RHI::CommandList* FrameScheduler::GetCommandList(uint32_t frameIndex)
    {
        m_graphicsCommandlistAllocator->SetFrameIndex(frameIndex);
        return m_graphicsCommandlistAllocator->Allocate();
    }

    void FrameScheduler::OnFrameEnd()
    {
        auto context = static_cast<Context*>(m_context);
        auto currentFrameFence = m_framesInflightFences[m_frameNumber % 3];
        auto result = vkWaitForFences(context->m_device, 1, &currentFrameFence, VK_TRUE, 1000);
        result = vkResetFences(context->m_device, 1, &currentFrameFence);
        VULKAN_ASSERT_SUCCESS(result);
    }

} // namespace Vulkan