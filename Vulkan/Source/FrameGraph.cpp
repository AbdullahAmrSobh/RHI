#include "FrameGraph.hpp"

#include "Allocator.hpp"
#include "CommandList.hpp"
#include "Common.hpp"
#include "Context.hpp"
#include "Resources.hpp"

#include <memory>

namespace Vulkan
{
    ///////////////////////////////////////////////////////////////////////////
    /// Utility functions
    ///////////////////////////////////////////////////////////////////////////

    VkAttachmentLoadOp ConvertLoadOp(RHI::ImageLoadOperation op)
    {
        switch (op)
        {
        case RHI::ImageLoadOperation::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case RHI::ImageLoadOperation::Load:     return VK_ATTACHMENT_LOAD_OP_LOAD;
        case RHI::ImageLoadOperation::Discard:  return VK_ATTACHMENT_LOAD_OP_CLEAR;
        default:                                RHI_UNREACHABLE(); return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
        }
    }

    VkAttachmentStoreOp ConvertStoreOp(RHI::ImageStoreOperation op)
    {
        switch (op)
        {
        case RHI::ImageStoreOperation::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case RHI::ImageStoreOperation::Store:    return VK_ATTACHMENT_STORE_OP_STORE;
        case RHI::ImageStoreOperation::Discard:  return VK_ATTACHMENT_STORE_OP_NONE;
        default:                                 RHI_UNREACHABLE(); return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
        }
    }

    VkImageLayout ConvertImageLayout(RHI::AttachmentUsage usage, RHI::AttachmentAccess access)
    {
        switch (usage)
        {
        case RHI::AttachmentUsage::RenderTarget: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case RHI::AttachmentUsage::Depth:        return access == RHI::AttachmentAccess::Read ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        // case RHI::AttachmentUsage::Stencil:        return access == RHI::AttachmentAccess::Read ? VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        // case RHI::AttachmentUsage::DepthStencil:   return access == RHI::AttachmentAccess::Read ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case RHI::AttachmentUsage::ShaderResource: return access == RHI::AttachmentAccess::Read ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        case RHI::AttachmentUsage::Copy:           return access == RHI::AttachmentAccess::Read ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        default:                                   RHI_UNREACHABLE(); return VK_IMAGE_LAYOUT_GENERAL;
        }
    }

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

        // for (auto stats : statistics)
        // {

