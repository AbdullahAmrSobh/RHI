#include "CommandList.hpp"
#include "Common.hpp"
#include "Context.hpp"
#include "FrameScheduler.hpp"
#include "Resources.hpp"

#include <RHI/Format.hpp>

#include <optional>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    //////////////////////////////////////////////////////////////////////////////////////////
    /// Utills functions
    //////////////////////////////////////////////////////////////////////////////////////////

    inline static VkAccessFlags2 GetAccessFlags(VkImageLayout layout)
    {
        switch (layout)
        {
        case VK_IMAGE_LAYOUT_UNDEFINED:
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:                              return 0;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:                               return VK_ACCESS_2_HOST_WRITE_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:                     return VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:                     return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR: return VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:                     return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:                         return VK_ACCESS_2_TRANSFER_READ_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:                         return VK_ACCESS_2_TRANSFER_WRITE_BIT;
        case VK_IMAGE_LAYOUT_GENERAL:
            RHI_UNREACHABLE();
            return 0;
        default:
            RHI_UNREACHABLE();
            return 0;
        }
    }

    inline static VkPipelineStageFlags2 GetPipelineStageFlags(VkImageLayout layout)
    {
        switch (layout)
        {
        case VK_IMAGE_LAYOUT_UNDEFINED:                                    return VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:                               return VK_PIPELINE_STAGE_2_HOST_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:                         return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:                     return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:                     return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR: return VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:                     return VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:                              return VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        case VK_IMAGE_LAYOUT_GENERAL:
            RHI_UNREACHABLE();
            return 0;
        default:
            RHI_UNREACHABLE();
            return 0;
        }
    }

    inline static VkImageMemoryBarrier2 CreateImageMemoryBarrier2(VkImage image,
                                                                  VkPipelineStageFlags2 srcStageMask,
                                                                  VkPipelineStageFlags2 dstStageMask,
                                                                  VkAccessFlags2 srcAccessMask,
                                                                  VkAccessFlags2 dstAccessMask,
                                                                  VkImageLayout oldLayout,
                                                                  VkImageLayout newLayout,
                                                                  const VkImageSubresourceRange& subresourceRange)
    {
        VkImageMemoryBarrier2 image_memory_barrier{};
        image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        image_memory_barrier.srcStageMask = srcStageMask;
        image_memory_barrier.dstStageMask = dstStageMask;
        image_memory_barrier.srcAccessMask = srcAccessMask;
        image_memory_barrier.dstAccessMask = dstAccessMask;
        image_memory_barrier.oldLayout = oldLayout;
        image_memory_barrier.newLayout = newLayout;
        image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_memory_barrier.image = image;
        image_memory_barrier.subresourceRange = subresourceRange;
        return image_memory_barrier;
    }

    inline static VkImageMemoryBarrier2 CreateImageMemoryBarrier2(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& subresourceRange)
    {
        VkPipelineStageFlags2 srcStageMask = GetPipelineStageFlags(oldLayout);
        VkPipelineStageFlags2 dstStageMask = GetPipelineStageFlags(newLayout);
        VkAccessFlags2 srcAccessMask = GetAccessFlags(oldLayout);
        VkAccessFlags2 dstAccessMask = GetAccessFlags(newLayout);

        return CreateImageMemoryBarrier2(image, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask, oldLayout, newLayout, subresourceRange);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandPool
    //////////////////////////////////////////////////////////////////////////////////////////

    VkResult CommandPool::Init(IContext* context, uint32_t queueFamilyIndex)
    {
        auto createInfo = VkCommandPoolCreateInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        return vkCreateCommandPool(context->m_device, &createInfo, nullptr, &m_commandPool);
    }

    void CommandPool::Shutdown(IContext* context)
    {
        vkDestroyCommandPool(context->m_device, m_commandPool, nullptr);
    }

    void CommandPool::Reset(IContext* context)
    {
        vkResetCommandPool(context->m_device, m_commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

        m_availableCommandLists.clear();
        for (auto& commandList : m_commandLists)
        {
            m_availableCommandLists.push_back(commandList.get());
        }
    }

    ICommandList* CommandPool::Allocate(IContext* context)
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

        return m_commandLists.emplace_back(std::make_unique<ICommandList>(context, commandBuffer)).get();
    }

    void CommandPool::Release(ICommandList* commandList)
    {
        m_availableCommandLists.push_back(commandList);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// ICommandListAllocator
    //////////////////////////////////////////////////////////////////////////////////////////

    ICommandListAllocator::ICommandListAllocator(IContext* context)
        : m_context(context)
        , m_maxFrameBufferingCount(2)
        , m_commandPools()
    {
    }

    ICommandListAllocator::~ICommandListAllocator()
    {
        for (auto& commandPool : m_commandPools)
        {
            commandPool.Shutdown(m_context);
        }
    }

    VkResult ICommandListAllocator::Init(QueueType queueType)
    {
        ZoneScoped;

        auto framesCount = m_context->GetScheduler().GetBufferedFramesCount();
        auto queueFamilyIndex = m_context->GetQueueFamilyIndex(queueType);
        m_currentFrameIndex = 0;
        for (uint32_t i = 0; i < framesCount; i++)
        {
            auto& commandPool = m_commandPools[i];
            auto result = commandPool.Init(m_context, queueFamilyIndex);
            VULKAN_RETURN_VKERR_CODE(result);
        }

        return VK_SUCCESS;
    }

    CommandList* ICommandListAllocator::Allocate()
    {
        ZoneScoped;

        auto& pool = m_commandPools[m_currentFrameIndex];
        return pool.Allocate(m_context);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandList
    //////////////////////////////////////////////////////////////////////////////////////////

    ICommandList::ICommandList(IContext* context, VkCommandBuffer commandBuffer)
        : m_context(context)
        , m_pass(nullptr)
        , m_commandBuffer(commandBuffer)
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

        auto& pass = static_cast<IPass&>(passBase);

        m_pass = &pass;
        m_pass->m_commandLists.push_back(this);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

        RenderingBegin(pass);
    }

    void ICommandList::End()
    {
        ZoneScoped;

        if (m_pass)
        {
            RenderingEnd(*m_pass);
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

    VkRenderingAttachmentInfo ICommandList::GetAttachmentInfo(const ImagePassAttachment& passAttachment) const
    {
        ZoneScoped;

        auto imageView = m_context->m_imageViewOwner.Get(passAttachment.view);
        RHI_ASSERT(imageView != nullptr);

        auto attachmentInfo = VkRenderingAttachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext = nullptr;
        attachmentInfo.imageView = imageView->handle;
        attachmentInfo.imageLayout = ConvertImageLayout(passAttachment.usage, passAttachment.access);
        attachmentInfo.loadOp = ConvertLoadOp(passAttachment.loadStoreOperations.loadOperation);
        attachmentInfo.storeOp = ConvertStoreOp(passAttachment.loadStoreOperations.storeOperation);

        static_assert(sizeof(VkClearValue) == sizeof(ClearValue));
        memcpy(&attachmentInfo.clearValue, &passAttachment.clearValue, sizeof(VkClearValue));

        return attachmentInfo;
    }

    void ICommandList::RenderingBegin(IPass& pass)
    {
        ZoneScoped;

        ImageSize2D renderArea = pass.m_size;
        std::vector<ImagePassAttachment*> passAttachments;

        for (auto& attachment : pass.m_imagePassAttachments)
        {
            if (IsRenderTarget(attachment->usage))
            {
                passAttachments.push_back(attachment);
            }
        }

        TransitionPassAttachments(BarrierType::PrePass, passAttachments);

        std::vector<VkRenderingAttachmentInfo> colorAttachmentInfo;
        std::optional<VkRenderingAttachmentInfo> depthAttachmentInfo;

        for (const auto& passAttachment : pass.m_imagePassAttachments)
        {
            if (passAttachment->usage == AttachmentUsage::Depth)
            {
                depthAttachmentInfo = GetAttachmentInfo(*passAttachment);
                continue;
            }

            if (passAttachment->usage == AttachmentUsage::Color)
            {
                auto attachmentInfo = GetAttachmentInfo(*passAttachment);
                colorAttachmentInfo.push_back(attachmentInfo);
            }
        }

        auto renderingInfo = VkRenderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.pNext = nullptr;
        renderingInfo.flags = 0;
        renderingInfo.renderArea.extent.width = renderArea.width;
        renderingInfo.renderArea.extent.height = renderArea.height;
        renderingInfo.renderArea.offset.x = 0;
        renderingInfo.renderArea.offset.y = 0;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = uint32_t(colorAttachmentInfo.size());
        renderingInfo.pColorAttachments = colorAttachmentInfo.data();
        renderingInfo.pDepthAttachment = depthAttachmentInfo.has_value() ? &depthAttachmentInfo.value() : nullptr;
        vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
    }

    void ICommandList::RenderingEnd(IPass& pass)
    {
        ZoneScoped;

        vkCmdEndRendering(m_commandBuffer);
        std::vector<ImagePassAttachment*> passAttachments;
        for (auto& attachment : pass.m_imagePassAttachments)
        {
            if (IsRenderTarget(attachment->usage))
            {
                passAttachments.push_back(attachment);
            }
        }
        TransitionPassAttachments(BarrierType::PostPass, passAttachments);
    }

    void ICommandList::PushDebugMarker(const char* name)
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
            // info.color
            m_context->m_vkCmdDebugMarkerBeginEXT(m_commandBuffer, &info);
        }
#endif
    }

    void ICommandList::PopDebugMarker()
    {
        ZoneScoped;

#if RHI_DEUG
        if (m_context->m_vkCmdDebugMarkerEndEXT)
        {
            m_context->m_vkCmdDebugMarkerEndEXT(m_commandBuffer);
        }
#endif
    }

    void ICommandList::BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<Handle<BindGroup>> bindGroups)
    {
        ZoneScoped;

        uint32_t count = 0;
        VkDescriptorSet descriptorSets[c_MaxPipelineBindGroupsCount] = {};
        for (auto bindGroupHandle : bindGroups)
        {
            auto bindGroup = m_context->m_bindGroupOwner.Get(bindGroupHandle);
            descriptorSets[count++] = bindGroup->descriptorSet;
        }
        vkCmdBindDescriptorSets(m_commandBuffer, bindPoint, pipelineLayout, 0, count, descriptorSets, 0, nullptr);
    }

    void ICommandList::TransitionPassAttachments(BarrierType barrierType, TL::Span<ImagePassAttachment*> passAttachments)
    {
        ZoneScoped;

        std::vector<VkImageMemoryBarrier2> barriers;

        for (auto passAttachment : passAttachments)
        {
            auto imageHandle = passAttachment->attachment->GetImage();
            auto image = m_context->m_imageOwner.Get(imageHandle);

            if (auto swapchain = passAttachment->attachment->swapchain)
            {
                auto handle = swapchain->GetImage();
                image = m_context->m_imageOwner.Get(handle);
            }

            auto barrier = VkImageMemoryBarrier2{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier.pNext = nullptr;
            barrier.image = image->handle;
            barrier.subresourceRange = ConvertSubresourceRange(passAttachment->viewInfo.subresource);

            if (passAttachment->next)
            {
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                if (barrier.srcQueueFamilyIndex == barrier.dstQueueFamilyIndex &&
                    passAttachment->access == AttachmentAccess::Read &&
                    passAttachment->next->access == AttachmentAccess::Read)
                {
                    continue;
                }

                barrier.srcStageMask = ConvertPipelineStageFlags(passAttachment->usage, passAttachment->stage);
                barrier.srcAccessMask = ConvertPipelineAccess(passAttachment->usage, passAttachment->access);
                barrier.oldLayout = ConvertImageLayout(passAttachment->usage, passAttachment->access);
                barrier.dstStageMask = ConvertPipelineStageFlags(passAttachment->next->usage, passAttachment->next->stage);
                barrier.dstAccessMask = ConvertPipelineAccess(passAttachment->next->usage, passAttachment->next->access);
                barrier.newLayout = ConvertImageLayout(passAttachment->next->usage, passAttachment->next->access);
                barriers.push_back(barrier);
            }
            else if (passAttachment->attachment->swapchain)
            {
                if (barrierType == BarrierType::PrePass)
                {
                    auto oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    auto newLayout = ConvertImageLayout(passAttachment->usage, passAttachment->access);
                    barrier = CreateImageMemoryBarrier2(image->handle, oldLayout, newLayout, barrier.subresourceRange);
                    barriers.push_back(barrier);
                }
                else if (barrierType == BarrierType::PostPass)
                {
                    auto oldLayout = ConvertImageLayout(passAttachment->usage, passAttachment->access);
                    auto newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    barrier = CreateImageMemoryBarrier2(image->handle, oldLayout, newLayout, barrier.subresourceRange);
                    barriers.push_back(barrier);
                }
                else
                {
                    RHI_UNREACHABLE();
                }
            }
            else if (barrierType == BarrierType::PrePass && passAttachment->prev == nullptr)
            {
                auto oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                auto newLayout = ConvertImageLayout(passAttachment->usage, passAttachment->access);
                barrier = CreateImageMemoryBarrier2(image->handle, oldLayout, newLayout, barrier.subresourceRange);
                barriers.push_back(barrier);
            }
            // else if (barrierType == BarrierType::PostPass && passAttachment->next == nullptr)
            // {
            // }
            // else
            // {
            // }
        }

        PipelineBarrier(barriers);
    }

    void ICommandList::TransitionPassAttachments(BarrierType barrierType, TL::Span<BufferPassAttachment*> passAttachments)
    {
        ZoneScoped;

        (void)barrierType;
        std::vector<VkBufferMemoryBarrier2> barriers;

        for (auto passAttachment : passAttachments)
        {
            auto buffer = m_context->m_bufferOwner.Get(passAttachment->attachment->GetBuffer());

            auto barrier = VkBufferMemoryBarrier2{};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            barrier.pNext = nullptr;
            barrier.buffer = buffer->handle;
            barrier.offset = passAttachment->viewInfo.byteOffset;
            barrier.size = passAttachment->viewInfo.byteSize;

            if (passAttachment->next)
            {
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                if (barrier.srcQueueFamilyIndex == barrier.dstQueueFamilyIndex &&
                    passAttachment->access == AttachmentAccess::Read &&
                    passAttachment->next->access == AttachmentAccess::Read)
                {
                    continue;
                }

                barrier.srcStageMask = ConvertPipelineStageFlags(passAttachment->usage, passAttachment->stage);
                barrier.srcAccessMask = ConvertPipelineAccess(passAttachment->usage, passAttachment->access);
                barrier.dstStageMask = ConvertPipelineStageFlags(passAttachment->next->usage, passAttachment->next->stage);
                barrier.dstAccessMask = ConvertPipelineAccess(passAttachment->next->usage, passAttachment->next->access);
                barriers.push_back(barrier);
            }
            else
            {
                RHI_UNREACHABLE();
            }
        }

        PipelineBarrier(barriers);
    }

    void ICommandList::PipelineBarrier(TL::Span<VkMemoryBarrier2> memoryBarriers, TL::Span<VkBufferMemoryBarrier2> bufferBarriers, TL::Span<VkImageMemoryBarrier2> imageBarriers)
    {
        ZoneScoped;

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
