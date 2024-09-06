#include "CommandList.hpp"
#include "Common.hpp"
#include "Context.hpp"
#include "Swapchain.hpp"
#include "Barrier.hpp"
#include "Image.hpp"
#include "Buffer.hpp"
#include "Pipeline.hpp"
#include "Sampler.hpp"

#include <RHI/Format.hpp>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    VkImageSubresourceLayers ConvertSubresourceLayer(const ImageSubresourceLayers& subresource)
    {
        auto vkSubresource = VkImageSubresourceLayers{};
        vkSubresource.aspectMask = ConvertImageAspect(subresource.imageAspects);
        vkSubresource.mipLevel = subresource.mipLevel;
        vkSubresource.baseArrayLayer = subresource.arrayBase;
        vkSubresource.layerCount = subresource.arrayCount;
        return vkSubresource;
    }

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

    ResultCode ICommandPool::Init(CommandPoolFlags flags)
    {
        for (uint32_t queueType = 0; queueType < uint32_t(QueueType::Count); queueType++)
        {
            VkCommandPoolCreateInfo createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = ConvertCommandPoolFlags(flags);
            createInfo.queueFamilyIndex = m_context->m_queue[queueType].GetFamilyIndex();
            TryValidateVk(vkCreateCommandPool(m_context->m_device, &createInfo, nullptr, &m_commandPools[queueType]));
        }

        return ResultCode::Success;
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
            commandLists.push_back(new ICommandList(m_context, commandBuffer));
        }
        return commandLists;
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

    ICommandList::ICommandList(IContext* context, VkCommandBuffer commandBuffer)
        : m_context(context)
        , m_commandBuffer(commandBuffer)
        , m_barriers()
        , m_waitSemaphores()
        , m_signalSemaphores()
        , m_isRenderPassStarted()
    {
    }

    ICommandList::~ICommandList() = default;

    void ICommandList::BeginRendering(
        VkRect2D renderingArea,
        TL::Span<const VkRenderingAttachmentInfo> colorAttachments,
        VkRenderingAttachmentInfo* depthAttachment,
        VkRenderingAttachmentInfo* stencilAttachment)
    {
        TL_ASSERT(m_isRenderPassStarted == false); // cannot start a new render pass inside another

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.pNext = nullptr;
        renderingInfo.flags = {};
        renderingInfo.renderArea = renderingArea;
        renderingInfo.layerCount = 1;
        renderingInfo.viewMask = 0;
        renderingInfo.colorAttachmentCount = (uint32_t)colorAttachments.size();
        renderingInfo.pColorAttachments = colorAttachments.data();
        renderingInfo.pDepthAttachment = depthAttachment;
        renderingInfo.pStencilAttachment = stencilAttachment;
        vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
        m_isRenderPassStarted = true;
    }

    void ICommandList::EndRendedring()
    {
        if (m_isRenderPassStarted)
        {
            vkCmdEndRendering(m_commandBuffer);
            m_isRenderPassStarted = false;
        }
    }

    void ICommandList::PipelineBarrier(TL::Span<const VkMemoryBarrier2> memoryBarriers, TL::Span<const VkBufferMemoryBarrier2> bufferBarriers, TL::Span<const VkImageMemoryBarrier2> imageBarriers)
    {
        if (memoryBarriers.empty() && bufferBarriers.empty() && imageBarriers.empty())
            return;

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

    void ICommandList::BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> bindingInfos)
    {
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
        vkCmdBindVertexBuffers(m_commandBuffer, firstBinding, (uint32_t)buffers.size(), buffers.data(), offsets.data());
    }

    void ICommandList::BindIndexBuffer(const BufferBindingInfo& bindingInfo, VkIndexType indexType)
    {
        if (bindingInfo.buffer == NullHandle)
            return;

        auto buffer = m_context->m_bufferOwner.Get(bindingInfo.buffer);
        vkCmdBindIndexBuffer(m_commandBuffer, buffer->handle, bindingInfo.offset, indexType);
    }

    void ICommandList::Begin()
    {
        ZoneScoped;

        m_isRenderPassStarted = false;
        for (auto& stage : m_barriers)
        {
            stage.memoryBarriers.clear();
            stage.bufferBarriers.clear();
            stage.imageBarriers.clear();
        }

        m_waitSemaphores.clear();
        m_signalSemaphores.clear();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    }

    void ICommandList::Begin(const CommandListBeginInfo& beginInfo)
    {
        ZoneScoped;

        Begin();

        auto renderGraph = beginInfo.renderGraph;
        auto pass = renderGraph->m_passPool.Get(beginInfo.pass);
        for (uint32_t i = 0; i < pass->m_imageAttachments.size(); i++)
        {
            auto& node = pass->m_imageAttachments[i];
            node->loadStoreOperation = beginInfo.loadStoreOperations[i];
            auto attachment = renderGraph->m_imageAttachmentPool.Get(node->attachment);
            auto subresources = ConvertSubresourceRange(node->viewInfo.subresources);
            auto imageHandle = renderGraph->GetImage(node->attachment);
            auto image = m_context->m_imageOwner.Get(imageHandle);
            auto swapchain = (ISwapchain*)attachment->swapchain;

            if (node->prev)
            {
                auto srcState = GetImageStageAccess(*node->prev);
                auto dstState = GetImageStageAccess(*node);
                auto barrier = CreateImageBarrier(image->handle, subresources, srcState, dstState);
                m_barriers[BarrierSlot::Priloge].imageBarriers.push_back(barrier);
            }
            else
            {
                auto dstState = GetImageStageAccess(*node);
                auto srcState = ImageStageAccess{ VK_IMAGE_LAYOUT_UNDEFINED, dstState.stage, 0 };
                auto barrier = CreateImageBarrier(image->handle, subresources, srcState, dstState);
                m_barriers[BarrierSlot::Priloge].imageBarriers.push_back(barrier);

                if (swapchain != nullptr)
                {
                    m_waitSemaphores.push_back(CreateSemaphoreSubmitInfo(swapchain->GetImageAcquiredSemaphore(), srcState.stage));
                }
            }

            if (node->next == nullptr && swapchain != nullptr)
            {
                auto srcState = GetImageStageAccess(*node);
                auto dstState = PIPELINE_IMAGE_BARRIER_PRESENT_SRC;
                auto barrier = CreateImageBarrier(image->handle, subresources, srcState, dstState);
                m_barriers[BarrierSlot::Epiloge].imageBarriers.push_back(barrier);
                m_signalSemaphores.push_back(CreateSemaphoreSubmitInfo(swapchain->GetImageSignaledSemaphore(), dstState.stage));
            }
        }

        PipelineBarrier(
            m_barriers[BarrierSlot::Priloge].memoryBarriers,
            m_barriers[BarrierSlot::Priloge].bufferBarriers,
            m_barriers[BarrierSlot::Priloge].imageBarriers);

        TL::Vector<VkRenderingAttachmentInfo> colorAttachments;
        VkRenderingAttachmentInfo depthAttachment, stencilAttachment;
        bool hasDepthAttachment = false, hasStencilAttachment = false;

        VkRect2D renderingArea{
            .offset = {},
            .extent = ConvertExtent2D(pass->m_renderTargetSize),
        };

        if (pass->m_colorAttachments.empty() == false)
        {
            for (auto colorAttachment : pass->m_colorAttachments)
            {
                auto imageViewHandle = renderGraph->PassGetImageView(colorAttachment->pass, colorAttachment->attachment);
                auto imageView = m_context->m_imageViewOwner.Get(imageViewHandle);
                colorAttachments.push_back(
                    CreateColorAttachment(imageView->handle, colorAttachment->loadStoreOperation));
            }

            if (auto depthStencilAttachment = pass->m_depthStencilAttachment)
            {
                auto imageViewHandle = renderGraph->PassGetImageView(depthStencilAttachment->pass, depthStencilAttachment->attachment);
                auto imageView = m_context->m_imageViewOwner.Get(imageViewHandle);

                if (depthStencilAttachment->usage & ImageUsage::Depth)
                {
                    depthAttachment = CreateDepthAttachment(imageView->handle, depthStencilAttachment->loadStoreOperation);
                    hasDepthAttachment = true;
                }

                if (depthStencilAttachment->usage & ImageUsage::Stencil)
                {
                    stencilAttachment = CreateStencilAttachment(imageView->handle, depthStencilAttachment->loadStoreOperation);
                    hasStencilAttachment = true;
                }
            }

            BeginRendering(
                renderingArea,
                colorAttachments,
                hasDepthAttachment ? &depthAttachment : nullptr,
                hasStencilAttachment ? &stencilAttachment : nullptr);
        }
    }

    void ICommandList::End()
    {
        ZoneScoped;

        EndRendedring();

        PipelineBarrier(
            m_barriers[BarrierSlot::Epiloge].memoryBarriers,
            m_barriers[BarrierSlot::Epiloge].bufferBarriers,
            m_barriers[BarrierSlot::Epiloge].imageBarriers);

        vkEndCommandBuffer(m_commandBuffer);
    }

    void ICommandList::DebugMarkerPush([[maybe_unused]] const char* name, [[maybe_unused]] ColorValue<float> color)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (auto fn = m_context->m_pfn.m_vkCmdBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT info{};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
            info.pNext = nullptr;
            info.pLabelName = name;
            info.color[0] = color.r;
            info.color[1] = color.g;
            info.color[2] = color.b;
            info.color[3] = color.a;
            fn(m_commandBuffer, &info);
        }
