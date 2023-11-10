#include "FrameGraph.hpp"
#include "Allocator.hpp"
#include "CommandList.hpp"
#include "Common.hpp"
#include "Context.hpp"
#include "Resources.hpp"

#include <bitset>
#include <memory>

namespace Vulkan
{
    ///////////////////////////////////////////////////////////////////////////
    /// Utility functions /////////////////////////////////////////////////////
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
        case RHI::AttachmentUsage::RenderTarget:   return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case RHI::AttachmentUsage::Depth:          return access == RHI::AttachmentAccess::Read ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        case RHI::AttachmentUsage::Stencil:        return access == RHI::AttachmentAccess::Read ? VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        case RHI::AttachmentUsage::DepthStencil:   return access == RHI::AttachmentAccess::Read ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case RHI::AttachmentUsage::ShaderResource: return access == RHI::AttachmentAccess::Read ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        case RHI::AttachmentUsage::Copy:           return access == RHI::AttachmentAccess::Read ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        default:                                   RHI_UNREACHABLE(); return VK_IMAGE_LAYOUT_GENERAL;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// TransientResourceAllocator definition /////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    TransientResourceAllocator::~TransientResourceAllocator()
    {
        for (auto block : m_blocks)
        {
            vmaFreeMemory(m_context->m_allocator, block.allocation);
            vmaDestroyVirtualBlock(block.virtualBlock);
        }
    }

    bool TransientResourceAllocator::Activate(RHI::Handle<RHI::Image> imageHandle)
    {
        auto image = m_context->m_resourceManager->m_imageOwner.Get(imageHandle);
        RHI_ASSERT(image != nullptr);
        RHI_ASSERT(image->swapchain == nullptr);

        auto requirements = image->GetMemoryRequirements(m_context->m_device);
        if (auto allocation = Allocate(requirements); allocation.has_value())
        {
            RHI_ASSERT(allocation->type == AllocationType::Aliasing);
            auto result = vmaBindImageMemory2(m_context->m_allocator, allocation->handle, allocation->virtualAllocation.offset, image->handle, nullptr);
            return result == VK_SUCCESS;
        }

        return false;
    }

    bool TransientResourceAllocator::Activate(RHI::Handle<RHI::Buffer> bufferHandle)
    {
        auto buffer = m_context->m_resourceManager->m_bufferOwner.Get(bufferHandle);
        RHI_ASSERT(buffer);

        auto requirements = buffer->GetMemoryRequirements(m_context->m_device);
        if (auto allocation = Allocate(requirements); allocation.has_value())
        {
            RHI_ASSERT(allocation->type == AllocationType::Aliasing);
            auto result = vmaBindBufferMemory2(m_context->m_allocator, allocation->handle, allocation->virtualAllocation.offset, buffer->handle, nullptr);
            return result == VK_SUCCESS;
        }

        return false;
    }

    void TransientResourceAllocator::Shutdown(RHI::Handle<RHI::Image> handle)
    {
        auto image = m_context->m_resourceManager->m_imageOwner.Get(handle);
        RHI_ASSERT(image);

        Free(image->allocation);
    }

    void TransientResourceAllocator::Shutdown(RHI::Handle<RHI::Buffer> handle)
    {
        auto buffer = m_context->m_resourceManager->m_imageOwner.Get(handle);
        RHI_ASSERT(buffer);

        Free(buffer->allocation);
    }

    size_t TransientResourceAllocator::CalculatePreferredBlockSize(uint32_t memTypeIndex)
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

    TransientResourceAllocator::Block TransientResourceAllocator::CreateBlockNewBlock(VkMemoryRequirements minRequirements)
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

    std::optional<Allocation> TransientResourceAllocator::Allocate(VkMemoryRequirements requirements)
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

