#include "CommandList.hpp"

#include <RHI/Format.hpp>

#include <tracy/Tracy.hpp>

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
    {
    }

    ICommandList::~ICommandList() = default;

    void ICommandList::AddPipelineBarriers(const PipelineBarriers& barriers)
    {
        ZoneScoped;

        TL_ASSERT(!(barriers.memoryBarriers.empty() && barriers.bufferBarriers.empty() && barriers.imageBarriers.empty()));

        VkDependencyInfo dependencyInfo{
            .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext                    = nullptr,
            .dependencyFlags          = 0,
            .memoryBarrierCount       = uint32_t(barriers.memoryBarriers.size()),
            .pMemoryBarriers          = barriers.memoryBarriers.data(),
            .bufferMemoryBarrierCount = uint32_t(barriers.bufferBarriers.size()),
            .pBufferMemoryBarriers    = barriers.bufferBarriers.data(),
            .imageMemoryBarrierCount  = uint32_t(barriers.imageBarriers.size()),
            .pImageMemoryBarriers     = barriers.imageBarriers.data(),
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

    void ICommandList::BeginPass(const Pass& pass)
    {
        m_isInsideRenderPass = true;

        TL::Vector<VkRenderingAttachmentInfo>   colorAttachmentInfos;
        TL::Optional<VkRenderingAttachmentInfo> depthAttachmentInfo;
        TL::Optional<VkRenderingAttachmentInfo> stencilAttachmentInfo;

        for (const auto& passAttachment : pass.GetColorAttachment())
        {
            auto colorAttachmentHandle   = passAttachment.attachment->GetImage();
            auto resolveAttachmentHandle = passAttachment.resolveAttachment ? passAttachment.resolveAttachment->GetImage() : NullHandle;

            IImage* colorAttachment   = m_device->m_imageOwner.Get(colorAttachmentHandle);
            IImage* resolveAttachment = passAttachment.resolveAttachment ? m_device->m_imageOwner.Get(resolveAttachmentHandle) : nullptr;

            VkClearColorValue colorValue = {
                .float32 =
                    {
                        passAttachment.clearValue.f32.r,
                        passAttachment.clearValue.f32.g,
                        passAttachment.clearValue.f32.b,
                        passAttachment.clearValue.f32.a,
                    },
            };

            colorAttachmentInfos.push_back({
                .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext              = nullptr,
                .imageView          = colorAttachment->viewHandle,
                .imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode        = ConvertResolveMode(passAttachment.resolveMode),
                .resolveImageView   = resolveAttachment ? resolveAttachment->viewHandle : VK_NULL_HANDLE,
                .resolveImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .loadOp             = ConvertLoadOp(passAttachment.loadOperation),
                .storeOp            = ConvertStoreOp(passAttachment.storeOperation),
                .clearValue         = {.color = colorValue},
            });
        }

        if (auto passAttachment = pass.GetDepthStencilAttachment())
        {
            auto depthStencilHandle      = passAttachment->attachment->GetImage();
            auto resolveAttachmentHandle = passAttachment->resolveAttachment ? passAttachment->resolveAttachment->GetImage() : NullHandle;

            IImage* depthStencilAttachment = m_device->m_imageOwner.Get(depthStencilHandle);
            IImage* resolveAttachment      = passAttachment->resolveAttachment ? m_device->m_imageOwner.Get(resolveAttachmentHandle) : nullptr;

            VkClearDepthStencilValue clearValue{passAttachment->clearValue.depthStencil.depthValue, passAttachment->clearValue.depthStencil.stencilValue};

            auto formatInfo = GetFormatInfo(passAttachment->attachment->GetFormat());

            if (formatInfo.hasDepth)
            {
                depthAttachmentInfo = VkRenderingAttachmentInfo{
                    .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext              = nullptr,
                    .imageView          = depthStencilAttachment->viewHandle,
                    .imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .resolveMode        = ConvertResolveMode(passAttachment->resolveMode),
                    .resolveImageView   = resolveAttachment ? resolveAttachment->viewHandle : VK_NULL_HANDLE,
                    .resolveImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .loadOp             = ConvertLoadOp(passAttachment->loadOperation),
                    .storeOp            = ConvertStoreOp(passAttachment->storeOperation),
                    .clearValue         = {.depthStencil = clearValue},
                };
            }

            if (formatInfo.hasStencil)
            {
                stencilAttachmentInfo = VkRenderingAttachmentInfo{
                    .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext              = nullptr,
                    .imageView          = depthStencilAttachment->viewHandle,
                    .imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .resolveMode        = ConvertResolveMode(passAttachment->resolveMode),
                    .resolveImageView   = resolveAttachment->viewHandle ? resolveAttachment->viewHandle : VK_NULL_HANDLE,
                    .resolveImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .loadOp             = ConvertLoadOp(passAttachment->stencilLoadOperation),
                    .storeOp            = ConvertStoreOp(passAttachment->stencilStoreOperation),
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
                    .offset = {0, 0},
                    .extent = {ConvertExtent2D(pass.GetSize())},
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

    void ICommandList::EndPass()
    {
        ZoneScoped;

        if (m_isInsideRenderPass)
        {
            vkCmdEndRendering(m_commandBuffer);
        }
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
            m_isGraphicsPipelineBound = false;
            return;
        }

        auto pipeline = m_device->m_graphicsPipelineOwner.Get(pipelineState);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
        BindShaderBindGroups(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, bindGroups);
        m_isGraphicsPipelineBound = true;
    }

    void ICommandList::BindComputePipeline(Handle<ComputePipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScoped;

        if (pipelineState == NullHandle)
        {
            m_isComputePipelineBound = false;
            return;
        }

        auto pipeline = m_device->m_computePipelineOwner.Get(pipelineState);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle);
        BindShaderBindGroups(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->layout, bindGroups);
        m_isComputePipelineBound = true;
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
        m_hasViewportSet = true;
    }

    void ICommandList::SetSicssor(const Scissor& scissor)
    {
        ZoneScoped;

        VkRect2D vkScissor{
            .offset = {scissor.offsetX, scissor.offsetY},
            .extent = {scissor.width, scissor.height},
        };
        vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
        m_hasScissorSet = true;
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
        m_hasVertexBuffer = true;
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
        m_hasIndexBuffer = true;
    }

    void ICommandList::Draw(const DrawParameters& parameters)
    {
        ZoneScoped;

        if (m_hasIndexBuffer)
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
