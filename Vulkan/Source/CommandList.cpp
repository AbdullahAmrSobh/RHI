#include "CommandList.hpp"
#include "Common.hpp"
#include "Context.hpp"
#include "Resources.hpp"
#include "Swapchain.hpp"
#include "VulkanFunctions.hpp"
#include "Barrier.hpp"

#include <RHI/Format.hpp>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    ICommandEncoder::ICommandEncoder(IContext* context)
        : m_context(context)
    {
    }

    ICommandEncoder::~ICommandEncoder(){};

    void ICommandEncoder::BindShaderBindGroups(Handle<CommandList> _commandList, VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        if (bindGroups.empty())
            return;

        TL::Vector<VkDescriptorSet> descriptorSets;
        TL::Vector<uint32_t> dynamicOffset;
        for (auto bindingInfo : bindGroups)
        {
            auto bindGroup = m_context->m_bindGroupOwner.Get(bindingInfo.bindGroup);

            descriptorSets.push_back(bindGroup->descriptorSet);
            dynamicOffset.insert(dynamicOffset.end(), bindingInfo.dynamicOffsets.begin(), bindingInfo.dynamicOffsets.end());
        }
        vkCmdBindDescriptorSets(commandList->handle, bindPoint, pipelineLayout, 0, uint32_t(descriptorSets.size()), descriptorSets.data(), (uint32_t)dynamicOffset.size(), dynamicOffset.data());
    }

    void ICommandEncoder::BindVertexBuffers(Handle<CommandList> _commandList, uint32_t firstBinding, TL::Span<const BufferBindingInfo> bindingInfos)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        if (bindingInfos.empty())
            return;

        TL::Vector<VkBuffer> buffers;
        TL::Vector<VkDeviceSize> offsets;
        for (auto bindingInfo : bindingInfos)
        {
            auto buffer = m_context->m_bufferOwner.Get(bindingInfo.buffer);
            buffers.push_back(buffer->handle);
            offsets.push_back(bindingInfo.offset);
        }
        vkCmdBindVertexBuffers(commandList->handle, firstBinding, (uint32_t)buffers.size(), buffers.data(), offsets.data());
    }

    void ICommandEncoder::BindIndexBuffer(Handle<CommandList> _commandList, const BufferBindingInfo& bindingInfo, VkIndexType indexType)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        if (bindingInfo.buffer == NullHandle)
            return;

        auto buffer = m_context->m_bufferOwner.Get(bindingInfo.buffer);
        vkCmdBindIndexBuffer(commandList->handle, buffer->handle, bindingInfo.offset, indexType);
    }

    void ICommandEncoder::BeginPrimary(Handle<CommandList> _commandList, [[maybe_unused]] RenderGraph& renderGraph, [[maybe_unused]] Handle<Pass> pass)
    {
        ZoneScoped;

        [[maybe_unused]] auto commandList = m_commandBufferPool[_commandList];
    }

    void ICommandEncoder::BeginSecondary(Handle<CommandList> _commandList, [[maybe_unused]] const RenderTargetLayoutDesc& renderTargetLayoutDesc)
    {
        ZoneScoped;

        [[maybe_unused]] auto commandList = m_commandBufferPool[_commandList];
    }

    void ICommandEncoder::PipelineBarrier(Handle<CommandList> _commandList, TL::Span<const VkMemoryBarrier2> memoryBarriers, TL::Span<const VkImageMemoryBarrier2> imageBarriers, TL::Span<const VkBufferMemoryBarrier2> bufferBarriers)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

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
        vkCmdPipelineBarrier2(commandList->handle, &dependencyInfo);
    }

    void ICommandEncoder::Allocate(Flags<CommandFlags> flags, RHI_OUT_PARM TL::Span<Handle<CommandList>> commandLists)
    {
        TL::Vector<VkCommandBuffer> commandBuffer;

        for (uint32_t i = 0; i < commandLists.size(); i++)
        {
            CommandBuffer cmdBuf{};
            cmdBuf.handle = commandBuffer[i];
            cmdBuf.level = flags & CommandFlags::Secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandLists[i] = m_commandBufferPool.Emplace(std::move(cmdBuf));
        }
    }

    void ICommandEncoder::Release([[maybe_unused]] TL::Span<Handle<CommandList>> commandList)
    {
    }

    void ICommandEncoder::Reset([[maybe_unused]] TL::Span<Handle<CommandList>> commandList)
    {
    }

    void ICommandEncoder::Begin(Handle<CommandList> _commandList)
    {
        ZoneScoped;

        [[maybe_unused]] auto commandList = m_commandBufferPool[_commandList];
    }

    void ICommandEncoder::Begin(Handle<CommandList> _commandList, [[maybe_unused]] const CommandListBeginInfo& beginInfo)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        vkEndCommandBuffer(commandList->handle);
    }

    void ICommandEncoder::End(Handle<CommandList> _commandList)
    {
        ZoneScoped;

        [[maybe_unused]] auto commandList = m_commandBufferPool[_commandList];
    }

    void ICommandEncoder::DebugMarkerPush(Handle<CommandList> _commandList, const char* name, ColorValue<float> color)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        (void)name;
        (void)color;

