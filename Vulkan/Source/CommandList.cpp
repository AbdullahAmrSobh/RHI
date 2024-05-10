#include "CommandList.hpp"
#include "Common.hpp"
#include "Context.hpp"
#include "Resources.hpp"
#include "Swapchain.hpp"
#include "RenderGraphCompiler.hpp"

#include <RHI/Format.hpp>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandList
    //////////////////////////////////////////////////////////////////////////////////////////

    ICommandList::ICommandList(IContext* context, VkCommandPool commandPool, VkCommandBuffer commandBuffer)
        : m_commandBuffer(commandBuffer)
        , m_commandPool(commandPool)
        , m_context(context)
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

    void ICommandList::Begin(RenderGraph& renderGraph, Handle<Pass> passHandle)
    {
        ZoneScoped;

        auto pass = renderGraph.m_passOwner.Get(passHandle);
        RenderGraphCompiler::CompilePass(m_context, renderGraph, pass);
        m_passSubmitData = (IPassSubmitData*)pass->submitData;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if (m_level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
        {
            vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

            PipelineBarrier({}, m_passSubmitData->bufferBarriers[BarrierType::PrePass], m_passSubmitData->imageBarriers[BarrierType::PrePass]);

            VkRenderingInfo renderingInfo{};
            renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
            renderingInfo.pNext = nullptr;
            renderingInfo.flags = 0;
            renderingInfo.renderArea.extent = ConvertExtent2D(pass->renderTargetSize);
            renderingInfo.layerCount = 1;
            renderingInfo.viewMask = 0;
            renderingInfo.colorAttachmentCount = (uint32_t)m_passSubmitData->colorAttachments.size();
            renderingInfo.pColorAttachments = m_passSubmitData->colorAttachments.data();
            renderingInfo.pDepthAttachment = m_passSubmitData->hasDepthAttachemnt ? &m_passSubmitData->depthAttachmentInfo : nullptr;
            renderingInfo.pStencilAttachment = m_passSubmitData->hasStencilAttachment ? &m_passSubmitData->stencilAttachmentInfo : nullptr;
            vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
        }
        else
        {
            VkCommandBufferInheritanceRenderingInfoKHR renderinginheritanceInfo{};
            renderinginheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO_KHR;
            renderinginheritanceInfo.pNext = nullptr;
            renderinginheritanceInfo.flags = 0;
            renderinginheritanceInfo.viewMask = 0;
            renderinginheritanceInfo.colorAttachmentCount = (uint32_t)m_passSubmitData->colorFormats.size();
            renderinginheritanceInfo.pColorAttachmentFormats = m_passSubmitData->colorFormats.data();
            renderinginheritanceInfo.depthAttachmentFormat = m_passSubmitData->depthFormat;
            renderinginheritanceInfo.stencilAttachmentFormat = m_passSubmitData->stencilformat;
            renderinginheritanceInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            VkCommandBufferInheritanceInfo inheritanceInfo{};
            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.pNext = &renderinginheritanceInfo;
            beginInfo.pInheritanceInfo = &inheritanceInfo;
            vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
        }
    }

    void ICommandList::End()
    {
        ZoneScoped;

        if (m_passSubmitData)
        {
            if (m_passSubmitData->colorAttachments.empty() == false || m_passSubmitData->hasDepthAttachemnt || m_passSubmitData->hasStencilAttachment)
            {
                vkCmdEndRendering(m_commandBuffer);
            }
            PipelineBarrier({}, m_passSubmitData->bufferBarriers[BarrierType::PrePass], m_passSubmitData->imageBarriers[BarrierType::PrePass]);
        }

        vkEndCommandBuffer(m_commandBuffer);
    }

    void ICommandList::DebugMarkerPush(const char* name, ColorValue<float> color)
    {
        ZoneScoped;

        (void)name;
        (void)color;

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

    void ICommandList::BeginConditionalCommands(Handle<Buffer> handle, size_t offset, bool inverted)
    {
        ZoneScoped;

        if (m_context->m_vkCmdBeginConditionalRenderingEXT)
        {
            m_context->DebugLogWarn("This function is not available");
            return;
        }

        auto buffer = m_context->m_bufferOwner.Get(handle);

        VkConditionalRenderingBeginInfoEXT beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT;
        beginInfo.pNext = nullptr;
        beginInfo.buffer = buffer->handle;
        beginInfo.offset = offset;
        beginInfo.flags = inverted ? VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT : 0u;
        m_context->m_vkCmdBeginConditionalRenderingEXT(m_commandBuffer, &beginInfo);
    }

    void ICommandList::EndConditionalCommands()
    {
        ZoneScoped;

        if (m_context->m_vkCmdEndConditionalRenderingEXT)
        {
            m_context->DebugLogWarn("This function is not available");
            return;
        }

        m_context->m_vkCmdEndConditionalRenderingEXT(m_commandBuffer);
    }

    void ICommandList::Execute(TL::Span<const CommandList*> commandLists)
    {
        ZoneScoped;

        TL::Vector<VkCommandBuffer> commandBuffers;
        commandBuffers.reserve(commandLists.size());
        for (auto _commandList : commandLists)
        {
            auto commandList = (const ICommandList*)_commandList;
            commandBuffers.push_back(commandList->m_commandBuffer);
        }
        vkCmdExecuteCommands(m_commandBuffer, uint32_t(commandBuffers.size()), commandBuffers.data());
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

    void ICommandList::Draw(const DrawInfo& drawInfo)
    {
        ZoneScoped;

        auto pipeline = m_context->m_graphicsPipelineOwner.Get(drawInfo.pipelineState);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);

        if (drawInfo.bindGroups.empty() == false)
        {
            BindShaderBindGroups(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, drawInfo.bindGroups, drawInfo.dynamicOffset);
        }

        if (drawInfo.vertexBuffers.empty() == false)
        {
            uint32_t vertexBuffersCount = 0;
            VkBuffer vertexBuffers[c_MaxPipelineVertexBindings] = {};
            VkDeviceSize vertexBufferOffsets[c_MaxPipelineVertexBindings] = {};
            for (auto handle : drawInfo.vertexBuffers)
            {
                auto buffer = m_context->m_bufferOwner.Get(handle);
                auto index = vertexBuffersCount++;
                vertexBuffers[index] = buffer->handle;
                vertexBufferOffsets[index] = 0;
            }
            vkCmdBindVertexBuffers(m_commandBuffer, 0, vertexBuffersCount, vertexBuffers, vertexBufferOffsets);
        }

        auto parameters = drawInfo.parameters;
        if (drawInfo.indexBuffers != RHI::NullHandle)
        {
            auto buffer = m_context->m_bufferOwner.Get(drawInfo.indexBuffers);
            vkCmdBindIndexBuffer(m_commandBuffer, buffer->handle, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(m_commandBuffer, parameters.elementCount, parameters.instanceCount, parameters.firstElement, int32_t(parameters.vertexOffset), uint32_t(parameters.firstInstance));
        }
        else
        {
            vkCmdDraw(m_commandBuffer, parameters.elementCount, parameters.instanceCount, parameters.firstElement, uint32_t(parameters.firstInstance));
        }
    }

    void ICommandList::Dispatch(const DispatchInfo& dispatchInfo)
    {
        ZoneScoped;

        auto pipeline = m_context->m_computePipelineOwner.Get(dispatchInfo.pipelineState);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle);
        if (dispatchInfo.bindGroups.size())
        {
            BindShaderBindGroups(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->layout, dispatchInfo.bindGroups, {});
        }
        auto parameters = dispatchInfo.parameters;
        vkCmdDispatchBase(m_commandBuffer, parameters.offsetX, parameters.offsetY, parameters.offsetZ, parameters.countX, parameters.countY, parameters.countZ);
    }

    void ICommandList::Copy(const BufferCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto srcBuffer = m_context->m_bufferOwner.Get(copyInfo.srcBuffer);
        auto dstBuffer = m_context->m_bufferOwner.Get(copyInfo.dstBuffer);

        VkBufferCopy bufferCopy{};
        bufferCopy.srcOffset = copyInfo.srcOffset;
        bufferCopy.dstOffset = copyInfo.dstOffset;
        bufferCopy.size = copyInfo.size;
        vkCmdCopyBuffer(m_commandBuffer, srcBuffer->handle, dstBuffer->handle, 1, &bufferCopy);
    }

    void ICommandList::Copy(const ImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto srcImage = m_context->m_imageOwner.Get(copyInfo.srcImage);
        auto dstImage = m_context->m_imageOwner.Get(copyInfo.dstImage);

        VkImageCopy imageCopy{};
        imageCopy.srcSubresource = ConvertSubresourceLayer(copyInfo.srcSubresource);
        imageCopy.srcOffset = ConvertOffset3D(copyInfo.srcOffset);
        imageCopy.dstSubresource = ConvertSubresourceLayer(copyInfo.dstSubresource);
        imageCopy.dstOffset = ConvertOffset3D(copyInfo.dstOffset);
        imageCopy.extent = ConvertExtent3D(copyInfo.srcSize);
        vkCmdCopyImage(m_commandBuffer, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
    }

    void ICommandList::Copy(const BufferToImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto srcBuffer = m_context->m_bufferOwner.Get(copyInfo.srcBuffer);
        auto dstImage = m_context->m_imageOwner.Get(copyInfo.dstImage);

        VkBufferImageCopy bufferImageCopy{};
        bufferImageCopy.bufferOffset = copyInfo.srcOffset;
        bufferImageCopy.bufferRowLength = copyInfo.srcBytesPerRow;
        bufferImageCopy.bufferImageHeight = copyInfo.srcBytesPerImage;
        bufferImageCopy.imageSubresource = ConvertSubresourceLayer(copyInfo.dstSubresource);
        bufferImageCopy.imageOffset = ConvertOffset3D(copyInfo.dstOffset);
        bufferImageCopy.imageExtent = ConvertExtent3D(copyInfo.dstSize);
        vkCmdCopyBufferToImage(m_commandBuffer, srcBuffer->handle, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
    }

    void ICommandList::Copy(const ImageToBufferCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto srcImage = m_context->m_imageOwner.Get(copyInfo.srcImage);
        auto dstBuffer = m_context->m_bufferOwner.Get(copyInfo.dstBuffer);

        VkBufferImageCopy bufferImageCopy{};
        bufferImageCopy.bufferOffset = copyInfo.dstOffset;
        bufferImageCopy.bufferRowLength = copyInfo.dstBytesPerRow;
        bufferImageCopy.bufferImageHeight = copyInfo.dstBytesPerImage;
        bufferImageCopy.imageSubresource = ConvertSubresourceLayer(copyInfo.srcSubresource);
        bufferImageCopy.imageOffset = ConvertOffset3D(copyInfo.srcOffset);
        bufferImageCopy.imageExtent = ConvertExtent3D(copyInfo.srcSize);
        vkCmdCopyImageToBuffer(m_commandBuffer, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstBuffer->handle, 1, &bufferImageCopy);
    }

    void ICommandList::BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<const Handle<BindGroup>> bindGroups, TL::Span<const uint32_t> dynamicOffset)
    {
        uint32_t count = 0;
        VkDescriptorSet descriptorSets[c_MaxPipelineBindGroupsCount] = {};
        for (auto bindGroupHandle : bindGroups)
        {
            auto bindGroup = m_context->m_bindGroupOwner.Get(bindGroupHandle);
            descriptorSets[count++] = bindGroup->descriptorSet;
        }
        vkCmdBindDescriptorSets(m_commandBuffer, bindPoint, pipelineLayout, 0, count, descriptorSets, (uint32_t)dynamicOffset.size(), dynamicOffset.size() ? dynamicOffset.data() : nullptr);
    }

    void ICommandList::PipelineBarrier(TL::Span<const VkMemoryBarrier2> memoryBarriers,
                                       TL::Span<const VkBufferMemoryBarrier2> bufferBarriers,
                                       TL::Span<const VkImageMemoryBarrier2> imageBarriers)
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
