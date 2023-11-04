#include "CommandList.hpp"
#include "Context.hpp"
#include "FrameGraph.hpp"
#include "Resources.hpp"

namespace Vulkan
{

    inline static VkPipelineStageFlags2 ConvertToVkPipelineStage(RHI::AttachmentUsage usage, RHI::Flags<RHI::PipelineAccessStage> accessStage)
    {
        switch (usage)
        {
        case RHI::AttachmentUsage::None:              RHI_UNREACHABLE(); return VK_PIPELINE_STAGE_2_NONE;
        case RHI::AttachmentUsage::VertexInputBuffer: return VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
        case RHI::AttachmentUsage::RenderTarget:      return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        case RHI::AttachmentUsage::Depth:             return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
        case RHI::AttachmentUsage::Stencil:           return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
        case RHI::AttachmentUsage::DepthStencil:      return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
        case RHI::AttachmentUsage::ShaderStorage:
        case RHI::AttachmentUsage::ShaderResource:
            {
                auto result = 0;
                if (accessStage & RHI::PipelineAccessStage::Vertex)
                    result |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;

                if (accessStage & RHI::PipelineAccessStage::Pixel)
                    result |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

                if (accessStage & RHI::PipelineAccessStage::Pixel)
                    result |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

                return result;
            }
        case RHI::AttachmentUsage::Copy:    return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        case RHI::AttachmentUsage::Resolve: return VK_PIPELINE_STAGE_2_RESOLVE_BIT;
        }
    }

    inline static VkAccessFlags2 ConvertToVkAccessFlags(RHI::AttachmentUsage usage, RHI::AttachmentAccess access)
    {
        switch (usage)
        {
        case RHI::AttachmentUsage::None:              RHI_UNREACHABLE(); return VK_ACCESS_FLAG_BITS_MAX_ENUM;
        case RHI::AttachmentUsage::VertexInputBuffer: return VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
        case RHI::AttachmentUsage::RenderTarget:      return VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        case RHI::AttachmentUsage::Depth:             return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case RHI::AttachmentUsage::Stencil:           return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case RHI::AttachmentUsage::DepthStencil:      return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case RHI::AttachmentUsage::ShaderStorage:     return access == RHI::AttachmentAccess::Read ? VK_ACCESS_2_SHADER_STORAGE_READ_BIT : VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        case RHI::AttachmentUsage::ShaderResource:    return access == RHI::AttachmentAccess::Read ? VK_ACCESS_2_SHADER_READ_BIT : VK_ACCESS_2_SHADER_WRITE_BIT;
        case RHI::AttachmentUsage::Copy:              return access == RHI::AttachmentAccess::Read ? VK_ACCESS_2_TRANSFER_READ_BIT : VK_ACCESS_2_TRANSFER_WRITE_BIT;
        case RHI::AttachmentUsage::Resolve:           return VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_READ_BIT;
        }
    }

    CommandList::~CommandList()
    {
        vkFreeCommandBuffers(m_context->m_device, m_commandPool, 1, &m_commandBuffer);
    }

    VkResult CommandList::Init(const VkCommandBufferAllocateInfo& allocateInfo)
    {
        m_commandPool = allocateInfo.commandPool;
        return vkAllocateCommandBuffers(m_context->m_device, &allocateInfo, &m_commandBuffer);
    }

