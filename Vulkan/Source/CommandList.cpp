#include "CommandList.hpp"

#include <RHI/Format.hpp>

#include <tracy/Tracy.hpp>

#include "Barrier.hpp"
#include "Buffer.hpp"
#include "Common.hpp"
#include "Device.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"
#include "Sampler.hpp"
#include "Swapchain.hpp"

namespace RHI::Vulkan
{
    VkImageSubresourceLayers ConvertSubresourceLayer(const ImageSubresourceLayers& subresource)
    {
        auto vkSubresource           = VkImageSubresourceLayers{};
        vkSubresource.aspectMask     = ConvertImageAspect(subresource.imageAspects);
        vkSubresource.mipLevel       = subresource.mipLevel;
        vkSubresource.baseArrayLayer = subresource.arrayBase;
        vkSubresource.layerCount     = subresource.arrayCount;
        return vkSubresource;
    }

    VkResolveModeFlagBits ConvertResolveMode(ResolveMode resolveMode)
    {
        switch (resolveMode)
        {
        case ResolveMode::None: return VK_RESOLVE_MODE_NONE;
        case ResolveMode::Min:  return VK_RESOLVE_MODE_MIN_BIT;
        case ResolveMode::Max:  return VK_RESOLVE_MODE_MAX_BIT;
        case ResolveMode::Avg:  return VK_RESOLVE_MODE_AVERAGE_BIT;
        }
        return {};
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandList
    //////////////////////////////////////////////////////////////////////////////////////////

    ICommandList::ICommandList(IDevice* device, VkCommandBuffer commandBuffer)
        : m_device(device)
        , m_commandBuffer(commandBuffer)
        , m_barriers()
        , m_state()
    {
    }

    ICommandList::~ICommandList() = default;

    void ICommandList::PipelineBarrier(
        TL::Span<const VkMemoryBarrier2>       memoryBarriers,
        TL::Span<const VkBufferMemoryBarrier2> bufferBarriers,
        TL::Span<const VkImageMemoryBarrier2>  imageBarriers)
    {
        ZoneScoped;

        if (memoryBarriers.empty() && bufferBarriers.empty() && imageBarriers.empty()) return;

        VkDependencyInfo dependencyInfo{
            .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext                    = nullptr,
            .dependencyFlags          = 0,
            .memoryBarrierCount       = uint32_t(memoryBarriers.size()),
            .pMemoryBarriers          = memoryBarriers.data(),
            .bufferMemoryBarrierCount = uint32_t(bufferBarriers.size()),
            .pBufferMemoryBarriers    = bufferBarriers.data(),
            .imageMemoryBarrierCount  = uint32_t(imageBarriers.size()),
            .pImageMemoryBarriers     = imageBarriers.data(),
        };
        vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
    }

    void ICommandList::BindShaderBindGroups(
        VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScoped;

        if (bindGroups.empty()) return;

        TL::Vector<VkDescriptorSet> descriptorSets;
        TL::Vector<uint32_t>        dynamicOffset;
        for (auto bindingInfo : bindGroups)
        {
            auto bindGroup = m_device->m_bindGroupOwner.Get(bindingInfo.bindGroup);

            descriptorSets.push_back(bindGroup->descriptorSet);
            dynamicOffset.insert(dynamicOffset.end(), bindingInfo.dynamicOffsets.begin(), bindingInfo.dynamicOffsets.end());
        }
        vkCmdBindDescriptorSets(
            m_commandBuffer,
            bindPoint,
            pipelineLayout,
            0,
            uint32_t(descriptorSets.size()),
            descriptorSets.data(),
            (uint32_t)dynamicOffset.size(),
            dynamicOffset.data());
    }

    void ICommandList::Begin()
    {
        ZoneScoped;

        for (auto& stage : m_barriers)
        {
            stage.memoryBarriers.clear();
            stage.bufferBarriers.clear();
            stage.imageBarriers.clear();
        }

        VkCommandBufferBeginInfo beginInfo{
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext            = nullptr,
            .flags            = 0,
            .pInheritanceInfo = nullptr,
        };
        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    }

    void ICommandList::End()
    {
        ZoneScoped;

        vkEndCommandBuffer(m_commandBuffer);
    }

    void ICommandList::BeginRenderPass(const RenderPassBeginInfo& beginInfo)
    {
        ZoneScoped;

        // auto renderGraph = beginInfo.renderGraph;
        // auto pass        = renderGraph->m_passPool.Get(beginInfo.pass);
        // for (uint32_t i = 0; i < pass->m_imageAttachments.size(); i++)
        // {
        //     auto& node         = pass->m_imageAttachments[i];
        //     auto  attachment   = renderGraph->m_imagePools.Get(node->image);
        //     auto  subresources = VkImageSubresourceRange{
        //         /// @fixme: deduce from actual attachment
        //          .aspectMask = static_cast<VkImageAspectFlags>(
        //             ((attachment->info.usageFlags & ImageUsage::DepthStencil) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
        //                                                                        : VK_IMAGE_ASPECT_COLOR_BIT)),
        //          .baseMipLevel   = 0,
        //          .levelCount     = VK_REMAINING_MIP_LEVELS,
        //          .baseArrayLayer = 0,
        //          .layerCount     = VK_REMAINING_ARRAY_LAYERS,
        //     };

        //     auto imageHandle = renderGraph->GetImage(node->image);
        //     auto image       = m_device->m_imageOwner.Get(imageHandle);
        //     auto swapchain   = (ISwapchain*)attachment->swapchain;

        //     // if (node->prev)
        //     // {
        //     //     auto srcState = GetImageStageAccess(*node->prev);
        //     //     auto dstState = GetImageStageAccess(*node);
        //     //     auto barrier  = CreateImageBarrier(image->handle, subresources, srcState, dstState);
        //     //     m_barriers[BarrierSlot::Priloge].imageBarriers.push_back(barrier);
        //     // }
        //     // else
        //     {
        //         // auto dstState = GetImageStageAccess(*node);
        //         auto dstState =
        //             ImageStageAccess{VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0};
        //         if (image->subresources.imageAspects & ImageAspect::Depth) dstState.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        //         auto srcState = ImageStageAccess{VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0};
        //         auto barrier  = CreateImageBarrier(image->handle, subresources, srcState, dstState);
        //         m_barriers[BarrierSlot::Priloge].imageBarriers.push_back(barrier);
        //     }

        //     if (node->next == nullptr && swapchain != nullptr)
        //     {
        //         // auto srcState = GetImageStageAccess(*node);
        //         auto srcState =
        //             ImageStageAccess{VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0};
        //         if (image->subresources.imageAspects & ImageAspect::Depth) srcState.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        //         auto dstState =
        //             ImageStageAccess{VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_NONE, 0};
        //         auto barrier = CreateImageBarrier(image->handle, subresources, srcState, dstState);
        //         m_barriers[BarrierSlot::Epiloge].imageBarriers.push_back(barrier);
        //     }
        // }

        PipelineBarrier(
            m_barriers[BarrierSlot::Priloge].memoryBarriers,
            m_barriers[BarrierSlot::Priloge].bufferBarriers,
            m_barriers[BarrierSlot::Priloge].imageBarriers);

        TL::Vector<VkRenderingAttachmentInfo>   colorAttachmentInfos;
        TL::Optional<VkRenderingAttachmentInfo> depthAttachmentInfo;
        TL::Optional<VkRenderingAttachmentInfo> stencilAttachmentInfo;

        for (auto passAttachment : beginInfo.colorAttachments)
        {
            IImage* colorAttachment = m_device->m_imageOwner.Get(passAttachment.attachment);
            IImage* resolveAttachment =
                passAttachment.resolveAttachment ? m_device->m_imageOwner.Get(passAttachment.resolveAttachment) : nullptr;

            VkClearColorValue colorValue = {
                .float32 =
                    {
                        passAttachment.loadStoreOperations.clearValue.f32.r,
                        passAttachment.loadStoreOperations.clearValue.f32.g,
                        passAttachment.loadStoreOperations.clearValue.f32.b,
                        passAttachment.loadStoreOperations.clearValue.f32.a,
                    },
            };

            VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext              = nullptr,
                .imageView          = colorAttachment->viewHandle,
                .imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode        = ConvertResolveMode(passAttachment.resolveMode),
                .resolveImageView   = resolveAttachment ? resolveAttachment->viewHandle : VK_NULL_HANDLE,
                .resolveImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .loadOp             = ConvertLoadOp(passAttachment.loadStoreOperations.loadOperation),
                .storeOp            = ConvertStoreOp(passAttachment.loadStoreOperations.storeOperation),
                .clearValue         = {.color = colorValue},
            };

            colorAttachmentInfos.push_back(colorAttachmentInfo);
        }

        if (beginInfo.depthStenciAttachments)
        {
            auto passAttachment = beginInfo.depthStenciAttachments.value();

            IImage* depthStencilAttachment = m_device->m_imageOwner.Get(passAttachment.attachment);
            IImage* resolveAttachment =
                passAttachment.resolveAttachment ? m_device->m_imageOwner.Get(passAttachment.resolveAttachment) : nullptr;

            auto clearValue = VkClearDepthStencilValue{
                .depth   = beginInfo.depthStenciAttachments->loadStoreOperations.clearValue.depthStencil.depthValue,
                .stencil = beginInfo.depthStenciAttachments->loadStoreOperations.clearValue.depthStencil.stencilValue,
            };

            if (depthStencilAttachment->subresources.imageAspects & ImageAspect::Depth)
            {
                depthAttachmentInfo = VkRenderingAttachmentInfo{
                    .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext              = nullptr,
                    .imageView          = depthStencilAttachment->viewHandle,
                    .imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .resolveMode        = ConvertResolveMode(passAttachment.resolveMode),
                    .resolveImageView   = resolveAttachment ? resolveAttachment->viewHandle : VK_NULL_HANDLE,
                    .resolveImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .loadOp             = ConvertLoadOp(passAttachment.loadStoreOperations.loadOperation),
                    .storeOp            = ConvertStoreOp(passAttachment.loadStoreOperations.storeOperation),
                    .clearValue         = {.depthStencil = clearValue},
                };
            }

            if (depthStencilAttachment->subresources.imageAspects & ImageAspect::Stencil)
            {
                depthAttachmentInfo = VkRenderingAttachmentInfo{
                    .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext              = nullptr,
                    .imageView          = depthStencilAttachment->viewHandle,
                    .imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .resolveMode        = ConvertResolveMode(passAttachment.resolveMode),
                    .resolveImageView   = resolveAttachment->viewHandle ? resolveAttachment->viewHandle : VK_NULL_HANDLE,
                    .resolveImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .loadOp             = ConvertLoadOp(passAttachment.loadStoreOperations.stencilLoadOperation),
                    .storeOp            = ConvertStoreOp(passAttachment.loadStoreOperations.storeOperation),
                    .clearValue         = {.depthStencil = clearValue},
                };
            }
        }

        VkRenderingInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext = nullptr,
            .flags = {},
            .renderArea =
                {
                    .offset = {beginInfo.renderArea.offsetX, beginInfo.renderArea.offsetY},
                    .extent = {beginInfo.renderArea.width, beginInfo.renderArea.height},
                },
            .layerCount           = 1,
            .viewMask             = 0,
            .colorAttachmentCount = (uint32_t)colorAttachmentInfos.size(),
            .pColorAttachments    = colorAttachmentInfos.data(),
            .pDepthAttachment     = depthAttachmentInfo.has_value() ? &depthAttachmentInfo.value() : nullptr,
            .pStencilAttachment   = stencilAttachmentInfo.has_value() ? &stencilAttachmentInfo.value() : nullptr,
        };
        vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
    }

    void ICommandList::EndRenderPass()
    {
        ZoneScoped;

        vkCmdEndRendering(m_commandBuffer);

        PipelineBarrier(
            m_barriers[BarrierSlot::Epiloge].memoryBarriers,
            m_barriers[BarrierSlot::Epiloge].bufferBarriers,
            m_barriers[BarrierSlot::Epiloge].imageBarriers);
    }

    void ICommandList::DebugMarkerPush([[maybe_unused]] const char* name, [[maybe_unused]] ColorValue<float> color)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (auto fn = m_device->m_pfn.m_vkCmdBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT info{
                .sType      = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
                .pNext      = nullptr,
                .pLabelName = name,
                .color      = {color.r, color.g, color.b, color.a},
            };
            fn(m_commandBuffer, &info);
        }