    void TransientResourceAllocator::Free(Allocation allocation)
    {
        vmaVirtualFree(allocation.virtualAllocation.blockHandle, allocation.virtualAllocation.handle);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Pass definition ///////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    Pass::Pass(Context* context)
        : RHI::Pass(static_cast<RHI::Context*>(context))
    {
    }

    Pass::~Pass()
    {
        auto context = static_cast<Context*>(m_context);

        vkDeviceWaitIdle(context->m_device);

        vkDestroyCommandPool(context->m_device, m_commandPool, nullptr);
    }

    VkResult Pass::Init(const RHI::PassCreateInfo& createInfo)
    {
        auto context = static_cast<Context*>(m_context);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        auto result = vkCreateSemaphore(context->m_device, &semaphoreInfo, nullptr, &m_signalSemaphore);
        VULKAN_RETURN_VKERR_CODE(result);

        m_commandList = std::make_unique<CommandList>(context);
        m_queuInfo.type = createInfo.type;
        m_queuInfo.id = context->GetQueueFamilyIndex(createInfo.type);

        VkCommandPoolCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;

        if (createInfo.type == RHI::QueueType::Graphics)
        {
            vkCreateInfo.queueFamilyIndex = context->m_graphicsQueueFamilyIndex;
        }
        else if (createInfo.type == RHI::QueueType::Compute)
        {
            vkCreateInfo.queueFamilyIndex = context->m_computeQueueFamilyIndex;
        }
        else if (createInfo.type == RHI::QueueType::Transfer)
        {
            vkCreateInfo.queueFamilyIndex = context->m_transferQueueFamilyIndex;
        }
        else
        {
            RHI_UNREACHABLE();
        }

        result = vkCreateCommandPool(context->m_device, &vkCreateInfo, nullptr, &m_commandPool);
        VULKAN_RETURN_VKERR_CODE(result);
        if (result != VK_SUCCESS)
            return result;

        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.commandPool = m_commandPool;
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.pNext = nullptr;
        allocateInfo.commandBufferCount = 1;

        result = m_commandList->Init(allocateInfo);
        VULKAN_RETURN_VKERR_CODE(result);
        if (result != VK_SUCCESS)
            return result;

        m_signalFence = context->m_resourceManager->CreateFence().value;

        return result;
    }

    RHI::CommandList& Pass::BeginCommandList(uint32_t commandsCount)
    {
        m_commandList->Begin();

        auto scheduler = static_cast<FrameScheduler*>(m_scheduler);
        auto queueInfo = GetQueueInfo();
        if (queueInfo.type == RHI::QueueType::Graphics)
        {
            m_commandList->RenderingBegin(*scheduler, *this);
        }

        return *m_commandList;
    }

    void Pass::EndCommandList()
    {
        auto scheduler = static_cast<FrameScheduler*>(m_scheduler);
        auto queueInfo = GetQueueInfo();
        if (queueInfo.type == RHI::QueueType::Graphics)
        {
            m_commandList->RenderingEnd(*scheduler, *this);
        }

        m_commandList->End();
    }

    void Pass::OnBegin()
    {
        auto context = static_cast<Context*>(m_context);

        if (m_signalFence)
        {
            auto fence = context->m_resourceManager->m_fenceOwner.Get(m_signalFence);
            RHI_ASSERT(fence);

            vkResetFences(context->m_device, 1, &fence->handle);
        }
    }

    void Pass::OnEnd()
    {
    }

    RHI::PassQueueState Pass::GetPassQueueStateInternal() const
    {
        auto context = static_cast<Context*>(m_context);

        return RHI::PassQueueState::NotSubmitted;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// FrameScheduler definition /////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    FrameScheduler::FrameScheduler(Context* context)
        : RHI::FrameScheduler(static_cast<RHI::Context*>(context))
    {
        m_transientAllocator = std::make_unique<TransientResourceAllocator>(context);
    }

    FrameScheduler::~FrameScheduler()
    {
    }

    VkResult FrameScheduler::Init()
    {
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

    void FrameScheduler::BeginInternal()
    {
        auto context = static_cast<Context*>(m_context);
        vkDeviceWaitIdle(context->m_device);

        for (auto passBase : m_passList)
        {
            auto pass = static_cast<Pass*>(passBase);
            if (pass->m_commandList)
            {
                vkResetCommandPool(context->m_device, pass->m_commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
            }
        }
    }

    void FrameScheduler::EndInternal()
    {
        auto context = static_cast<Context*>(m_context);
    }

    void FrameScheduler::ExecutePass(RHI::Pass* passBase)
    {
        auto context = static_cast<Context*>(m_context);
        auto pass = static_cast<Pass*>(passBase);

        auto commandList = static_cast<CommandList*>(pass->m_commandList.get());

        auto waitSemaphores = GetSemaphores(pass->m_producers);
        auto signalSemaphores = GetSemaphores(pass->m_consumers);

        VkCommandBufferSubmitInfo commandSubmitInfo{};
        commandSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandSubmitInfo.pNext = nullptr;
        commandSubmitInfo.commandBuffer = commandList->m_commandBuffer;

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.pNext = nullptr;
        submitInfo.flags = 0;
        submitInfo.waitSemaphoreInfoCount = waitSemaphores.size();
        submitInfo.pWaitSemaphoreInfos = waitSemaphores.data();
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &commandSubmitInfo;
        submitInfo.signalSemaphoreInfoCount = signalSemaphores.size();
        submitInfo.pSignalSemaphoreInfos = signalSemaphores.data();

        auto signalFence = context->m_resourceManager->m_fenceOwner.Get(pass->m_signalFence);

        auto result = vkQueueSubmit2(context->m_graphicsQueue, 1, &submitInfo, signalFence->handle);
        RHI_ASSERT(result == VK_SUCCESS);
    }

    void FrameScheduler::Allocate(RHI::Handle<RHI::Attachment> handle)
    {
        auto attachment = m_attachments.Get(handle);

        if (attachment->lifetime == RHI::AttachmentLifetime::Persistent)
            return;

        if (attachment->type == RHI::AttachmentType::Image)
        {
            m_transientAllocator->Activate(attachment->asImage.handle);
        }
        else if (attachment->type == RHI::AttachmentType::Buffer)
        {
            m_transientAllocator->Activate(attachment->asBuffer.handle);
        }
        else
        {
            RHI_UNREACHABLE();
        }
    }

    void FrameScheduler::Release(RHI::Handle<RHI::Attachment> handle)
    {
        auto attachment = m_attachments.Get(handle);

        if (attachment->type == RHI::AttachmentType::Image)
        {
            m_transientAllocator->Shutdown(attachment->asImage.handle);
        }
        else if (attachment->type == RHI::AttachmentType::Buffer)
        {
            m_transientAllocator->Shutdown(attachment->asBuffer.handle);
        }
        else
        {
            RHI_UNREACHABLE();
        }
    }

    RHI::Handle<RHI::Image> FrameScheduler::CreateTransientImageResource(const RHI::ImageCreateInfo& createInfo)
    {
        auto context = static_cast<Context*>(m_context);
        auto [image, result] = context->m_resourceManager->CreateImage({}, createInfo, nullptr, true);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return image;
    }

    RHI::Handle<RHI::Buffer> FrameScheduler::CreateTransientBufferResource(const RHI::BufferCreateInfo& createInfo)
    {
        auto context = static_cast<Context*>(m_context);
        auto [buffer, result] = context->m_resourceManager->CreateBuffer({}, createInfo, nullptr, true);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return buffer;
    }

    RHI::Handle<RHI::ImageView> FrameScheduler::CreateImageView(RHI::Attachment* attachment, const RHI::ImageAttachmentUseInfo& useInfo)
    {
        auto context = static_cast<Context*>(m_context);
        auto [view, result] = context->m_resourceManager->CreateImageView(attachment->asImage.handle, useInfo);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return view;
    }

    RHI::Handle<RHI::BufferView> FrameScheduler::CreateBufferView(RHI::Attachment* attachment, const RHI::BufferAttachmentUseInfo& useInfo)
    {
        auto context = static_cast<Context*>(m_context);
        auto [view, result] = context->m_resourceManager->CreateBufferView(attachment->asBuffer.handle, useInfo);
        RHI_ASSERT(result == RHI::ResultCode::Success);
        return view;
    }

    void FrameScheduler::FreeTransientBufferResource(RHI::Handle<RHI::Buffer> handle)
    {
        auto context = static_cast<Context*>(m_context);
        auto resource = context->m_resourceManager->m_bufferOwner.Get(handle);

        resource->Shutdown(context);

        context->m_resourceManager->m_bufferOwner.Remove(handle);
    }

    void FrameScheduler::FreeTransientImageResource(RHI::Handle<RHI::Image> handle)
    {
        auto context = static_cast<Context*>(m_context);
        auto resource = context->m_resourceManager->m_imageOwner.Get(handle);

        resource->Shutdown(context);

        context->m_resourceManager->m_imageOwner.Remove(handle);
    }

    void FrameScheduler::FreeImageView(RHI::Handle<RHI::ImageView> handle)
    {
        auto context = static_cast<Context*>(m_context);
        auto resource = context->m_resourceManager->m_imageViewOwner.Get(handle);

        resource->Shutdown(context);

        context->m_resourceManager->m_imageViewOwner.Remove(handle);
    }

    void FrameScheduler::FreeBufferView(RHI::Handle<RHI::BufferView> handle)
    {
        auto context = static_cast<Context*>(m_context);
        auto resource = context->m_resourceManager->m_bufferViewOwner.Get(handle);

        resource->Shutdown(context);

        context->m_resourceManager->m_bufferViewOwner.Remove(handle);
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

} // namespace Vulkan