#endif
    }

    void ICommandList::DebugMarkerPop()
    {
        ZoneScoped;

#if RHI_DEBUG
        if (auto fn = m_context->m_pfn.m_vkCmdEndDebugUtilsLabelEXT)
        {
            fn(m_commandBuffer);
        }
#endif
    }

    void ICommandList::BeginConditionalCommands(Handle<Buffer> handle, size_t offset, bool inverted)
    {
        ZoneScoped;

        auto buffer = m_context->m_bufferOwner.Get(handle);

        VkConditionalRenderingBeginInfoEXT beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT;
        beginInfo.pNext = nullptr;
        beginInfo.buffer = buffer->handle;
        beginInfo.offset = offset;
        beginInfo.flags = inverted ? VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT : 0u;
        m_context->m_pfn.m_vkCmdBeginConditionalRenderingEXT(m_commandBuffer, &beginInfo);
    }

    void ICommandList::EndConditionalCommands()
    {
        ZoneScoped;

        m_context->m_pfn.m_vkCmdEndConditionalRenderingEXT(m_commandBuffer);
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
        BindVertexBuffers(0, drawInfo.vertexBuffers);

        auto parameters = drawInfo.parameters;
        if (drawInfo.indexBuffer.buffer != RHI::NullHandle)
        {
            BindIndexBuffer(drawInfo.indexBuffer, VK_INDEX_TYPE_UINT32);
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
} // namespace RHI::Vulkan