    void CommandList::Begin()
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    }

    void CommandList::End()
    {
        vkEndCommandBuffer(m_commandBuffer);
    }

    VkImageMemoryBarrier2 CommandList::TransitionResource(FrameScheduler* scheduler, RHI::ImagePassAttachment* resourceBefore, RHI::ImagePassAttachment* resourceAfter)
    {
        RHI_ASSERT(resourceBefore || resourceAfter);

        auto passAttachment = resourceBefore ? resourceBefore : resourceAfter;
        auto attachment = scheduler->GetAttachment(passAttachment->attachment);
        auto image = m_context->m_resourceManager->m_imageOwner.Get(attachment->asImage.handle);

        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.pNext = nullptr;
        barrier.image = image->handle;
        barrier.subresourceRange.aspectMask = ConvertToVkImageAspect(passAttachment->info.subresource.imageAspects);
        barrier.subresourceRange.baseArrayLayer = passAttachment->info.subresource.arrayBase;
        barrier.subresourceRange.layerCount = passAttachment->info.subresource.arrayCount;
        barrier.subresourceRange.baseMipLevel = passAttachment->info.subresource.mipBase;
        barrier.subresourceRange.levelCount = passAttachment->info.subresource.mipCount;

        auto queueFamilyIndex = m_context->GetQueueFamilyIndex(passAttachment->pass->GetQueueInfo().type);
        barrier.srcQueueFamilyIndex = queueFamilyIndex;
        barrier.dstQueueFamilyIndex = queueFamilyIndex;

        if (resourceBefore == nullptr)
        {
            barrier.srcStageMask = ConvertToVkPipelineStage(resourceAfter->info.usage, resourceAfter->stages);
            barrier.dstStageMask = barrier.srcStageMask;

            barrier.srcAccessMask = ConvertToVkAccessFlags(resourceAfter->info.usage, resourceAfter->info.access);
            barrier.dstAccessMask = barrier.srcAccessMask;

            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = ConvertToVkImageLayout(resourceAfter->info.usage, resourceAfter->info.access);
        }
        else if (resourceAfter == nullptr)
        {
            barrier.srcStageMask = ConvertToVkPipelineStage(resourceBefore->info.usage, resourceBefore->stages);
            barrier.dstStageMask = barrier.srcStageMask;

            barrier.srcAccessMask = ConvertToVkAccessFlags(resourceBefore->info.usage, resourceBefore->info.access);
            barrier.dstAccessMask = barrier.srcAccessMask;

            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = ConvertToVkImageLayout(resourceBefore->info.usage, resourceBefore->info.access);

            if (image->swapchain)
            {
                barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }
        }
        else
        {
            auto nextPass = resourceAfter->pass;
            auto currentPass = resourceAfter->pass;

            barrier.srcStageMask = ConvertToVkPipelineStage(passAttachment->info.usage, passAttachment->stages);
            barrier.dstStageMask = ConvertToVkPipelineStage(passAttachment->next->info.usage, passAttachment->next->stages);
            barrier.srcAccessMask = ConvertToVkAccessFlags(passAttachment->info.usage, passAttachment->info.access);
            barrier.dstAccessMask = ConvertToVkAccessFlags(passAttachment->next->info.usage, passAttachment->next->info.access);
            barrier.oldLayout = ConvertToVkImageLayout(passAttachment->info.usage, passAttachment->info.access);
            barrier.newLayout = ConvertToVkImageLayout(passAttachment->next->info.usage, passAttachment->next->info.access);

            barrier.srcQueueFamilyIndex = m_context->GetQueueFamilyIndex(currentPass->GetQueueInfo().type);
            barrier.dstQueueFamilyIndex = m_context->GetQueueFamilyIndex(nextPass->GetQueueInfo().type);
        }

        return barrier;
    }

    VkBufferMemoryBarrier2 CommandList::TransitionResource(FrameScheduler* scheduler, RHI::BufferPassAttachment* resourceBefore, RHI::BufferPassAttachment* resourceAfter)
    {
        VkBufferMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        barrier.pNext = nullptr;
        // barrier.srcStageMask;
        // barrier.srcAccessMask;
        // barrier.dstStageMask;
        // barrier.dstAccessMask;
        // barrier.srcQueueFamilyIndex;
        // barrier.dstQueueFamilyIndex;
        // barrier.buffer;
        // barrier.offset;
        // barrier.size;
        return barrier;
    }

    void CommandList::TransitionPassAttachments(FrameScheduler* scheduler, RHI::TL::Span<RHI::ImagePassAttachment*> passAttachments)
    {
        auto resourceManager = m_context->m_resourceManager.get();

        std::vector<VkImageMemoryBarrier2> barriers;

        for (auto passAttachment : passAttachments)
        {
            auto barrier = TransitionResource(scheduler, passAttachment, passAttachment->next);
            barriers.push_back(barrier);
        }

        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.pNext = nullptr;
        // dependencyInfo.dependencyFlags;
        // dependencyInfo.memoryBarrierCount;
        // dependencyInfo.pMemoryBarriers;
        dependencyInfo.imageMemoryBarrierCount = barriers.size();
        dependencyInfo.pImageMemoryBarriers = barriers.data();

        vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
    }

    void CommandList::TransitionPassAttachments(FrameScheduler* scheduler, RHI::TL::Span<RHI::BufferPassAttachment*> passAttachments)
    {
        auto resourceManager = m_context->m_resourceManager.get();

        std::vector<VkBufferMemoryBarrier2> barriers;

        for (auto passAttachment : passAttachments)
        {
            auto barrier = TransitionResource(scheduler, passAttachment, passAttachment->next);
            barriers.push_back(barrier);
        }

        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.pNext = nullptr;
        // dependencyInfo.dependencyFlags;
        // dependencyInfo.memoryBarrierCount;
        // dependencyInfo.pMemoryBarriers;
        dependencyInfo.bufferMemoryBarrierCount = barriers.size();
        dependencyInfo.pBufferMemoryBarriers = barriers.data();

        vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
    }

    void CommandList::RenderingBegin(FrameScheduler& scheduler, Pass& pass)
    {
        auto resourceManager = m_context->m_resourceManager.get();

        uint32_t width = 0, height = 0;

        std::vector<RHI::ImagePassAttachment*> passAttachments;
        for (auto& attachment : pass.m_imagePassAttachments)
        {
            passAttachments.push_back(&attachment);
        }
        TransitionPassAttachments(&scheduler, passAttachments);

        uint32_t colorAttachmentsCount;
        std::vector<VkRenderingAttachmentInfo> colorAttachmentInfo;
        VkRenderingInfo renderingInfo{};

        for (const auto& attachment : pass.m_imagePassAttachments)
        {
            auto view = resourceManager->m_imageViewOwner.Get(attachment.view);
            auto frameAttachment = scheduler.GetAttachment(attachment.attachment);
            auto image = m_context->m_resourceManager->m_imageOwner.Get(frameAttachment->asImage.handle);

            width = std::max(image->createInfo.extent.width, width);
            height = std::max(image->createInfo.extent.height, height);

            if (attachment.info.usage == RHI::AttachmentUsage::Depth)
            {
                VkRenderingAttachmentInfo depthAttachmentInfo{};
                depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                depthAttachmentInfo.pNext = nullptr;
                depthAttachmentInfo.imageView = view->handle;
                depthAttachmentInfo.imageLayout = ConvertToVkImageLayout(attachment.info.usage, attachment.info.access);
                depthAttachmentInfo.loadOp = ConvertToVkLoadOp(attachment.info.loadStoreOperations.loadOperation);
                depthAttachmentInfo.storeOp = ConvertToVkStoreOp(attachment.info.loadStoreOperations.storeOperation);
                depthAttachmentInfo.clearValue.depthStencil.depth = attachment.info.clearValue.depth.depthValue;
                renderingInfo.pDepthAttachment = &depthAttachmentInfo;
            }

            if (attachment.info.usage == RHI::AttachmentUsage::Stencil)
            {
                VkRenderingAttachmentInfo stencilAttachmentInfo{};
                stencilAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                stencilAttachmentInfo.pNext = nullptr;
                stencilAttachmentInfo.imageView = view->handle;
                stencilAttachmentInfo.imageLayout = ConvertToVkImageLayout(attachment.info.usage, attachment.info.access);
                stencilAttachmentInfo.loadOp = ConvertToVkLoadOp(attachment.info.loadStoreOperations.loadOperation);
                stencilAttachmentInfo.storeOp = ConvertToVkStoreOp(attachment.info.loadStoreOperations.storeOperation);
                stencilAttachmentInfo.clearValue.depthStencil.stencil = attachment.info.clearValue.depth.stencilValue;
                renderingInfo.pStencilAttachment = &stencilAttachmentInfo;
            }

            if ((attachment.info.usage == RHI::AttachmentUsage::Depth) || (attachment.info.usage == RHI::AttachmentUsage::Stencil))
            {
                continue;
            }

            VkRenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.pNext = nullptr;
            attachmentInfo.imageView = view->handle;
            attachmentInfo.imageLayout = ConvertToVkImageLayout(attachment.info.usage, attachment.info.access);
            attachmentInfo.loadOp = ConvertToVkLoadOp(attachment.info.loadStoreOperations.loadOperation);
            attachmentInfo.storeOp = ConvertToVkStoreOp(attachment.info.loadStoreOperations.storeOperation);
            attachmentInfo.clearValue.color.float32[0] = attachment.info.clearValue.color.r;
            attachmentInfo.clearValue.color.float32[1] = attachment.info.clearValue.color.g;
            attachmentInfo.clearValue.color.float32[2] = attachment.info.clearValue.color.b;
            attachmentInfo.clearValue.color.float32[3] = attachment.info.clearValue.color.a;
            colorAttachmentInfo.push_back(attachmentInfo);
        }

        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.pNext = nullptr;
        renderingInfo.flags = 0;
        renderingInfo.renderArea.extent.width = width;
        renderingInfo.renderArea.extent.height = height;
        renderingInfo.renderArea.offset.x = 0;
        renderingInfo.renderArea.offset.y = 0;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = colorAttachmentInfo.size();
        renderingInfo.pColorAttachments = colorAttachmentInfo.data();

        vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
    }

    void CommandList::RenderingEnd(FrameScheduler& scheduler, Pass& pass)
    {
        vkCmdEndRendering(m_commandBuffer);
        std::vector<RHI::ImagePassAttachment*> passAttachments;
        for (auto& attachment : pass.m_imagePassAttachments)
        {
            passAttachments.push_back(&attachment);
        }
        TransitionPassAttachments(&scheduler, passAttachments);
    }

    void CommandList::PushDebugMarker(const char* name)
    {
        VkDebugMarkerMarkerInfoEXT info{};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        info.pNext = nullptr;
        info.pMarkerName = name;
        // info.color
        m_context->m_vkCmdDebugMarkerBeginEXT(m_commandBuffer, &info);
    }

    void CommandList::PopDebugMarker()
    {
        m_context->m_vkCmdDebugMarkerEndEXT(m_commandBuffer);
    }

    void CommandList::SetViewport(const RHI::Viewport& viewport)
    {
        VkViewport vkViewport{};
        vkViewport.x = viewport.offsetX;
        vkViewport.y = viewport.offsetY;
        vkViewport.width = viewport.width;
        vkViewport.height = viewport.height;
        vkViewport.minDepth = viewport.minDepth;
        vkViewport.maxDepth = viewport.maxDepth;

        vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
    }

    void CommandList::SetSicssor(const RHI::Scissor& scissor)
    {
        VkRect2D vkScissor{};
        vkScissor.extent.width = scissor.width;
        vkScissor.extent.height = scissor.height;
        vkScissor.offset.x = scissor.offsetX;
        vkScissor.offset.y = scissor.offsetY;
        vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
    }

    void CommandList::Submit(const RHI::CommandDraw& command)
    {
        auto resourceManager = m_context->m_resourceManager.get();

        auto pipeline = resourceManager->m_graphicsPipelineOwner.Get(command.pipelineState);

        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);

        if (command.shaderBindGroups.size())
        {
            auto layout = resourceManager->m_pipelineLayoutOwner.Get(pipeline->layout);

            std::vector<VkDescriptorSet> descriptorSets;
            std::vector<uint32_t> descriptorSetOffsets;

            for (auto bindGroup : command.shaderBindGroups)
            {
                auto descriptorSet = resourceManager->m_descriptorSetOwner.Get(bindGroup);
                descriptorSets.push_back(descriptorSet->handle);
                descriptorSetOffsets.push_back(0);
            }

            // vkCmdBindDescriptorSets(
            //     m_commandBuffer,
            //     VK_PIPELINE_BIND_POINT_COMPUTE,
            //     layout->handle,
            //     0,
            //     static_cast<uint32_t>(descriptorSets.size()),
            //     descriptorSets.data(),
            //     static_cast<uint32_t>(descriptorSetOffsets.size()),
            //     descriptorSetOffsets.data());
        }

        std::vector<VkBuffer> vertexBuffers;
        std::vector<VkDeviceSize> vertexBufferSizes;

        for (auto vertexBuffer : command.vertexBuffers)
        {
            auto buffer = resourceManager->m_bufferOwner.Get(vertexBuffer);

            vertexBuffers.push_back(buffer->handle);
            vertexBufferSizes.push_back(0);
        }

        if (command.vertexBuffers.size())
        {
            vkCmdBindVertexBuffers(
                m_commandBuffer,
                0,
                static_cast<uint32_t>(vertexBuffers.size()),
                vertexBuffers.data(),
                vertexBufferSizes.data());
        }

        if (command.indexBuffers)
        {
            auto buffer = resourceManager->m_bufferOwner.Get(command.indexBuffers);

            vkCmdBindIndexBuffer(m_commandBuffer, buffer->handle, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(
                m_commandBuffer,
                command.parameters.elementCount,
                command.parameters.instanceCount,
                command.parameters.firstElement,
                command.parameters.vertexOffset,
                static_cast<int32_t>(command.parameters.firstInstance));
        }
        else
        {
            vkCmdDraw(
                m_commandBuffer,
                command.parameters.elementCount,
                command.parameters.instanceCount,
                command.parameters.firstElement,
                command.parameters.firstInstance);
        }

        // vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
        // vkCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
        // vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
        // vkCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);

        // vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
    }

    void CommandList::Submit(const RHI::CommandCopy& _command)
    {
        auto resourceManager = m_context->m_resourceManager.get();

        switch (_command.type)
        {
        case RHI::CopyCommandType::Buffer:
            {
                auto& command = _command.buffer;

                auto srcBuffer = resourceManager->m_bufferOwner.Get(command.sourceBuffer);
                auto destinationBuffer = resourceManager->m_bufferOwner.Get(command.destinationBuffer);

                VkBufferCopy copyInfo{};
                copyInfo.srcOffset = command.sourceOffset;
                copyInfo.dstOffset = command.destinationOffset;
                copyInfo.size = command.size;

                vkCmdCopyBuffer(m_commandBuffer, srcBuffer->handle, destinationBuffer->handle, 1, &copyInfo);
                break;
            }
        case RHI::CopyCommandType::Image:
            {
                auto& command = _command.image;

                auto srcImage = resourceManager->m_imageOwner.Get(command.sourceImage);
                auto dstImage = resourceManager->m_imageOwner.Get(command.destinationImage);

                VkImageCopy copyInfo{};
                copyInfo.srcSubresource.aspectMask = ConvertToVkImageAspect(command.sourceSubresource.imageAspects);
                copyInfo.srcSubresource.baseArrayLayer = command.sourceSubresource.arrayBase;
                copyInfo.srcSubresource.layerCount = command.sourceSubresource.arrayCount;
                copyInfo.srcSubresource.mipLevel = command.sourceSubresource.mipBase;
                copyInfo.srcOffset.x = command.sourceOffset.x;
                copyInfo.srcOffset.y = command.sourceOffset.y;
                copyInfo.srcOffset.z = command.sourceOffset.z;
                copyInfo.dstSubresource.aspectMask = ConvertToVkImageAspect(command.destinationSubresource.imageAspects);
                copyInfo.dstSubresource.baseArrayLayer = command.destinationSubresource.arrayBase;
                copyInfo.dstSubresource.layerCount = command.destinationSubresource.arrayCount;
                copyInfo.dstSubresource.mipLevel = command.destinationSubresource.mipBase;
                copyInfo.dstOffset.x = command.destinationOffset.x;
                copyInfo.dstOffset.y = command.destinationOffset.y;
                copyInfo.dstOffset.z = command.destinationOffset.z;
                copyInfo.extent.width = command.sourceSize.width;
                copyInfo.extent.height = command.sourceSize.height;
                copyInfo.extent.depth = command.sourceSize.depth;

                vkCmdCopyImage(m_commandBuffer, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);
                break;
            }
        case RHI::CopyCommandType::BufferToImage:
            {
                auto& command = _command.bufferToImage;

                auto srcBuffer = resourceManager->m_bufferOwner.Get(command.srcBuffer);
                auto dstImage = resourceManager->m_imageOwner.Get(command.dstImage);

                VkBufferImageCopy copyInfo{};
                vkCmdCopyBufferToImage(m_commandBuffer, srcBuffer->handle, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);
                break;
            }
        case RHI::CopyCommandType::ImageToBuffer:
            {
                auto& command = _command.imageToBuffer;

                auto srcImage = resourceManager->m_imageOwner.Get(command.sourceImage);
                auto destinationBuffer = resourceManager->m_bufferOwner.Get(command.destinationBuffer);

                VkBufferImageCopy copyInfo{};
                vkCmdCopyImageToBuffer(m_commandBuffer, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destinationBuffer->handle, 1, &copyInfo);
                break;
            }
        default:
            {
                RHI_UNREACHABLE();
                break;
            }
        }
    }

    void CommandList::Submit(const RHI::CommandCompute& command)
    {
        auto resourceManager = m_context->m_resourceManager.get();

        auto pipeline = resourceManager->m_computePipelineOwner.Get(command.pipelineState);

        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle);

        if (command.shaderBindGroups.size())
        {
            auto layout = resourceManager->m_pipelineLayoutOwner.Get(pipeline->layout);

            std::vector<VkDescriptorSet> descriptorSets;
            std::vector<uint32_t> descriptorSetOffsets;

            for (auto bindGroup : command.shaderBindGroups)
            {
                auto descriptorSet = resourceManager->m_descriptorSetOwner.Get(bindGroup);
                descriptorSets.push_back(descriptorSet->handle);
                descriptorSetOffsets.push_back(0);
            }

            vkCmdBindDescriptorSets(
                m_commandBuffer,
                VK_PIPELINE_BIND_POINT_COMPUTE,
                layout->handle,
                0,
                static_cast<uint32_t>(descriptorSets.size()),
                descriptorSets.data(),
                static_cast<uint32_t>(descriptorSetOffsets.size()),
                descriptorSetOffsets.data());
        }

        vkCmdDispatchBase(
            m_commandBuffer,
            command.parameters.offsetX,
            command.parameters.offsetY,
            command.parameters.offsetZ,
            command.parameters.countX,
            command.parameters.countY,
            command.parameters.countZ);
    }

} // namespace Vulkan