#endif
    }

    void ICommandList::DebugMarkerPop()
    {
        ZoneScoped;

#if RHI_DEBUG
        if (auto fn = m_device->m_pfn.m_vkCmdEndDebugUtilsLabelEXT)
        {
            fn(m_commandBuffer);
        }
#endif
    }

    void ICommandList::BeginConditionalCommands(Handle<Buffer> handle, size_t offset, bool inverted)
    {
        ZoneScoped;

        auto buffer = m_device->m_bufferOwner.Get(handle);

        VkConditionalRenderingBeginInfoEXT beginInfo{
            .sType  = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT,
            .pNext  = nullptr,
            .buffer = buffer->handle,
            .offset = offset,
            .flags  = inverted ? VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT : 0u,
        };
        m_device->m_pfn.m_vkCmdBeginConditionalRenderingEXT(m_commandBuffer, &beginInfo);
    }

    void ICommandList::EndConditionalCommands()
    {
        ZoneScoped;

        m_device->m_pfn.m_vkCmdEndConditionalRenderingEXT(m_commandBuffer);
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

    void ICommandList::BindGraphicsPipeline(Handle<GraphicsPipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScoped;

        if (pipelineState == NullHandle)
        {
            m_state.isGraphicsPipelineBound = false;
            return;
        }

        auto pipeline = m_device->m_graphicsPipelineOwner.Get(pipelineState);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
        BindShaderBindGroups(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, bindGroups);
        m_state.isGraphicsPipelineBound = true;
    }

    void ICommandList::BindComputePipeline(Handle<ComputePipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScoped;

        if (pipelineState == NullHandle)
        {
            m_state.isComputePipelineBound = false;
            return;
        }

        auto pipeline = m_device->m_computePipelineOwner.Get(pipelineState);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle);
        BindShaderBindGroups(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->layout, bindGroups);
        m_state.isComputePipelineBound = true;
    }

    void ICommandList::SetViewport(const Viewport& viewport)
    {
        ZoneScoped;

        VkViewport vkViewport{
            .x        = viewport.offsetX,
            .y        = viewport.offsetY,
            .width    = viewport.width,
            .height   = viewport.height,
            .minDepth = viewport.minDepth,
            .maxDepth = viewport.maxDepth,
        };
        vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
        m_state.hasViewportSet = true;
    }

    void ICommandList::SetSicssor(const Scissor& scissor)
    {
        ZoneScoped;

        VkRect2D vkScissor{
            .offset = {scissor.offsetX, scissor.offsetY},
            .extent = {scissor.width, scissor.height},
        };
        vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
        m_state.hasScissorSet = true;
    }

    void ICommandList::BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers)
    {
        ZoneScoped;

        TL::Vector<VkBuffer>     buffers;
        TL::Vector<VkDeviceSize> offsets;
        for (auto bindingInfo : vertexBuffers)
        {
            auto buffer = m_device->m_bufferOwner.Get(bindingInfo.buffer);
            buffers.push_back(buffer->handle);
            offsets.push_back(bindingInfo.offset);
        }
        vkCmdBindVertexBuffers(m_commandBuffer, firstBinding, (uint32_t)buffers.size(), buffers.data(), offsets.data());
        m_state.hasVertexBuffer = true;
    }

    void ICommandList::BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType)
    {
        ZoneScoped;

        auto buffer = m_device->m_bufferOwner.Get(indexBuffer.buffer);
        vkCmdBindIndexBuffer(
            m_commandBuffer,
            buffer->handle,
            indexBuffer.offset,
            indexType == IndexType::uint32 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
        m_state.hasIndexBuffer = true;
    }

    void ICommandList::Draw(const DrawParameters& parameters)
    {
        ZoneScoped;

        if (m_state.hasIndexBuffer)
        {
            vkCmdDrawIndexed(
                m_commandBuffer,
                parameters.elementsCount,
                parameters.instanceCount,
                parameters.firstElement,
                parameters.vertexOffset,
                parameters.firstInstance);
        }
        else
        {
            vkCmdDraw(
                m_commandBuffer, parameters.elementsCount, parameters.instanceCount, parameters.firstElement, parameters.firstInstance);
        }
    }

    void ICommandList::Dispatch(const DispatchParameters& parameters)
    {
        ZoneScoped;

        vkCmdDispatchBase(
            m_commandBuffer,
            parameters.offsetX,
            parameters.offsetY,
            parameters.offsetZ,
            parameters.countX,
            parameters.countY,
            parameters.countZ);
    }

    void ICommandList::CopyBuffer(const BufferCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto srcBuffer = m_device->m_bufferOwner.Get(copyInfo.srcBuffer);
        auto dstBuffer = m_device->m_bufferOwner.Get(copyInfo.dstBuffer);

        VkBufferCopy bufferCopy{
            .srcOffset = copyInfo.srcOffset,
            .dstOffset = copyInfo.dstOffset,
            .size      = copyInfo.size,
        };
        vkCmdCopyBuffer(m_commandBuffer, srcBuffer->handle, dstBuffer->handle, 1, &bufferCopy);
    }

    void ICommandList::CopyImage(const ImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto srcImage = m_device->m_imageOwner.Get(copyInfo.srcImage);
        auto dstImage = m_device->m_imageOwner.Get(copyInfo.dstImage);

        VkImageCopy imageCopy{
            .srcSubresource = ConvertSubresourceLayer(copyInfo.srcSubresource),
            .srcOffset      = ConvertOffset3D(copyInfo.srcOffset),
            .dstSubresource = ConvertSubresourceLayer(copyInfo.dstSubresource),
            .dstOffset      = ConvertOffset3D(copyInfo.dstOffset),
            .extent         = ConvertExtent3D(copyInfo.srcSize),
        };
        vkCmdCopyImage(
            m_commandBuffer,
            srcImage->handle,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstImage->handle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageCopy);
    }

    void ICommandList::CopyImageToBuffer(const BufferImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto buffer = m_device->m_bufferOwner.Get(copyInfo.buffer);
        auto image  = m_device->m_imageOwner.Get(copyInfo.image);

        VkBufferImageCopy bufferImageCopy{
            .bufferOffset      = copyInfo.bufferOffset,
            .bufferRowLength   = copyInfo.bytesPerRow,
            .bufferImageHeight = copyInfo.bytesPerImage,
            .imageSubresource  = ConvertSubresourceLayer(copyInfo.subresource),
            .imageOffset       = ConvertOffset3D(copyInfo.imageOffset),
            .imageExtent       = ConvertExtent3D(copyInfo.imageSize),
        };
        vkCmdCopyImageToBuffer(m_commandBuffer, image->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer->handle, 1, &bufferImageCopy);
    }

    void ICommandList::CopyBufferToImage(const BufferImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto buffer = m_device->m_bufferOwner.Get(copyInfo.buffer);
        auto image  = m_device->m_imageOwner.Get(copyInfo.image);

        VkBufferImageCopy bufferImageCopy{
            .bufferOffset      = copyInfo.bufferOffset,
            .bufferRowLength   = copyInfo.bytesPerRow,
            .bufferImageHeight = copyInfo.bytesPerImage,
            .imageSubresource  = ConvertSubresourceLayer(copyInfo.subresource),
            .imageOffset       = ConvertOffset3D(copyInfo.imageOffset),
            .imageExtent       = ConvertExtent3D(copyInfo.imageSize),
        };
        vkCmdCopyBufferToImage(m_commandBuffer, buffer->handle, image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
    }

    void ICommandList::BlitImage([[maybe_unused]] const ImageBlitInfo& blitInfo)
    {
        ZoneScoped;

        TL_UNREACHABLE();
    }

} // namespace RHI::Vulkan
