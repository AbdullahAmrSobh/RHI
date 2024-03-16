#pragma once

#include "Pass.hpp"
#include "Context.hpp"
#include "Swapchain.hpp"
#include "Common.hpp"
#include "CommandList.hpp"
#include "Resources.hpp"

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

    inline static TransitionInfo
    GetImageTransitionInfo(ImageUsage usage,
                           Access access,
                           Flags<ShaderStage> stages,
                           LoadStoreOperations loadStoreOperations)
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

    void IPass::Begin()
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

        std::vector<VkImageMemoryBarrier2> imageBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferBarriers;

        for (auto& passAttachment : m_imagePassAttachments)
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
                waitSemaphore.semaphore = RequestNewSemaphore();
            }

            if (passAttachment->GetPrev() == nullptr && IsSwapchainAttachment(*passAttachment))
            {
                auto swapchain = (ISwapchain*)passAttachment->GetAttachment()->m_swapchain;
                auto& waitSemaphore = m_waitSemaphores.emplace_back();
                waitSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                waitSemaphore.semaphore = swapchain->m_semaphores.imageAcquired;
                waitSemaphore.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            }
        }

        for (auto& passAttachment : m_bufferPassAttachments)
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
                waitSemaphore.semaphore = RequestNewSemaphore();
            }
        }

        PipelineBarrier({}, bufferBarriers, imageBarriers);

        if (m_queueType == QueueType::Graphics)
        {
            std::vector<VkRenderingAttachmentInfo> attachmentInfos{};
            VkRenderingAttachmentInfo depthAttachment{};

            for (auto passAttachment : m_colorAttachments)
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

            if (auto passAttachment = m_depthStencilAttachment; passAttachment != nullptr)
            {
                auto view = m_context->m_imageViewOwner.Get(passAttachment->m_view);

                depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                depthAttachment.pNext = nullptr;
                depthAttachment.imageView = view->handle;
                depthAttachment.imageLayout = GetImageLayout(passAttachment->m_usage, Access::None);
                depthAttachment.loadOp = ConvertLoadOp(passAttachment->m_loadStoreOperations.loadOperation);
                depthAttachment.storeOp = ConvertStoreOp(passAttachment->m_loadStoreOperations.storeOperation);
                depthAttachment.clearValue.depthStencil = ConvertDepthStencilValue(passAttachment->m_clearValue.depthStencilValue);
            }

            VkRenderingInfo renderingInfo = {};
            renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
            renderingInfo.pNext = nullptr;
            renderingInfo.flags = 0;
            renderingInfo.renderArea.extent = ConvertExtent2D(m_frameSize);
            renderingInfo.renderArea.offset = ConvertOffset2D({ 0u, 0u, 0u });
            renderingInfo.layerCount = 1;
            renderingInfo.colorAttachmentCount = uint32_t(attachmentInfos.size());
            renderingInfo.pColorAttachments = attachmentInfos.data();
            renderingInfo.pDepthAttachment = m_depthStencilAttachment ? &depthAttachment : nullptr;
            vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
        }
    }

    void IPass::End()
    {
        if (m_queueType == QueueType::Graphics)
        {
            vkCmdEndRendering(m_commandBuffer);
        }

        std::vector<VkImageMemoryBarrier2> imageBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferBarriers;

        for (auto& passAttachment : m_imagePassAttachments)
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
                signalSemaphore.semaphore = swapchain->GetImageRenderCompleteSemaphore();
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
                signalSemaphore.semaphore = RequestNewSemaphore();
            }
        }

        for (auto& passAttachment : m_bufferPassAttachments)
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
                signalSemaphore.semaphore = RequestNewSemaphore();
            }
        }

        PipelineBarrier({}, bufferBarriers, imageBarriers);

        vkEndCommandBuffer(m_commandBuffer);
    }

    void IPass::Submit(TL::Span<const CommandList*> commandLists)
    {
        std::vector<VkCommandBuffer> commandBuffers;
        commandBuffers.reserve(commandLists.size());
        for (auto _commandList : commandLists)
        {
            auto commandList = (ICommandList*)_commandList;
            commandBuffers.push_back(commandList->GetHandle());
        }
        vkCmdExecuteCommands(m_commandBuffer, uint32_t(commandBuffers.size()), commandBuffers.data());
    }

    void IPass::PipelineBarrier(TL::Span<VkMemoryBarrier2> memoryBarriers,
                                TL::Span<VkBufferMemoryBarrier2> bufferBarriers,
                                TL::Span<VkImageMemoryBarrier2> imageBarriers)
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