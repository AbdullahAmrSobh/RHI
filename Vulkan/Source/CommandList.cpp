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

        if ((barriers.memoryBarriers.empty() && barriers.bufferBarriers.empty() && barriers.imageBarriers.empty()) == true)
            return;

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

    void ICommandList::BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScoped;

        if (bindGroups.empty()) return;

        constexpr size_t MaxDescriptorSets = 4;
        constexpr size_t MaxDynamicOffsets = MaxDescriptorSets;

        VkDescriptorSet descriptorSets[MaxDescriptorSets];
        uint32_t        dynamicOffsets[MaxDynamicOffsets];

        uint32_t descriptorSetCount = 0;
        uint32_t dynamicOffsetCount = 0;

        for (const auto& bindingInfo : bindGroups)
        {
            auto bindGroup = m_device->m_bindGroupOwner.Get(bindingInfo.bindGroup);

            // Ensure we don't exceed the array limits
            TL_ASSERT(descriptorSetCount < MaxDescriptorSets);
            descriptorSets[descriptorSetCount++] = bindGroup->descriptorSet;

            for (uint32_t offset : bindingInfo.dynamicOffsets)
            {
                TL_ASSERT(dynamicOffsetCount < MaxDynamicOffsets);
                dynamicOffsets[dynamicOffsetCount++] = offset;
            }
        }

        vkCmdBindDescriptorSets(
            m_commandBuffer,
            bindPoint,
            pipelineLayout,
            0,
            descriptorSetCount,
            descriptorSets,
            dynamicOffsetCount,
            dynamicOffsets);
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
        ZoneScoped;

        m_isInsideRenderPass = true;

        constexpr size_t          MaxColorAttachments = 8;
        VkRenderingAttachmentInfo colorAttachmentInfos[MaxColorAttachments];
        size_t                    colorAttachmentCount = 0;

        VkRenderingAttachmentInfo depthAttachmentInfo{};
        VkRenderingAttachmentInfo stencilAttachmentInfo{};
        bool                      hasDepthAttachment   = false;
        bool                      hasStencilAttachment = false;

        for (const auto& passAttachment : pass.GetColorAttachment())
        {
            TL_ASSERT(colorAttachmentCount < MaxColorAttachments);

            auto colorAttachmentHandle   = passAttachment.attachment->GetImage();
            auto resolveAttachmentHandle = passAttachment.resolveAttachment ? passAttachment.resolveAttachment->GetImage() : NullHandle;

            IImage* colorAttachment   = m_device->m_imageOwner.Get(colorAttachmentHandle);
            IImage* resolveAttachment = passAttachment.resolveAttachment ? m_device->m_imageOwner.Get(resolveAttachmentHandle) : nullptr;

            auto clearValue = ConvertColorValue(passAttachment.clearValue.f32);

            colorAttachmentInfos[colorAttachmentCount++] = {
                .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext              = nullptr,
                .imageView          = colorAttachment->viewHandle,
                .imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode        = ConvertResolveMode(passAttachment.resolveMode),
                .resolveImageView   = resolveAttachment ? resolveAttachment->viewHandle : VK_NULL_HANDLE,
                .resolveImageLayout = resolveAttachment ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp             = ConvertLoadOp(passAttachment.loadOperation),
                .storeOp            = ConvertStoreOp(passAttachment.storeOperation),
                .clearValue         = {clearValue},
            };
        }

        if (auto passAttachment = pass.GetDepthStencilAttachment())
        {
            auto depthStencilHandle      = passAttachment->attachment->GetImage();
            auto resolveAttachmentHandle = passAttachment->resolveAttachment ? passAttachment->resolveAttachment->GetImage() : NullHandle;

            IImage* depthStencilAttachment = m_device->m_imageOwner.Get(depthStencilHandle);
            IImage* resolveAttachment      = passAttachment->resolveAttachment ? m_device->m_imageOwner.Get(resolveAttachmentHandle) : nullptr;

            auto clearValue = ConvertClearValue(passAttachment->clearValue);

            auto formatInfo = GetFormatInfo(passAttachment->attachment->GetFormat());

            if (formatInfo.hasDepth)
            {
                depthAttachmentInfo = {
                    .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext              = nullptr,
                    .imageView          = depthStencilAttachment->viewHandle,
                    .imageLayout        = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                    .resolveMode        = ConvertResolveMode(passAttachment->resolveMode),
                    .resolveImageView   = resolveAttachment ? resolveAttachment->viewHandle : VK_NULL_HANDLE,
                    .resolveImageLayout = resolveAttachment ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED,
                    .loadOp             = ConvertLoadOp(passAttachment->loadOperation),
                    .storeOp            = ConvertStoreOp(passAttachment->storeOperation),
                    .clearValue         = {clearValue},
                };
                hasDepthAttachment = true;
            }

            if (formatInfo.hasStencil)
            {
                stencilAttachmentInfo = {
                    .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext              = nullptr,
                    .imageView          = depthStencilAttachment->viewHandle,
                    .imageLayout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    .resolveMode        = ConvertResolveMode(passAttachment->resolveMode),
                    .resolveImageView   = resolveAttachment ? resolveAttachment->viewHandle : VK_NULL_HANDLE,
                    .resolveImageLayout = resolveAttachment ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED,
                    .loadOp             = ConvertLoadOp(passAttachment->stencilLoadOperation),
                    .storeOp            = ConvertStoreOp(passAttachment->stencilStoreOperation),
                    .clearValue         = {clearValue},
                };
                hasStencilAttachment = true;
            }
        }

        VkRenderingInfo renderingInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext = nullptr,
            .flags = {},
            .renderArea =
                {
                    .offset = {0, 0},
                    .extent = ConvertExtent2D(pass.GetSize()),
                },
            .layerCount           = 1,
            .viewMask             = 0,
            .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentCount),
            .pColorAttachments    = colorAttachmentInfos,
            .pDepthAttachment     = hasDepthAttachment ? &depthAttachmentInfo : nullptr,
            .pStencilAttachment   = hasStencilAttachment ? &stencilAttachmentInfo : nullptr,
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
                .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
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

        constexpr size_t MaxCommandLists = 16;
        VkCommandBuffer  commandBuffers[MaxCommandLists];
        size_t           commandListCount = commandLists.size();
        TL_ASSERT(commandListCount <= MaxCommandLists, "Exceeded MaxCommandLists limit!");
        for (size_t i = 0; i < commandListCount; ++i)
        {
            auto commandList  = (const ICommandList*)commandLists[i];
            commandBuffers[i] = commandList->m_commandBuffer;
        }
        vkCmdExecuteCommands(m_commandBuffer, static_cast<uint32_t>(commandListCount), commandBuffers);
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
        // Flip the viewport so Vulkan NDC is consitant with other APIs
        VkViewport vkViewport{
            .x        = viewport.offsetX,
            .y        = viewport.offsetY + viewport.height,
            .width    = viewport.width,
            .height   = -viewport.height,
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

        constexpr size_t MaxVertexBuffers = 16;

        VkBuffer     buffers[MaxVertexBuffers];
        VkDeviceSize offsets[MaxVertexBuffers];

        size_t vertexBufferCount = vertexBuffers.size();
        TL_ASSERT(vertexBufferCount <= MaxVertexBuffers, "Vertex buffer count exceeds MaxVertexBuffers!");

        for (size_t i = 0; i < vertexBufferCount; ++i)
        {
            const auto& bindingInfo = vertexBuffers[i];
            auto        buffer      = m_device->m_bufferOwner.Get(bindingInfo.buffer);

            buffers[i] = buffer->handle;
            offsets[i] = bindingInfo.offset;
        }

        vkCmdBindVertexBuffers(
            m_commandBuffer,
            firstBinding,
            static_cast<uint32_t>(vertexBufferCount),
            buffers,
            offsets);
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
