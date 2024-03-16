#include "CommandList.hpp"
#include "Common.hpp"
#include "Context.hpp"
#include "FrameScheduler.hpp"
#include "Resources.hpp"
#include <RHI/Attachments.hpp>

#include <RHI/Format.hpp>

#include <Swapchain.hpp>
#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    struct TransitionInfo
    {
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 access;
        VkImageLayout layout;
    };

    struct QueueOwnershipTransferInfo
    {
        uint32_t srcQueueFamilyIndex;
        uint32_t dstQueueFamilyIndex;
    };

    inline static bool IsSwapchainAttachment(const ImagePassAttachment& passAttachment)
    {
        return passAttachment.GetAttachment()->m_swapchain != nullptr;
    }

    inline static QueueOwnershipTransferInfo GetQueueOwnershipTransferInfo(IContext* context, const PassAttachment* srcPassAttachment, const PassAttachment* dstPassAttachment)
    {
        if (srcPassAttachment == nullptr ||
            dstPassAttachment == nullptr)
        {
            return { VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED };
        }

        // NOTE: we need to insert a barrier here
        QueueOwnershipTransferInfo info{};
        info.srcQueueFamilyIndex = context->GetQueueFamilyIndex(srcPassAttachment->m_pass->GetQueueType());
        info.dstQueueFamilyIndex = context->GetQueueFamilyIndex(dstPassAttachment->m_pass->GetQueueType());

        if (info.srcQueueFamilyIndex == info.dstQueueFamilyIndex)
        {
            return { VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED };
        }

        return { VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED };
        // return info;
    }

    inline static VkImageLayout GetImageLayout(ImageUsage usage, Access access)
    {
        (void)access;
        switch (usage)
        {
        case ImageUsage::Color:           return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ImageUsage::Depth:           return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        case ImageUsage::Stencil:         return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        case ImageUsage::DepthStencil:    return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case ImageUsage::CopySrc:         return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ImageUsage::CopyDst:         return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case ImageUsage::ShaderResource:  return VK_IMAGE_LAYOUT_GENERAL;
        case ImageUsage::StorageResource: return VK_IMAGE_LAYOUT_GENERAL;
        default:
            RHI_UNREACHABLE();
            return VK_IMAGE_LAYOUT_GENERAL;
        }
    }

    inline static TransitionInfo GetTransitionToPresentInfo()
    {
        return { VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };
    }

    inline static TransitionInfo GetImageTransitionInfo(ImageUsage usage, Access access, Flags<ShaderStage> stages, LoadStoreOperations loadStoreOperations)
    {
        VkAccessFlags2 renderTargetAccessFlags = {};
        if (loadStoreOperations.loadOperation == LoadOperation::Load)
            renderTargetAccessFlags |= usage == ImageUsage::Color ? VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

        if (loadStoreOperations.storeOperation == StoreOperation::Store)
            renderTargetAccessFlags |= usage == ImageUsage::Color ? VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        switch (usage)
        {
        case ImageUsage::Color:        return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, renderTargetAccessFlags, GetImageLayout(usage, access) };
        case ImageUsage::Depth:        return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT, renderTargetAccessFlags, GetImageLayout(usage, access) };
        case ImageUsage::Stencil:      return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT, renderTargetAccessFlags, GetImageLayout(usage, access) };
        case ImageUsage::DepthStencil: return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT, renderTargetAccessFlags, GetImageLayout(usage, access) };
        case ImageUsage::CopySrc:      return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, GetImageLayout(usage, access) };
        case ImageUsage::CopyDst:      return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, GetImageLayout(usage, access) };
        case ImageUsage::ShaderResource:
            {
                VkPipelineStageFlags2 pipelineStages = {};
                VkAccessFlags2 pipelineAccess = {};
                VkImageLayout imageLayout = GetImageLayout(usage, access);

                if (stages & ShaderStage::Compute)
                {
                    return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, IsWriteAccess(access) ? VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_SHADER_READ_BIT : VK_ACCESS_2_SHADER_READ_BIT, GetImageLayout(usage, access) };
                }

                if (stages & ShaderStage::Vertex)
                {
                    pipelineStages |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
                    pipelineAccess |= VK_ACCESS_2_SHADER_READ_BIT;
                }

                if (stages & ShaderStage::Pixel)
                {
                    pipelineStages |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
                    pipelineAccess = IsWriteAccess(access) ? VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_SHADER_READ_BIT : VK_ACCESS_2_SHADER_READ_BIT;
                }

                return { pipelineStages, pipelineAccess, imageLayout };
            }
        case RHI::ImageUsage::StorageResource:
            {
                VkPipelineStageFlags2 pipelineStages = {};
                VkAccessFlags2 pipelineAccess = {};
                VkImageLayout imageLayout = GetImageLayout(usage, access);

                if (stages & ShaderStage::Compute)
                {
                    return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, IsWriteAccess(access) ? VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_SHADER_READ_BIT : VK_ACCESS_2_SHADER_READ_BIT, GetImageLayout(usage, access) };
                }

                if (stages & ShaderStage::Vertex)
                {
                    pipelineStages |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
                    pipelineAccess |= VK_ACCESS_2_SHADER_READ_BIT;
                }

                if (stages & ShaderStage::Pixel)
                {
                    pipelineStages |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
                    pipelineAccess = IsWriteAccess(access) ? VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_SHADER_READ_BIT : VK_ACCESS_2_SHADER_READ_BIT;
                }

                return { pipelineStages, pipelineAccess, imageLayout };
            }
        default: RHI_UNREACHABLE(); return {};
        }
    }

    inline static TransitionInfo GetBufferTransitionInfo(BufferUsage usage, Access access, Flags<ShaderStage> stages)
    {
        (void)stages;
        VkAccessFlags2 accessFlags = {};
        if (access == Access::Read)
            accessFlags = VK_ACCESS_2_SHADER_READ_BIT;
        else if (access == Access::Write)
            accessFlags = VK_ACCESS_2_SHADER_WRITE_BIT;
        else if (access == Access::ReadWrite)
            accessFlags = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;

        VkPipelineStageFlags2 stageFlags = {};

        if (stages & ShaderStage::Vertex)
            stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        if (stages & ShaderStage::Pixel)
            stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        if (stages & ShaderStage::Compute)
            stageFlags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

        switch (usage)
        {
        case BufferUsage::Storage: return { stageFlags, accessFlags, {} };
        case BufferUsage::Uniform: return { stageFlags, VK_ACCESS_2_SHADER_READ_BIT, {} };
        case BufferUsage::Vertex:  return { VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_ACCESS_2_SHADER_READ_BIT, {} };
        case BufferUsage::Index:   return { VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT, VK_ACCESS_2_SHADER_READ_BIT, {} };
        case BufferUsage::CopySrc: return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, {} };
        case BufferUsage::CopyDst: return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, {} };
        default:                   RHI_UNREACHABLE(); return {};
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandPool
    //////////////////////////////////////////////////////////////////////////////////////////

    ICommandListAllocator::ICommandListAllocator(IContext* context)
        : m_context(context)
    {
    }

    ICommandListAllocator::~ICommandListAllocator()
    {
        for (auto queueCommandPool : m_commandPools)
        {
            for (auto commandPool : queueCommandPool)
            {
                vkDestroyCommandPool(m_context->m_device, commandPool, nullptr);
            }
        }
    }

    VkResult ICommandListAllocator::Init()
    {
        for (uint32_t queueType = 0; queueType < uint32_t(QueueType::Count); queueType++)
        {
            m_commandPools[(queueType)].resize(2);
            for (auto& commandPool : m_commandPools[(queueType)])
            {
                
                VkCommandPoolCreateInfo createInfo;
                createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                createInfo.pNext = nullptr;
                createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                createInfo.queueFamilyIndex = m_context->GetQueueFamilyIndex(QueueType(queueType));
                auto result = vkCreateCommandPool(m_context->m_device, &createInfo, nullptr, &commandPool);
                RHI_ASSERT(result == VK_SUCCESS);
                if (result != VK_SUCCESS)
                {
                    return result;
                }
            }
        }

        return VK_SUCCESS;
    }

    void ICommandListAllocator::Reset()
    {
        for (auto queueCommandPool : m_commandPools)
        {
            for (auto commandPool : queueCommandPool)
            {
                vkTrimCommandPool(m_context->m_device, commandPool, 0);
                vkResetCommandPool(m_context->m_device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
            }
        }
    }

    CommandList* ICommandListAllocator::Allocate(QueueType queueType)
    {
        return Allocate(queueType, 1).front();
    }

    std::vector<CommandList*> ICommandListAllocator::Allocate(QueueType queueType, uint32_t count)
    {
        auto commandPool = m_commandPools[uint32_t(queueType)][0];
        auto commandBuffers = AllocateCommandBuffers(commandPool, count, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        std::vector<CommandList*> commandLists;
        commandLists.reserve(count);
        for (auto commandBuffer : commandBuffers)
        {
            commandLists.push_back(new ICommandList(m_context, commandPool, commandBuffer));
        }
        return commandLists;
    }

    void ICommandListAllocator::Release(TL::Span<CommandList*> _commandLists)
    {
        (void)_commandLists;
        // auto commandLists = TL::Span((ICommandList*)_commandLists.data(), _commandLists.size());
        // std::vector<VkCommandBuffer> commandBuffers; 
        // commandBuffers.reserve(_commandLists.size());
        // auto commandPool = commandLists[0].m_commandPool;
        // auto device = m_context->m_device;

        // for (auto& commandList : commandLists)
        // {
        //     commandBuffers.push_back(commandList.m_commandBuffer);
        // }

        // m_context->m_deferDeleteQueue.push_back([=](){
        //     vkFreeCommandBuffers(device, commandPool, uint32_t(commandBuffers.size()), commandBuffers.data());
        // });
    }

    std::vector<VkCommandBuffer> ICommandListAllocator::AllocateCommandBuffers(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level)
    {
        std::vector<VkCommandBuffer> commandBuffers;
        commandBuffers.resize(count);

        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.commandPool = pool;
        allocateInfo.level = level;
        allocateInfo.commandBufferCount = count;
        vkAllocateCommandBuffers(m_context->m_device, &allocateInfo, commandBuffers.data());
        return commandBuffers;
    }

    void ICommandListAllocator::ReleaseCommandBuffers(VkCommandPool pool, TL::Span<VkCommandBuffer> commandBuffers)
    {
        vkFreeCommandBuffers(m_context->m_device, pool, uint32_t(commandBuffers.size()), commandBuffers.data());
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandList
    //////////////////////////////////////////////////////////////////////////////////////////

    ICommandList::ICommandList(IContext* context, VkCommandPool commandPool, VkCommandBuffer commandBuffer)
        : m_commandBuffer(commandBuffer)
        , m_commandPool(commandPool)
        , m_context(context)
        , m_pass(nullptr)
    {
    }

    void ICommandList::Begin()
    {
        ZoneScoped;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    }

    void ICommandList::Begin(Pass& passBase)
    {
        ZoneScoped;

        auto& pass = static_cast<Pass&>(passBase);

        m_pass = &pass;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

        std::vector<VkImageMemoryBarrier2> imageBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferBarriers;

        for (auto& passAttachment : pass.m_imagePassAttachments)
        {
            IImage* image = m_context->m_imageOwner.Get(passAttachment->GetAttachment()->GetHandle());
            ImagePassAttachment* srcPassAttachment = passAttachment->GetPrev();
            ImagePassAttachment* dstPassAttachment = passAttachment.get();

            auto srcInfo = TransitionInfo{ VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_UNDEFINED };
            auto dstInfo = GetImageTransitionInfo(dstPassAttachment->m_usage, dstPassAttachment->m_access, dstPassAttachment->m_stage, dstPassAttachment->m_loadStoreOperations);
            if (srcPassAttachment)
                srcInfo = GetImageTransitionInfo(srcPassAttachment->m_usage, srcPassAttachment->m_access, srcPassAttachment->m_stage, dstPassAttachment->m_loadStoreOperations);

            auto [srcQueueFamily, dstQueueFamily] = GetQueueOwnershipTransferInfo(m_context, srcPassAttachment, dstPassAttachment);

            VkImageMemoryBarrier2& barrier = imageBarriers.emplace_back();
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier.pNext = nullptr;
            barrier.srcStageMask = srcInfo.stage;
            barrier.srcAccessMask = srcInfo.access;
            barrier.dstStageMask = dstInfo.stage;
            barrier.dstAccessMask = dstInfo.access;
            barrier.oldLayout = srcInfo.layout;
            barrier.newLayout = dstInfo.layout;
            barrier.image = image->handle;
            barrier.subresourceRange = ConvertSubresourceRange(passAttachment->m_viewInfo.subresource);
            barrier.srcQueueFamilyIndex = srcQueueFamily;
            barrier.dstQueueFamilyIndex = dstQueueFamily;

            // this resource is produced by another different queue, we must wait for it to be finished
            if (srcQueueFamily != dstQueueFamily && srcQueueFamily != VK_QUEUE_FAMILY_IGNORED)
            {
                auto waitSemaphore = m_waitSemaphores.emplace_back();
                waitSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                waitSemaphore.pNext = nullptr;
                waitSemaphore.stageMask = srcInfo.stage;
                waitSemaphore.semaphore = ((IFrameScheduler&)m_context->GetScheduler()).CreateTempSemaphore();
            }

            if (passAttachment->GetPrev() == nullptr && IsSwapchainAttachment(*passAttachment))
            {
                auto swapchain = (ISwapchain*)passAttachment->GetAttachment()->m_swapchain;
                auto& waitSemaphore = m_waitSemaphores.emplace_back();
                waitSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                waitSemaphore.semaphore = swapchain->GetImageReadySemaphore();
                waitSemaphore.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            }
        }

        for (auto& passAttachment : pass.m_bufferPassAttachments)
        {
            IBuffer* buffer = m_context->m_bufferOwner.Get(passAttachment->GetAttachment()->GetHandle());
            BufferPassAttachment* srcPassAttachment = passAttachment->GetPrev();
            BufferPassAttachment* dstPassAttachment = passAttachment.get();

            auto srcInfo = TransitionInfo{ VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_UNDEFINED };
            auto dstInfo = GetBufferTransitionInfo(dstPassAttachment->m_usage, dstPassAttachment->m_access, dstPassAttachment->m_stage);
            if (srcPassAttachment)
                srcInfo = GetBufferTransitionInfo(srcPassAttachment->m_usage, srcPassAttachment->m_access, srcPassAttachment->m_stage);

            auto [srcQueueFamily, dstQueueFamily] = GetQueueOwnershipTransferInfo(m_context, srcPassAttachment, dstPassAttachment);

            VkBufferMemoryBarrier2& barrier = bufferBarriers.emplace_back();
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier.pNext = nullptr;
            barrier.srcStageMask = srcInfo.stage;
            barrier.srcAccessMask = srcInfo.access;
            barrier.dstStageMask = dstInfo.stage;
            barrier.dstAccessMask = dstInfo.access;
            barrier.buffer = buffer->handle;
            barrier.offset = passAttachment->m_viewInfo.byteOffset;
            barrier.size = passAttachment->m_viewInfo.byteSize;
            barrier.srcQueueFamilyIndex = srcQueueFamily;
            barrier.dstQueueFamilyIndex = dstQueueFamily;

            if (srcQueueFamily != dstQueueFamily && srcQueueFamily != VK_QUEUE_FAMILY_IGNORED)
            {
                auto waitSemaphore = m_waitSemaphores.emplace_back();
                waitSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                waitSemaphore.pNext = nullptr;
                waitSemaphore.stageMask = srcInfo.stage;
                waitSemaphore.semaphore = ((IFrameScheduler&)m_context->GetScheduler()).CreateTempSemaphore();
            }
        }

        PipelineBarrier(bufferBarriers, imageBarriers);

        if (m_pass && m_pass->GetQueueType() == QueueType::Graphics)
        {
            RenderingBegin(pass);
        }
    }

    void ICommandList::End()
    {
        ZoneScoped;

        if (m_pass && m_pass->GetQueueType() == QueueType::Graphics)
        {
            RenderingEnd(*m_pass);
        }

        std::vector<VkImageMemoryBarrier2> imageBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferBarriers;

        if (m_pass)
        {
            for (auto& passAttachment : m_pass->m_imagePassAttachments)
            {
                IImage* image = m_context->m_imageOwner.Get(passAttachment->GetAttachment()->GetHandle());
                ImagePassAttachment* srcPassAttachment = passAttachment.get();
                ImagePassAttachment* dstPassAttachment = passAttachment->GetNext();

                auto dstInfo = TransitionInfo{ VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_UNDEFINED };
                auto srcInfo = GetImageTransitionInfo(srcPassAttachment->m_usage, srcPassAttachment->m_access, srcPassAttachment->m_stage, srcPassAttachment->m_loadStoreOperations);
                if (dstPassAttachment)
                {
                    dstInfo = GetImageTransitionInfo(dstPassAttachment->m_usage, dstPassAttachment->m_access, dstPassAttachment->m_stage, dstPassAttachment->m_loadStoreOperations);
                }
                else if (IsSwapchainAttachment(*passAttachment.get()))
                {
                    dstInfo = GetTransitionToPresentInfo();
                    auto& signalSemaphore = m_signalSemaphores.emplace_back();
                    auto swapchain = (ISwapchain*)passAttachment->GetAttachment()->m_swapchain;
                    signalSemaphore.semaphore = swapchain->GetFrameReadySemaphore();
                    signalSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                    signalSemaphore.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
                }
                else
                {
                    continue;
                }

                auto [srcQueueFamily, dstQueueFamily] = GetQueueOwnershipTransferInfo(m_context, srcPassAttachment, dstPassAttachment);

                VkImageMemoryBarrier2& barrier = imageBarriers.emplace_back();
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                barrier.pNext = nullptr;
                barrier.srcStageMask = srcInfo.stage;
                barrier.srcAccessMask = srcInfo.access;
                barrier.dstStageMask = dstInfo.stage;
                barrier.dstAccessMask = dstInfo.access;
                barrier.oldLayout = srcInfo.layout;
                barrier.newLayout = dstInfo.layout;
                barrier.image = image->handle;
                barrier.subresourceRange = ConvertSubresourceRange(passAttachment->m_viewInfo.subresource);
                barrier.srcQueueFamilyIndex = srcQueueFamily;
                barrier.dstQueueFamilyIndex = dstQueueFamily;

                // this resource is produced by another different queue, we must wait for it to be finished
                if (srcQueueFamily != dstQueueFamily && srcQueueFamily != VK_QUEUE_FAMILY_IGNORED)
                {
                    auto signalSemaphore = m_signalSemaphores.emplace_back();
                    signalSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                    signalSemaphore.pNext = nullptr;
                    signalSemaphore.stageMask = srcInfo.stage;
                    signalSemaphore.semaphore = ((IFrameScheduler&)m_context->GetScheduler()).CreateTempSemaphore();
                }
            }

            for (auto& passAttachment : m_pass->m_bufferPassAttachments)
            {
                IBuffer* buffer = m_context->m_bufferOwner.Get(passAttachment->GetAttachment()->GetHandle());
                BufferPassAttachment* srcPassAttachment = passAttachment.get();
                BufferPassAttachment* dstPassAttachment = passAttachment->GetNext();

                auto srcInfo = TransitionInfo{ VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_UNDEFINED };
                auto dstInfo = GetBufferTransitionInfo(dstPassAttachment->m_usage, dstPassAttachment->m_access, dstPassAttachment->m_stage);
                if (srcPassAttachment)
                    srcInfo = GetBufferTransitionInfo(srcPassAttachment->m_usage, srcPassAttachment->m_access, srcPassAttachment->m_stage);

                auto [srcQueueFamily, dstQueueFamily] = GetQueueOwnershipTransferInfo(m_context, srcPassAttachment, dstPassAttachment);

                VkBufferMemoryBarrier2& barrier = bufferBarriers.emplace_back();
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                barrier.pNext = nullptr;
                barrier.srcStageMask = srcInfo.stage;
                barrier.srcAccessMask = srcInfo.access;
                barrier.dstStageMask = dstInfo.stage;
                barrier.dstAccessMask = dstInfo.access;
                barrier.buffer = buffer->handle;
                barrier.offset = passAttachment->m_viewInfo.byteOffset;
                barrier.size = passAttachment->m_viewInfo.byteSize;
                barrier.srcQueueFamilyIndex = srcQueueFamily;
                barrier.dstQueueFamilyIndex = dstQueueFamily;

                if (srcQueueFamily != dstQueueFamily && srcQueueFamily != VK_QUEUE_FAMILY_IGNORED)
                {
                    auto signalSemaphore = m_signalSemaphores.emplace_back();
                    signalSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                    signalSemaphore.pNext = nullptr;
                    signalSemaphore.stageMask = srcInfo.stage;
                    signalSemaphore.semaphore = ((IFrameScheduler&)m_context->GetScheduler()).CreateTempSemaphore();
                }
            }

            PipelineBarrier(bufferBarriers, imageBarriers);
        }

        vkEndCommandBuffer(m_commandBuffer);
    }

    void ICommandList::SetViewport(const Viewport& viewport)
    {
        ZoneScoped;

        VkViewport vkViewport{};
        vkViewport.x = viewport.offsetX;
        vkViewport.y = viewport.offsetY;
        vkViewport.width = viewport.width;
        vkViewport.height = viewport.height;
        vkViewport.minDepth = viewport.minDepth;
        vkViewport.maxDepth = viewport.maxDepth;
        vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
    }

    void ICommandList::SetSicssor(const Scissor& scissor)
    {
        ZoneScoped;

        VkRect2D vkScissor{};
        vkScissor.extent.width = scissor.width;
        vkScissor.extent.height = scissor.height;
        vkScissor.offset.x = scissor.offsetX;
        vkScissor.offset.y = scissor.offsetY;
        vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
    }

    void ICommandList::Draw(const DrawInfo& command)
    {
        ZoneScoped;

        auto pipeline = m_context->m_graphicsPipelineOwner.Get(command.pipelineState);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);

        if (command.bindGroups.empty() == false)
        {
            BindShaderBindGroups(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, command.bindGroups);
        }

        if (command.vertexBuffers.empty() == false)
        {
            uint32_t vertexBuffersCount = 0;
            VkBuffer vertexBuffers[c_MaxPipelineVertexBindings] = {};
            VkDeviceSize vertexBufferOffsets[c_MaxPipelineVertexBindings] = {};
            for (auto handle : command.vertexBuffers)
            {
                auto buffer = m_context->m_bufferOwner.Get(handle);
                auto index = vertexBuffersCount++;
                vertexBuffers[index] = buffer->handle;
                vertexBufferOffsets[index] = 0;
            }
            vkCmdBindVertexBuffers(m_commandBuffer, 0, vertexBuffersCount, vertexBuffers, vertexBufferOffsets);
        }

        auto parameters = command.parameters;
        if (command.indexBuffers)
        {
            auto buffer = m_context->m_bufferOwner.Get(command.indexBuffers);
            vkCmdBindIndexBuffer(m_commandBuffer, buffer->handle, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(m_commandBuffer, parameters.elementCount, parameters.instanceCount, parameters.firstElement, int32_t(parameters.vertexOffset), uint32_t(parameters.firstInstance));
        }
        else
        {
            vkCmdDraw(m_commandBuffer, parameters.elementCount, parameters.instanceCount, parameters.firstElement, uint32_t(parameters.firstInstance));
        }
    }

    void ICommandList::Dispatch(const DispatchInfo& command)
    {
        ZoneScoped;

        auto pipeline = m_context->m_computePipelineOwner.Get(command.pipelineState);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle);
        if (command.bindGroups.size())
        {
            BindShaderBindGroups(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->layout, command.bindGroups);
        }
        auto parameters = command.parameters;
        vkCmdDispatchBase(m_commandBuffer, parameters.offsetX, parameters.offsetY, parameters.offsetZ, parameters.countX, parameters.countY, parameters.countZ);
    }

    void ICommandList::Copy(const BufferCopyInfo& command)
    {
        ZoneScoped;

        auto srcBuffer = m_context->m_bufferOwner.Get(command.srcBuffer);
        auto destinationBuffer = m_context->m_bufferOwner.Get(command.dstBuffer);

        auto copyInfo = VkBufferCopy{};
        copyInfo.srcOffset = command.srcOffset;
        copyInfo.dstOffset = command.dstOffset;
        copyInfo.size = command.size;
        vkCmdCopyBuffer(m_commandBuffer, srcBuffer->handle, destinationBuffer->handle, 1, &copyInfo);
    }

    void ICommandList::Copy(const ImageCopyInfo& command)
    {
        ZoneScoped;

        auto srcImage = m_context->m_imageOwner.Get(command.srcImage);
        auto dstImage = m_context->m_imageOwner.Get(command.dstImage);

        auto copyInfo = VkImageCopy{};
        copyInfo.srcSubresource = ConvertSubresourceLayer(command.srcSubresource);
        copyInfo.srcOffset = ConvertOffset3D(command.srcOffset);
        copyInfo.dstSubresource = ConvertSubresourceLayer(command.dstSubresource);
        copyInfo.dstOffset = ConvertOffset3D(command.dstOffset);
        copyInfo.extent = ConvertExtent3D(command.srcSize);
        vkCmdCopyImage(m_commandBuffer, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);
    }

    void ICommandList::Copy(const BufferToImageCopyInfo& command)
    {
        ZoneScoped;

        auto srcBuffer = m_context->m_bufferOwner.Get(command.srcBuffer);
        auto dstImage = m_context->m_imageOwner.Get(command.dstImage);

        VkImageMemoryBarrier2 transitionImage{};
        transitionImage.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        transitionImage.pNext = nullptr;
        transitionImage.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        transitionImage.srcAccessMask = 0;
        transitionImage.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        transitionImage.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        transitionImage.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        transitionImage.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        transitionImage.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        transitionImage.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        transitionImage.image = dstImage->handle;
        auto layer = ConvertSubresourceLayer(command.dstSubresource);
        transitionImage.subresourceRange.aspectMask = layer.aspectMask;
        transitionImage.subresourceRange.levelCount = 1;
        transitionImage.subresourceRange.layerCount = 1;

        VkDependencyInfo barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        barrier.imageMemoryBarrierCount = 1;
        barrier.pImageMemoryBarriers = &transitionImage;

        vkCmdPipelineBarrier2(m_commandBuffer, &barrier);

        auto copyInfo = VkBufferImageCopy{};
        copyInfo.bufferOffset = command.srcOffset;
        copyInfo.bufferRowLength = command.srcBytesPerRow;
        // copyInfo.bufferImageHeight;
        copyInfo.imageSubresource = ConvertSubresourceLayer(command.dstSubresource);
        copyInfo.imageOffset = ConvertOffset3D(command.dstOffset);
        copyInfo.imageExtent = ConvertExtent3D(command.srcSize);
        vkCmdCopyBufferToImage(m_commandBuffer, srcBuffer->handle, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

        transitionImage.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        transitionImage.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        transitionImage.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        transitionImage.dstAccessMask = 0;
        transitionImage.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        transitionImage.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkCmdPipelineBarrier2(m_commandBuffer, &barrier);
    }

    void ICommandList::Copy(const ImageToBufferCopyInfo& command)
    {
        ZoneScoped;

        auto srcImage = m_context->m_imageOwner.Get(command.srcImage);
        auto dstBuffer = m_context->m_bufferOwner.Get(command.dstBuffer);

        auto copyInfo = VkBufferImageCopy{};
        copyInfo.bufferOffset = command.dstOffset;
        copyInfo.bufferRowLength = command.dstBytesPerRow;
        // copyInfo.bufferImageHeight;
        copyInfo.imageSubresource = ConvertSubresourceLayer(command.srcSubresource);
        copyInfo.imageOffset = ConvertOffset3D(command.srcOffset);
        copyInfo.imageExtent = ConvertExtent3D(command.srcSize);
        vkCmdCopyImageToBuffer(m_commandBuffer, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstBuffer->handle, 1, &copyInfo);
    }

    void ICommandList::DebugMarkerPush(const char* name, const ColorValue& color)
    {
        ZoneScoped;

        (void)name;
#if RHI_DEBUG
        if (m_context->m_vkCmdDebugMarkerBeginEXT)
        {
            VkDebugMarkerMarkerInfoEXT info{};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
            info.pNext = nullptr;
            info.pMarkerName = name;
            info.color[0] = color.r;
            info.color[1] = color.g;
            info.color[2] = color.b;
            info.color[3] = color.a;
            m_context->m_vkCmdDebugMarkerBeginEXT(m_commandBuffer, &info);
        }
#endif
    }

    void ICommandList::DebugMarkerPop()
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_context->m_vkCmdDebugMarkerEndEXT)
        {
            m_context->m_vkCmdDebugMarkerEndEXT(m_commandBuffer);
        }
#endif
    }

    void ICommandList::BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<Handle<BindGroup>> bindGroups)
    {
        uint32_t count = 0;
        VkDescriptorSet descriptorSets[c_MaxPipelineBindGroupsCount] = {};
        for (auto bindGroupHandle : bindGroups)
        {
            auto bindGroup = m_context->m_bindGroupOwner.Get(bindGroupHandle);
            descriptorSets[count++] = bindGroup->descriptorSet;
        }
        vkCmdBindDescriptorSets(m_commandBuffer, bindPoint, pipelineLayout, 0, count, descriptorSets, 0, nullptr);
    }

    void ICommandList::RenderingBegin(Pass& pass)
    {
        ZoneScoped;

        std::vector<VkRenderingAttachmentInfo> attachmentInfos{};
        VkRenderingAttachmentInfo depthAttachment{};
        bool hasDepthAttachment = false;

        for (auto passAttachment : pass.GetColorAttachments())
        {
            auto view = m_context->m_imageViewOwner.Get(passAttachment->m_view);

            auto& attachmentInfo = attachmentInfos.emplace_back();
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.pNext = nullptr;
            attachmentInfo.imageView = view->handle;
            attachmentInfo.imageLayout = GetImageLayout(passAttachment->m_usage, Access::None);
            attachmentInfo.loadOp = ConvertLoadOp(passAttachment->m_loadStoreOperations.loadOperation);
            attachmentInfo.storeOp = ConvertStoreOp(passAttachment->m_loadStoreOperations.storeOperation);
            attachmentInfo.clearValue.color = ConvertColorValue(passAttachment->m_clearValue.colorValue);
        }

        if (auto passAttachment = pass.GetDepthStencilAttachment(); passAttachment != nullptr)
        {
            auto view = m_context->m_imageViewOwner.Get(passAttachment->m_view);

            depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depthAttachment.pNext = nullptr;
            depthAttachment.imageView = view->handle;
            depthAttachment.imageLayout = GetImageLayout(passAttachment->m_usage, Access::None);
            depthAttachment.loadOp = ConvertLoadOp(passAttachment->m_loadStoreOperations.loadOperation);
            depthAttachment.storeOp = ConvertStoreOp(passAttachment->m_loadStoreOperations.storeOperation);
            depthAttachment.clearValue.depthStencil = ConvertDepthStencilValue(passAttachment->m_clearValue.depthStencilValue);
            hasDepthAttachment = true;
        }

        VkRenderingInfo renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.pNext = nullptr;
        renderingInfo.flags = 0;
        renderingInfo.renderArea.extent = ConvertExtent2D(pass.m_frameSize);
        renderingInfo.renderArea.offset = ConvertOffset2D({ 0u, 0u, 0u });
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = uint32_t(attachmentInfos.size());
        renderingInfo.pColorAttachments = attachmentInfos.data();
        renderingInfo.pDepthAttachment = hasDepthAttachment ? &depthAttachment : nullptr;
        vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
    }

    void ICommandList::RenderingEnd(Pass& pass)
    {
        ZoneScoped;
        (void)pass;
        vkCmdEndRendering(m_commandBuffer);
    }

    void ICommandList::PipelineBarrier(TL::Span<VkMemoryBarrier2> memoryBarriers, TL::Span<VkBufferMemoryBarrier2> bufferBarriers, TL::Span<VkImageMemoryBarrier2> imageBarriers)
    {
        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.pNext = nullptr;
        dependencyInfo.dependencyFlags = 0;
        dependencyInfo.memoryBarrierCount = uint32_t(memoryBarriers.size());
        dependencyInfo.pMemoryBarriers = memoryBarriers.data();
        dependencyInfo.bufferMemoryBarrierCount = uint32_t(bufferBarriers.size());
        dependencyInfo.pBufferMemoryBarriers = bufferBarriers.data();
        dependencyInfo.imageMemoryBarrierCount = uint32_t(imageBarriers.size());
        dependencyInfo.pImageMemoryBarriers = imageBarriers.data();
        vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
    }

} // namespace RHI::Vulkan