        // }
    }

    void TransientAttachmentAllocator::Allocate(RHI::ImageAttachment* attachment)
    {
        RHI_ASSERT(attachment->lifetime == RHI::AttachmentLifetime::Transient);

        attachment->handle = m_context->m_resourceManager->CreateImage({}, attachment->info, nullptr, true).GetValue();

        auto image = m_context->m_resourceManager->m_imageOwner.Get(attachment->handle);
        RHI_ASSERT(image->swapchain == nullptr);

        auto requirements = image->GetMemoryRequirements(m_context->m_device);
        if (auto allocation = Allocate(requirements); allocation.has_value())
        {
            RHI_ASSERT(allocation->type == AllocationType::Aliasing);
            auto result = vmaBindImageMemory2(m_context->m_allocator, allocation->handle, allocation->virtualAllocation.offset, image->handle, nullptr);
            VULKAN_ASSERT_SUCCESS(result);
        }
    }

    void TransientAttachmentAllocator::Free(RHI::ImageAttachment* attachment)
    {
        auto image = m_context->m_resourceManager->m_imageOwner.Get(attachment->handle);
        RHI_ASSERT(image);

        Free(image->allocation);
    }

    void TransientAttachmentAllocator::Allocate(RHI::BufferAttachment* attachment)
    {
        RHI_ASSERT(attachment->lifetime == RHI::AttachmentLifetime::Transient);

        attachment->handle = m_context->m_resourceManager->CreateBuffer({}, attachment->info, nullptr, true).GetValue();

        auto buffer = m_context->m_resourceManager->m_bufferOwner.Get(attachment->handle);
        RHI_ASSERT(buffer);

        auto requirements = buffer->GetMemoryRequirements(m_context->m_device);
        if (auto allocation = Allocate(requirements); allocation.has_value())
        {
            RHI_ASSERT(allocation->type == AllocationType::Aliasing);
            auto result = vmaBindBufferMemory2(m_context->m_allocator, allocation->handle, allocation->virtualAllocation.offset, buffer->handle, nullptr);
            VULKAN_ASSERT_SUCCESS(result);
        }
    }

    void TransientAttachmentAllocator::Free(RHI::BufferAttachment* attachment)
    {
        auto buffer = m_context->m_resourceManager->m_imageOwner.Get(attachment->handle);
        RHI_ASSERT(buffer);

        Free(buffer->allocation);
    }

    size_t TransientAttachmentAllocator::CalculatePreferredBlockSize(uint32_t memTypeIndex)
    {
        constexpr size_t                 VMA_SMALL_HEAP_MAX_SIZE           = (1024ull * 1024 * 1024);
        constexpr size_t                 VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE = (256ull * 1024 * 1024);

        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties(m_context->m_physicalDevice, &properties);

        auto heapIndex   = properties.memoryTypes[memTypeIndex].heapIndex;
        auto heapSize    = properties.memoryHeaps[heapIndex].size;
        auto isSmallHeap = heapSize <= VMA_SMALL_HEAP_MAX_SIZE;
        return AlignUp(isSmallHeap ? (heapSize / 8) : VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE, (size_t)32);
    }

    TransientAttachmentAllocator::Block TransientAttachmentAllocator::CreateBlockNewBlock(VkMemoryRequirements minRequirements)
    {
        // TODO: Hardcoded for my local machine, should probably handle this later.
        minRequirements.size = CalculatePreferredBlockSize(2);

        Block                   block{};

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.flags         = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        auto result = vmaAllocateMemory(m_context->m_allocator, &minRequirements, &allocationCreateInfo, &block.allocation, &block.info);
        VULKAN_ASSERT_SUCCESS(result);

        VmaVirtualBlockCreateInfo virtualBlockCreateInfo{};
        virtualBlockCreateInfo.size  = block.info.size;
        virtualBlockCreateInfo.flags = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        result                       = vmaCreateVirtualBlock(&virtualBlockCreateInfo, &block.virtualBlock);
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
            createInfo.size      = block.info.size;
            createInfo.alignment = requirements.size;
            createInfo.flags     = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
            createInfo.pUserData = nullptr;
            auto result          = vmaVirtualAllocate(block.virtualBlock, &createInfo, &allocation.virtualAllocation.handle, &allocation.virtualAllocation.offset);

            if (result == VK_SUCCESS)
            {
                allocation.handle = block.allocation;
                allocation.info   = block.info;
                return allocation;
            }
        }

        Block block       = m_blocks.emplace_back(CreateBlockNewBlock(requirements));
        allocation.handle = block.allocation;

        VmaVirtualAllocationCreateInfo createInfo{};
        createInfo.size      = block.info.size;
        createInfo.alignment = requirements.size;
        createInfo.flags     = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        createInfo.pUserData = nullptr;
        allocation.info      = block.info;
        auto result          = vmaVirtualAllocate(block.virtualBlock, &createInfo, &allocation.virtualAllocation.handle, &allocation.virtualAllocation.offset);

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
        m_name             = createInfo.name;
        m_queueType        = createInfo.type;
        m_queueFamilyIndex = static_cast<Context*>(m_context)->GetQueueFamilyIndex(m_queueType);

        auto context = static_cast<Context*>(m_context);

        m_signalSemaphore = context->CreateSemaphore();
        m_signalFence     = context->CreateFence();

        return VK_SUCCESS;
    }

    RHI::CommandList& Pass::BeginCommandList(uint32_t commandsCount)
    {
        auto context     = static_cast<Context*>(m_context);
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
        auto context     = static_cast<Context*>(m_context);
        auto commandList = static_cast<CommandList*>(m_commandList);

        if (m_queueType == RHI::QueueType::Graphics)
        {
            commandList->RenderingEnd(*this);
        }
        commandList->End();
    }

    RHI::PassQueueState Pass::GetPassQueueStateInternal() const
    {
        return {};
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
        pass->m_scheduler          = this;
        auto result                = pass->Init(createInfo);
        RHI_ASSERT(result == VK_SUCCESS);
        return pass;
    }

    std::vector<VkSemaphoreSubmitInfo> FrameScheduler::GetSemaphores(const std::vector<RHI::Pass*>& passes) const
    {
        std::vector<VkSemaphoreSubmitInfo> submitInfos{};
        for (auto passBase : passes)
        {
            auto                  pass = static_cast<Pass*>(passBase);

            VkSemaphoreSubmitInfo submitInfo{};
            submitInfo.sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            submitInfo.pNext     = nullptr;
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
        auto  context          = static_cast<Context*>(m_context);
        auto& pass             = static_cast<Pass&>(passBase);
        auto  commandList      = static_cast<CommandList*>(pass.m_commandList);
        auto  waitSemaphores   = GetSemaphores(pass.m_producers);
        auto  signalSemaphores = GetSemaphores(pass.m_consumers);

        if (pass.m_swapchain)
        {
            auto imageReadySemaphore = static_cast<Swapchain*>(pass.m_swapchain)->m_imageReadySemaphore;

            auto submitInfo      = VkSemaphoreSubmitInfo{};
            submitInfo.sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            submitInfo.pNext     = nullptr;
            submitInfo.semaphore = imageReadySemaphore;
            submitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR; // ConvertShaderStage(pass);
            waitSemaphores.push_back(submitInfo);
        }

        auto signalSemaphore      = VkSemaphoreSubmitInfo{};
        signalSemaphore.sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signalSemaphore.pNext     = nullptr;
        signalSemaphore.semaphore = pass.m_signalSemaphore;
        signalSemaphore.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR; // ConvertShaderStage(pass);

        auto commandSubmitInfo          = VkCommandBufferSubmitInfo{};
        commandSubmitInfo.sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandSubmitInfo.pNext         = nullptr;
        commandSubmitInfo.commandBuffer = commandList->m_commandBuffer;

        auto submitInfo                     = VkSubmitInfo2{};
        submitInfo.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.pNext                    = nullptr;
        submitInfo.flags                    = 0;
        submitInfo.waitSemaphoreInfoCount   = waitSemaphores.size();
        submitInfo.pWaitSemaphoreInfos      = waitSemaphores.data();
        submitInfo.commandBufferInfoCount   = 1;
        submitInfo.pCommandBufferInfos      = &commandSubmitInfo;
        submitInfo.signalSemaphoreInfoCount = 1;
        submitInfo.pSignalSemaphoreInfos    = &signalSemaphore;

        auto currentFrameFence = m_framesInflightFences[m_currentFrameIndex];

        auto result = vkQueueSubmit2(context->m_graphicsQueue, 1, &submitInfo, currentFrameFence);
        VULKAN_ASSERT_SUCCESS(result);
    }

    void FrameScheduler::ResetPass(RHI::Pass& passBase)
    {
        auto& pass    = static_cast<Pass&>(passBase);
        auto  context = static_cast<Context*>(m_context);
        auto  result  = vkResetFences(context->m_device, 1, &pass.m_signalFence);
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

        m_currentFrameIndex = 0;
        if (m_attachmentsRegistry->m_swapchainAttachments.empty() == false)
        {
            m_currentFrameIndex = m_attachmentsRegistry->m_swapchainAttachments.front()->swapchain->GetCurrentImageIndex();
        }

        // todo
        static uint32_t init = 0;

        if (init < 3)
        {
            init++;
            return;
        }

        auto currentFrameFence = m_framesInflightFences[m_currentFrameIndex];
        auto result            = vkWaitForFences(context->m_device, 1, &currentFrameFence, VK_TRUE, UINT64_MAX);
        result                 = vkResetFences(context->m_device, 1, &currentFrameFence);
        VULKAN_ASSERT_SUCCESS(result);
    }

} // namespace Vulkan