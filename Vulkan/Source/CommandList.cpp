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
    ICommandPool::ICommandPool(IContext* context)
        : m_context(context)
    {
    }

    ICommandPool::~ICommandPool()
    {
        for (auto commandPool : m_commandPools)
        {
            vkDestroyCommandPool(m_context->m_device, commandPool, nullptr);
        }
    }

    VkResult ICommandPool::Init(CommandPoolFlags flags)
    {
        (void)flags;

        for (uint32_t queueType = 0; queueType < uint32_t(QueueType::Count); queueType++)
        {
            VkCommandPoolCreateInfo createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            createInfo.queueFamilyIndex = m_context->GetQueueFamilyIndex(QueueType(queueType));
            auto result = vkCreateCommandPool(m_context->m_device, &createInfo, nullptr, &m_commandPools[queueType]);
            RHI_ASSERT(result == VK_SUCCESS);
            if (result != VK_SUCCESS)
            {
                return result;
            }
        }

        return VK_SUCCESS;
    }

    void ICommandPool::Reset()
    {
        for (auto commandPool : m_commandPools)
        {
            vkResetCommandPool(m_context->m_device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        }
    }

    TL::Vector<CommandList*> ICommandPool::Allocate(QueueType queueType, CommandListLevel level, uint32_t count)
    {
        auto commandPool = m_commandPools[uint32_t(queueType)];
        auto commandBuffers = AllocateCommandBuffers(commandPool, count, level == CommandListLevel::Primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY);
        TL::Vector<CommandList*> commandLists;
        for (auto commandBuffer : commandBuffers)
        {
            commandLists.push_back(new ICommandList(m_context, commandPool, commandBuffer));
        }
        return commandLists;
    }

    void ICommandPool::Release(TL::Span<const CommandList* const> commandLists)
    {
        for (auto& _commandList : commandLists)
        {
            ICommandList* commandList = (ICommandList*)_commandList;
            VkDevice device = m_context->m_device;
            m_context->PushDeferCommand([=]()
            {
                vkFreeCommandBuffers(device, commandList->m_commandPool, 1, &commandList->m_commandBuffer);
                delete commandList;
            });
        }
    }

    TL::Vector<VkCommandBuffer> ICommandPool::AllocateCommandBuffers(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level)
    {
        TL::Vector<VkCommandBuffer> commandBuffers;
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

    void ICommandList::Begin(RenderGraph& renderGraph, Handle<Pass> passHandle, TL::Span<const ClearValue> colorClearValues, const DepthStencilValue& depthStencilClearValue)
    {
        ZoneScoped;

        (void)colorClearValues;       // todo: use
        (void)depthStencilClearValue; // todo: use

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
            PipelineBarrier({}, m_passSubmitData->bufferBarriers[BarrierType::PostPass], m_passSubmitData->imageBarriers[BarrierType::PostPass]);
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

        BindShaderBindGroups(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, drawInfo.bindGroups);

        if (drawInfo.vertexBuffers.empty() == false)
        {
            TL::Vector<VkBuffer> vertexBuffers;
            TL::Vector<VkDeviceSize> vertexBufferOffsets;
            for (auto bindingInfo : drawInfo.vertexBuffers)
            {
                auto buffer = m_context->m_bufferOwner.Get(bindingInfo.buffer);
                vertexBuffers.push_back(buffer->handle);
                vertexBufferOffsets.push_back(bindingInfo.offset);
            }
            vkCmdBindVertexBuffers(m_commandBuffer, 0, (uint32_t)vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());
        }

        auto parameters = drawInfo.parameters;
        if (drawInfo.indexBuffer.buffer != RHI::NullHandle)
        {
            auto buffer = m_context->m_bufferOwner.Get(drawInfo.indexBuffer.buffer);
            vkCmdBindIndexBuffer(m_commandBuffer, buffer->handle, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(m_commandBuffer, parameters.elementsCount, parameters.instanceCount, parameters.firstElement, parameters.vertexOffset, parameters.firstInstance);
        }
        else
        {
            vkCmdDraw(m_commandBuffer, parameters.elementsCount, parameters.instanceCount, parameters.firstElement, parameters.firstInstance);
        }
    }

    void ICommandList::Dispatch(const DispatchInfo& dispatchInfo)
    {
        ZoneScoped;

        auto pipeline = m_context->m_computePipelineOwner.Get(dispatchInfo.pipelineState);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle);
        if (dispatchInfo.bindGroups.size())
        {
            BindShaderBindGroups(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->layout, dispatchInfo.bindGroups);
        }
        auto parameters = dispatchInfo.parameters;
        vkCmdDispatchBase(m_commandBuffer, parameters.offsetX, parameters.offsetY, parameters.offsetZ, parameters.countX, parameters.countY, parameters.countZ);
    }

    void ICommandList::CopyBuffer(const BufferCopyInfo& copyInfo)
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

    void ICommandList::CopyImage(const ImageCopyInfo& copyInfo)
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

    void ICommandList::CopyImageToBuffer(const BufferImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto buffer = m_context->m_bufferOwner.Get(copyInfo.buffer);
        auto image = m_context->m_imageOwner.Get(copyInfo.image);

        VkBufferImageCopy bufferImageCopy{};
        bufferImageCopy.bufferOffset = copyInfo.bufferOffset;
        bufferImageCopy.bufferRowLength = copyInfo.bytesPerRow;
        bufferImageCopy.bufferImageHeight = copyInfo.bytesPerImage;
        bufferImageCopy.imageSubresource = ConvertSubresourceLayer(copyInfo.subresource);
        bufferImageCopy.imageOffset = ConvertOffset3D(copyInfo.imageOffset);
        bufferImageCopy.imageExtent = ConvertExtent3D(copyInfo.imageSize);
        vkCmdCopyImageToBuffer(m_commandBuffer, image->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer->handle, 1, &bufferImageCopy);
    }

    void ICommandList::CopyBufferToImage(const BufferImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto buffer = m_context->m_bufferOwner.Get(copyInfo.buffer);
        auto image = m_context->m_imageOwner.Get(copyInfo.image);

        VkBufferImageCopy bufferImageCopy{};
        bufferImageCopy.bufferOffset = copyInfo.bufferOffset;
        bufferImageCopy.bufferRowLength = copyInfo.bytesPerRow;
        bufferImageCopy.bufferImageHeight = copyInfo.bytesPerImage;
        bufferImageCopy.imageSubresource = ConvertSubresourceLayer(copyInfo.subresource);
        bufferImageCopy.imageOffset = ConvertOffset3D(copyInfo.imageOffset);
        bufferImageCopy.imageExtent = ConvertExtent3D(copyInfo.imageSize);
        vkCmdCopyBufferToImage(m_commandBuffer, buffer->handle, image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
    }

    void ICommandList::BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
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
        vkCmdBindDescriptorSets(m_commandBuffer, bindPoint, pipelineLayout, 0, uint32_t(descriptorSets.size()), descriptorSets.data(), (uint32_t)dynamicOffset.size(), dynamicOffset.data());
    }

    void ICommandList::PipelineBarrier(TL::Span<const VkMemoryBarrier2> memoryBarriers, TL::Span<const VkBufferMemoryBarrier2> bufferBarriers, TL::Span<const VkImageMemoryBarrier2> imageBarriers)
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
