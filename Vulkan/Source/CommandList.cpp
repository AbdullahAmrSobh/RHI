#include "CommandList.hpp"
#include "Conversion.hpp"
#include "Common.hpp"
#include "Context.hpp"
#include "FrameScheduler.hpp"
#include "Resources.hpp"

#include <RHI/Format.hpp>

namespace Vulkan
{
    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandPool
    //////////////////////////////////////////////////////////////////////////////////////////

    VkResult CommandPool::Init(Context* context, uint32_t queueFamilyIndex)
    {
        auto createInfo = VkCommandPoolCreateInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        return vkCreateCommandPool(context->m_device, &createInfo, nullptr, &m_commandPool);
    }

    void CommandPool::Shutdown(Context* context)
    {
        vkDestroyCommandPool(context->m_device, m_commandPool, nullptr);
    }

    void CommandPool::Reset(Context* context)
    {
        vkResetCommandPool(context->m_device, m_commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

        m_availableCommandLists.clear();
        for (auto& commandList : m_commandLists)
        {
            m_availableCommandLists.push_back(commandList.get());
        }
    }

    CommandList* CommandPool::Allocate(Context* context)
    {
        if (!m_availableCommandLists.empty())
        {
            auto commandList = m_availableCommandLists.back();
            m_availableCommandLists.pop_back();
            return commandList;
        }

        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.commandPool = m_commandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        auto result = vkAllocateCommandBuffers(context->m_device, &allocateInfo, &commandBuffer);
        VULKAN_ASSERT_SUCCESS(result);

        return m_commandLists.emplace_back(std::make_unique<CommandList>(context, commandBuffer)).get();
    }

    void CommandPool::Release(CommandList* commandList)
    {
        m_availableCommandLists.push_back(commandList);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandListAllocator
    //////////////////////////////////////////////////////////////////////////////////////////

    CommandListAllocator::~CommandListAllocator()
    {
        for (auto& commandPool : m_commandPools)
        {
            commandPool.Shutdown(m_context);
        }
    }

    VkResult CommandListAllocator::Init(uint32_t queueFamilyIndex)
    {
        m_currentFrameIndex = 0;
        for (uint32_t i = 0; i < m_maxFrameBufferingCount; i++)
        {
            auto& commandPool = m_commandPools[i];
            auto result = commandPool.Init(m_context, queueFamilyIndex);
            VULKAN_RETURN_VKERR_CODE(result);
        }

        return VK_SUCCESS;
    }

    void CommandListAllocator::Flush()
    {
        m_currentFrameIndex = (m_currentFrameIndex + 1) % m_maxFrameBufferingCount;
        auto& commandPool = m_commandPools[m_currentFrameIndex];
        commandPool.Reset(m_context);
    }

    RHI::CommandList* CommandListAllocator::Allocate()
    {
        auto& commandPool = m_commandPools[m_currentFrameIndex];
        return commandPool.Allocate(m_context);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandList
    //////////////////////////////////////////////////////////////////////////////////////////

    void CommandList::Reset()
    {
        vkResetCommandBuffer(m_commandBuffer, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
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

    void CommandList::Begin(RHI::Pass& passBase)
    {
        auto& pass = static_cast<Pass&>(passBase);

        m_pass = &pass;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

        RenderingBegin(pass);
    }

    void CommandList::End()
    {
        if (m_pass)
        {
            RenderingEnd(*m_pass);
        }

        vkEndCommandBuffer(m_commandBuffer);
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
        auto pipeline = m_context->m_graphicsPipelineOwner.Get(command.pipelineState);

        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);

        if (command.bindGroups.size())
        {
            BindShaderBindGroups(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, command.bindGroups);
        }

        std::vector<VkBuffer> vertexBuffers;
        std::vector<VkDeviceSize> vertexBufferSizes;

        for (auto vertexBuffer : command.vertexBuffers)
        {
            auto buffer = m_context->m_bufferOwner.Get(vertexBuffer);

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

            if (command.indexBuffers)
            {
                auto buffer = m_context->m_bufferOwner.Get(command.indexBuffers);

                vkCmdBindIndexBuffer(m_commandBuffer, buffer->handle, 0, VK_INDEX_TYPE_UINT32);

                vkCmdDrawIndexed(
                    m_commandBuffer,
                    command.parameters.elementCount,
                    command.parameters.instanceCount,
                    command.parameters.firstElement,
                    command.parameters.vertexOffset,
                    static_cast<int32_t>(command.parameters.firstInstance));
            }
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
    }

    void CommandList::Submit(const RHI::CopyBufferDescriptor& command)
    {
        auto srcBuffer = m_context->m_bufferOwner.Get(command.sourceBuffer);
        auto destinationBuffer = m_context->m_bufferOwner.Get(command.destinationBuffer);

        auto copyInfo = VkBufferCopy{};
        copyInfo.srcOffset = command.sourceOffset;
        copyInfo.dstOffset = command.destinationOffset;
        copyInfo.size = command.size;
        vkCmdCopyBuffer(m_commandBuffer, srcBuffer->handle, destinationBuffer->handle, 1, &copyInfo);
    }

    void CommandList::Submit(const RHI::CopyImageDescriptor& command)
    {
        auto srcImage = m_context->m_imageOwner.Get(command.sourceImage);
        auto dstImage = m_context->m_imageOwner.Get(command.destinationImage);

        auto copyInfo = VkImageCopy{};
        copyInfo.srcSubresource = ConvertSubresourceLayer(command.sourceSubresource);
        copyInfo.srcOffset = ConvertOffset3D(command.sourceOffset);
        copyInfo.dstSubresource = ConvertSubresourceLayer(command.destinationSubresource);
        copyInfo.dstOffset = ConvertOffset3D(command.destinationOffset);
        copyInfo.extent = ConvertExtent3D(command.sourceSize);
        vkCmdCopyImage(m_commandBuffer, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);
    }

    void CommandList::Submit(const RHI::CopyBufferToImageDescriptor& command)
    {
        auto srcBuffer = m_context->m_bufferOwner.Get(command.srcBuffer);
        auto dstImage = m_context->m_imageOwner.Get(command.dstImage);

        auto copyInfo = VkBufferImageCopy{};
        copyInfo.bufferOffset = command.srcOffset;
        copyInfo.bufferRowLength = command.srcBytesPerRow;
        // copyInfo.bufferImageHeight;
        copyInfo.imageSubresource = ConvertSubresourceLayer(command.dstSubresource);
        copyInfo.imageOffset = ConvertOffset3D(command.dstOffset);
        copyInfo.imageExtent = ConvertExtent3D(command.srcSize);
        vkCmdCopyBufferToImage(m_commandBuffer, srcBuffer->handle, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);
    }

    void CommandList::Submit(const RHI::CopyImageToBufferDescriptor& command)
    {
        auto srcImage = m_context->m_imageOwner.Get(command.sourceImage);
        auto dstBuffer = m_context->m_bufferOwner.Get(command.destinationBuffer);

        auto copyInfo = VkBufferImageCopy{};
        copyInfo.bufferOffset = command.destinationOffset;
        copyInfo.bufferRowLength = command.destinationBytesPerRow;
        // copyInfo.bufferImageHeight;
        copyInfo.imageSubresource = ConvertSubresourceLayer(command.sourceSubresource);
        copyInfo.imageOffset = ConvertOffset3D(command.sourceOffset);
        copyInfo.imageExtent = ConvertExtent3D(command.sourceSize);
        vkCmdCopyImageToBuffer(m_commandBuffer, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstBuffer->handle, 1, &copyInfo);
    }

    void CommandList::Submit(const RHI::CommandCompute& command)
    {
        auto pipeline = m_context->m_computePipelineOwner.Get(command.pipelineState);

        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle);

        if (command.bindGroups.size())
        {
            BindShaderBindGroups(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->layout, command.bindGroups);
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

    VkRenderingAttachmentInfo CommandList::GetAttachmentInfo(const RHI::ImagePassAttachment& passAttachment) const
    {
        auto imageView = m_context->m_imageViewOwner.Get(passAttachment.view);

        auto attachmentInfo = VkRenderingAttachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext = nullptr;
        attachmentInfo.imageView = imageView->handle;
        attachmentInfo.imageLayout = ConvertImageLayout(passAttachment.info.usage, passAttachment.info.access);
        attachmentInfo.loadOp = ConvertLoadOp(passAttachment.info.loadStoreOperations.loadOperation);
        attachmentInfo.storeOp = ConvertStoreOp(passAttachment.info.loadStoreOperations.storeOperation);
        attachmentInfo.clearValue.color.float32[0] = passAttachment.info.clearValue.color.r;
        attachmentInfo.clearValue.color.float32[1] = passAttachment.info.clearValue.color.g;
        attachmentInfo.clearValue.color.float32[2] = passAttachment.info.clearValue.color.b;
        attachmentInfo.clearValue.color.float32[3] = passAttachment.info.clearValue.color.a;
        return attachmentInfo;
    }

    void CommandList::RenderingBegin(Pass& pass)
    {
        std::vector<RHI::ImagePassAttachment*> passAttachments;

        for (auto& attachment : pass.m_imagePassAttachments)
        {
            if (attachment.info.usage == RHI::AttachmentUsage::RenderTarget)
            {
                passAttachments.push_back(&attachment);
            }
        }
        TransitionPassAttachments(BarrierType::PrePass, passAttachments);

        std::vector<VkRenderingAttachmentInfo> colorAttachmentInfo;
        std::optional<VkRenderingAttachmentInfo> depthAttachmentInfo;

        for (const auto& passAttachment : pass.m_imagePassAttachments)
        {
            if (passAttachment.info.usage == RHI::AttachmentUsage::Depth)
            {
                depthAttachmentInfo = GetAttachmentInfo(passAttachment);
                continue;
            }

            if (passAttachment.info.usage == RHI::AttachmentUsage::RenderTarget)
            {
                auto attachmentInfo = GetAttachmentInfo(passAttachment);
                colorAttachmentInfo.push_back(attachmentInfo);
            }
        }

        auto renderingInfo = VkRenderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.pNext = nullptr;
        renderingInfo.flags = 0;
        renderingInfo.renderArea.extent.width = 800;
        renderingInfo.renderArea.extent.height = 600;
        renderingInfo.renderArea.offset.x = 0;
        renderingInfo.renderArea.offset.y = 0;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = colorAttachmentInfo.size();
        renderingInfo.pColorAttachments = colorAttachmentInfo.data();
        renderingInfo.pDepthAttachment = depthAttachmentInfo.has_value() ? &depthAttachmentInfo.value() : nullptr;
        vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
    }

    void CommandList::RenderingEnd(Pass& pass)
    {
        vkCmdEndRendering(m_commandBuffer);
        std::vector<RHI::ImagePassAttachment*> passAttachments;
        for (auto& attachment : pass.m_imagePassAttachments)
        {
            if (attachment.info.usage == RHI::AttachmentUsage::RenderTarget)
            {
                passAttachments.push_back(&attachment);
            }
        }
        TransitionPassAttachments(BarrierType::PostPass, passAttachments);
    }

    void CommandList::PushDebugMarker(const char* name)
    {
        (void)name;
#if RHI_DEBUG
        if (m_context->m_vkCmdDebugMarkerBeginEXT)
        {
            VkDebugMarkerMarkerInfoEXT info{};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
            info.pNext = nullptr;
            info.pMarkerName = name;
            // info.color
            m_context->m_vkCmdDebugMarkerBeginEXT(m_commandBuffer, &info);
        }
#endif
    }

    void CommandList::PopDebugMarker()
    {
#if RHI_DEUG
        if (m_context->m_vkCmdDebugMarkerEndEXT)
        {
            m_context->m_vkCmdDebugMarkerEndEXT(m_commandBuffer);
        }
#endif
    }

    void CommandList::BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<RHI::Handle<RHI::BindGroup>> bindGroups)
    {
        std::vector<VkDescriptorSet> descriptorSets;

        for (auto bindGroupHandle : bindGroups)
        {
            auto bindGroup = m_context->m_bindGroupOwner.Get(bindGroupHandle);
            descriptorSets.push_back(bindGroup->handle);
        }

        vkCmdBindDescriptorSets(m_commandBuffer, bindPoint, pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
    }

    void CommandList::TransitionPassAttachments(BarrierType barrierType, TL::Span<RHI::ImagePassAttachment*> passAttachments) const
    {
        std::vector<VkImageMemoryBarrier2> barriers;

        for (auto passAttachment : passAttachments)
        {
            auto image = m_context->m_imageOwner.Get(passAttachment->attachment->handle);

            auto barrier = VkImageMemoryBarrier2{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier.pNext = nullptr;
            barrier.image = image->handle;
            barrier.subresourceRange.aspectMask = ConvertImageAspect(passAttachment->info.subresource.imageAspects);
            barrier.subresourceRange.baseArrayLayer = passAttachment->info.subresource.arrayBase;
            barrier.subresourceRange.layerCount = passAttachment->info.subresource.arrayCount;
            barrier.subresourceRange.baseMipLevel = passAttachment->info.subresource.mipBase;
            barrier.subresourceRange.levelCount = passAttachment->info.subresource.mipCount;

            if (passAttachment->next)
            {
                barrier.srcQueueFamilyIndex = static_cast<Pass*>(passAttachment->pass)->m_queueFamilyIndex;
                barrier.dstQueueFamilyIndex = static_cast<Pass*>(passAttachment->next->pass)->m_queueFamilyIndex;

                if (barrier.srcQueueFamilyIndex == barrier.dstQueueFamilyIndex &&
                    passAttachment->info.access == RHI::AttachmentAccess::Read &&
                    passAttachment->next->info.access == RHI::AttachmentAccess::Read)
                {
                    continue;
                }

                barrier.srcStageMask = ConvertPipelineStageFlags(passAttachment->info.usage, passAttachment->stage);
                barrier.srcAccessMask = ConvertPipelineAccess(passAttachment->info.usage, passAttachment->info.access, false);
                barrier.oldLayout = ConvertImageLayout(passAttachment->info.usage, passAttachment->info.access);
                barrier.dstStageMask = ConvertPipelineStageFlags(passAttachment->next->info.usage, passAttachment->next->stage);
                barrier.dstAccessMask = ConvertPipelineAccess(passAttachment->next->info.usage, passAttachment->next->info.access, true);
                barrier.newLayout = ConvertImageLayout(passAttachment->next->info.usage, passAttachment->next->info.access);
                barriers.push_back(barrier);
            }
            else if (barrierType == BarrierType::PostPass && passAttachment->attachment->swapchain)
            {
                barrier.srcStageMask = ConvertPipelineStageFlags(passAttachment->info.usage, passAttachment->stage);
                barrier.srcAccessMask = ConvertPipelineAccess(passAttachment->info.usage, passAttachment->info.access, true);
                barrier.oldLayout = ConvertImageLayout(passAttachment->info.usage, passAttachment->info.access);
                barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
                barrier.dstAccessMask = 0;
                barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                barriers.push_back(barrier);
            }
            else if (barrierType == BarrierType::PrePass && passAttachment->prev == nullptr && passAttachment->info.access != RHI::AttachmentAccess::Read)
            {
                barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
                barrier.srcAccessMask = 0;
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.dstStageMask = ConvertPipelineStageFlags(passAttachment->info.usage, passAttachment->stage);
                barrier.dstAccessMask = ConvertPipelineAccess(passAttachment->info.usage, passAttachment->info.access, false);
                barrier.newLayout = ConvertImageLayout(passAttachment->info.usage, passAttachment->info.access);
                barriers.push_back(barrier);
            }
        }

        auto dependencyInfo = VkDependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.pNext = nullptr;
        dependencyInfo.imageMemoryBarrierCount = barriers.size();
        dependencyInfo.pImageMemoryBarriers = barriers.data();
        vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
    }

    void CommandList::TransitionPassAttachments(BarrierType barrierType, TL::Span<RHI::BufferPassAttachment*> passAttachments) const
    {
        (void)barrierType;
        std::vector<VkBufferMemoryBarrier2> barriers;

        for (auto passAttachment : passAttachments)
        {
            auto buffer = m_context->m_bufferOwner.Get(passAttachment->attachment->handle);

            auto barrier = VkBufferMemoryBarrier2{};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            barrier.pNext = nullptr;
            barrier.buffer = buffer->handle;
            barrier.offset = passAttachment->info.byteOffset;
            barrier.size = passAttachment->info.byteSize;

            if (passAttachment->next)
            {
                barrier.srcQueueFamilyIndex = static_cast<Pass*>(passAttachment->pass)->m_queueFamilyIndex;
                barrier.dstQueueFamilyIndex = static_cast<Pass*>(passAttachment->next->pass)->m_queueFamilyIndex;

                if (barrier.srcQueueFamilyIndex == barrier.dstQueueFamilyIndex &&
                    passAttachment->info.access == RHI::AttachmentAccess::Read &&
                    passAttachment->next->info.access == RHI::AttachmentAccess::Read)
                {
                    continue;
                }

                barrier.srcStageMask = ConvertPipelineStageFlags(passAttachment->info.usage, passAttachment->stage);
                barrier.srcAccessMask = ConvertPipelineAccess(passAttachment->info.usage, passAttachment->info.access, false);
                barrier.dstStageMask = ConvertPipelineStageFlags(passAttachment->next->info.usage, passAttachment->next->stage);
                barrier.dstAccessMask = ConvertPipelineAccess(passAttachment->next->info.usage, passAttachment->next->info.access, true);
                barriers.push_back(barrier);
            }
            else
            {
                RHI_UNREACHABLE();
            }
        }

        auto dependencyInfo = VkDependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.pNext = nullptr;
        dependencyInfo.bufferMemoryBarrierCount = barriers.size();
        dependencyInfo.pBufferMemoryBarriers = barriers.data();
        vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
    }

} // namespace Vulkan