#if RHI_DEBUG
        if (m_context->m_fnTable->m_cmdDebugMarkerBeginEXT)
        {
            VkDebugMarkerMarkerInfoEXT info{};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
            info.pNext = nullptr;
            info.pMarkerName = name;
            info.color[0] = color.r;
            info.color[1] = color.g;
            info.color[2] = color.b;
            info.color[3] = color.a;
            m_context->m_fnTable->m_cmdDebugMarkerBeginEXT(commandList->handle, &info);
        }
#endif
    }

    void ICommandEncoder::DebugMarkerPop(Handle<CommandList> _commandList)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

#if RHI_DEBUG
        if (m_context->m_fnTable->m_cmdDebugMarkerEndEXT)
        {
            m_context->m_fnTable->m_cmdDebugMarkerEndEXT(commandList->handle);
        }
#endif
    }

    void ICommandEncoder::BeginConditionalCommands(Handle<CommandList> _commandList, Handle<Buffer> _buffer, size_t offset, CommandConditionMode inverted)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        auto buffer = m_context->m_bufferOwner.Get(_buffer);

        VkConditionalRenderingBeginInfoEXT beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT;
        beginInfo.pNext = nullptr;
        beginInfo.buffer = buffer->handle;
        beginInfo.offset = offset;
        beginInfo.flags = inverted == CommandConditionMode::Inverted ? VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT : 0u;
        m_context->m_fnTable->m_cmdBeginConditionalRenderingEXT(commandList->handle, &beginInfo);
    }

    void ICommandEncoder::EndConditionalCommands(Handle<CommandList> _commandList)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        m_context->m_fnTable->m_cmdEndConditionalRenderingEXT(commandList->handle);
    }

    void ICommandEncoder::Execute(Handle<CommandList> _commandList, TL::Span<const Handle<CommandList>> commandLists)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        TL::Vector<VkCommandBuffer> commandBuffers;
        commandBuffers.reserve(commandLists.size());
        // for (auto _commandList : commandLists)
        // {
        //     auto commandList = m_commandBufferPool[_commandList];
        //     commandBuffers.push_back(commandList->handle);
        // }
        vkCmdExecuteCommands(commandList->handle, uint32_t(commandBuffers.size()), commandBuffers.data());
    }

    void ICommandEncoder::SetViewport(Handle<CommandList> _commandList, const Viewport& viewport)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        VkViewport vkViewport{};
        vkViewport.x = viewport.offsetX;
        vkViewport.y = viewport.offsetY;
        vkViewport.width = viewport.width;
        vkViewport.height = viewport.height;
        vkViewport.minDepth = viewport.minDepth;
        vkViewport.maxDepth = viewport.maxDepth;
        vkCmdSetViewport(commandList->handle, 0, 1, &vkViewport);
    }

    void ICommandEncoder::SetScissor(Handle<CommandList> _commandList, const Scissor& scissor)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        VkRect2D vkScissor{};
        vkScissor.extent.width = scissor.width;
        vkScissor.extent.height = scissor.height;
        vkScissor.offset.x = scissor.offsetX;
        vkScissor.offset.y = scissor.offsetY;
        vkCmdSetScissor(commandList->handle, 0, 1, &vkScissor);
    }

    void ICommandEncoder::Draw(Handle<CommandList> _commandList, const DrawInfo& drawInfo)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        auto pipeline = m_context->m_graphicsPipelineOwner.Get(drawInfo.pipelineState);
        vkCmdBindPipeline(commandList->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);

        BindShaderBindGroups(_commandList, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, drawInfo.bindGroups);
        BindVertexBuffers(_commandList, 0, drawInfo.vertexBuffers);

        auto parameters = drawInfo.parameters;
        if (drawInfo.indexBuffer.buffer != RHI::NullHandle)
        {
            BindIndexBuffer(_commandList, drawInfo.indexBuffer, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandList->handle, parameters.elementsCount, parameters.instanceCount, parameters.firstElement, parameters.vertexOffset, parameters.firstInstance);
        }
        else
        {
            vkCmdDraw(commandList->handle, parameters.elementsCount, parameters.instanceCount, parameters.firstElement, parameters.firstInstance);
        }
    }

    void ICommandEncoder::Dispatch(Handle<CommandList> _commandList, const DispatchInfo& dispatchInfo)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        auto pipeline = m_context->m_computePipelineOwner.Get(dispatchInfo.pipelineState);
        vkCmdBindPipeline(commandList->handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle);
        if (dispatchInfo.bindGroups.size())
        {
            BindShaderBindGroups(_commandList, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->layout, dispatchInfo.bindGroups);
        }
        auto parameters = dispatchInfo.parameters;
        vkCmdDispatchBase(
            commandList->handle,
            parameters.offsetX,
            parameters.offsetY,
            parameters.offsetZ,
            parameters.countX,
            parameters.countY,
            parameters.countZ);
    }

    void ICommandEncoder::CopyBuffer(Handle<CommandList> _commandList, const BufferCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        auto srcBuffer = m_context->m_bufferOwner.Get(copyInfo.srcBuffer);
        auto dstBuffer = m_context->m_bufferOwner.Get(copyInfo.dstBuffer);

        VkBufferCopy bufferCopy{};
        bufferCopy.srcOffset = copyInfo.srcOffset;
        bufferCopy.dstOffset = copyInfo.dstOffset;
        bufferCopy.size = copyInfo.size;
        vkCmdCopyBuffer(commandList->handle, srcBuffer->handle, dstBuffer->handle, 1, &bufferCopy);
    }

    void ICommandEncoder::CopyImage(Handle<CommandList> _commandList, const ImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        auto srcImage = m_context->m_imageOwner.Get(copyInfo.srcImage);
        auto dstImage = m_context->m_imageOwner.Get(copyInfo.dstImage);

        VkImageCopy imageCopy{};
        imageCopy.srcSubresource = ConvertSubresourceLayer(copyInfo.srcSubresource);
        imageCopy.srcOffset = ConvertOffset3D(copyInfo.srcOffset);
        imageCopy.dstSubresource = ConvertSubresourceLayer(copyInfo.dstSubresource);
        imageCopy.dstOffset = ConvertOffset3D(copyInfo.dstOffset);
        imageCopy.extent = ConvertExtent3D(copyInfo.srcSize);
        vkCmdCopyImage(commandList->handle, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
    }

    void ICommandEncoder::CopyImageToBuffer(Handle<CommandList> _commandList, const BufferImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        auto buffer = m_context->m_bufferOwner.Get(copyInfo.buffer);
        auto image = m_context->m_imageOwner.Get(copyInfo.image);

        VkBufferImageCopy bufferImageCopy{};
        bufferImageCopy.bufferOffset = copyInfo.bufferOffset;
        bufferImageCopy.bufferRowLength = copyInfo.bytesPerRow;
        bufferImageCopy.bufferImageHeight = copyInfo.bytesPerImage;
        bufferImageCopy.imageSubresource = ConvertSubresourceLayer(copyInfo.subresource);
        bufferImageCopy.imageOffset = ConvertOffset3D(copyInfo.imageOffset);
        bufferImageCopy.imageExtent = ConvertExtent3D(copyInfo.imageSize);
        vkCmdCopyImageToBuffer(commandList->handle, image->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer->handle, 1, &bufferImageCopy);
    }

    void ICommandEncoder::CopyBufferToImage(Handle<CommandList> _commandList, const BufferImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto commandList = m_commandBufferPool[_commandList];

        auto buffer = m_context->m_bufferOwner.Get(copyInfo.buffer);
        auto image = m_context->m_imageOwner.Get(copyInfo.image);

        VkBufferImageCopy bufferImageCopy{};
        bufferImageCopy.bufferOffset = copyInfo.bufferOffset;
        bufferImageCopy.bufferRowLength = copyInfo.bytesPerRow;
        bufferImageCopy.bufferImageHeight = copyInfo.bytesPerImage;
        bufferImageCopy.imageSubresource = ConvertSubresourceLayer(copyInfo.subresource);
        bufferImageCopy.imageOffset = ConvertOffset3D(copyInfo.imageOffset);
        bufferImageCopy.imageExtent = ConvertExtent3D(copyInfo.imageSize);
        vkCmdCopyBufferToImage(commandList->handle, buffer->handle, image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
    }

} // namespace RHI::Vulkan
