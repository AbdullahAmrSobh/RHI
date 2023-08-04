#pragma once
#include "RHI/Backend/Vulkan/Command.hpp"

#include "RHI/FrameGraph.hpp"
#include "RHI/FrameGraphAttachments.hpp"
#include "RHI/Pass.hpp"

#include "RHI/Backend/Vulkan/Conversion.inl"
#include "RHI/Backend/Vulkan/Pipeline.hpp"
#include "RHI/Backend/Vulkan/Resources.hpp"
#include "RHI/Backend/Vulkan/ShaderResourceGroup.hpp"
#include "RHI/Backend/Vulkan/Vulkan.hpp"

namespace Vulkan
{

inline static vk::AttachmentStoreOp ConvertStoreOp(RHI::ImageStoreOperation storeOp)
{
    switch (storeOp)
    {
        case RHI::ImageStoreOperation::DontCare: return vk::AttachmentStoreOp::eDontCare;
        case RHI::ImageStoreOperation::Store: return vk::AttachmentStoreOp::eStore;
        case RHI::ImageStoreOperation::Discard: return vk::AttachmentStoreOp::eNone;
        default: RHI_ASSERT_MSG(false, "invalid enum");
    }
    return {};
}

inline static vk::AttachmentLoadOp ConvertLoadOp(RHI::ImageLoadOperation loadOp)
{
    switch (loadOp)
    {
        case RHI::ImageLoadOperation::DontCare: return vk::AttachmentLoadOp::eDontCare;
        case RHI::ImageLoadOperation::Load: return vk::AttachmentLoadOp::eLoad;
        case RHI::ImageLoadOperation::Discard: return vk::AttachmentLoadOp::eClear;
        default: RHI_ASSERT_MSG(false, "invalid enum");
    }
    return {};
}

inline static vk::ImageSubresourceLayers ConvertImageSubresourceLayers(const RHI::ImageSubresourceView& subresource)
{
    vk::ImageSubresourceLayers layers {};
    // TODO Aspects
    layers.setMipLevel(subresource.mipLevel);
    layers.setBaseArrayLayer(subresource.arrayOffset);
    layers.setLayerCount(subresource.arrayCount);
    return layers;
}

void CommandList::BeginRendering(RHI::Pass& pass)
{
    std::vector<vk::RenderingAttachmentInfo> colorAttachments {};
    vk::RenderingInfo                        renderingInfo {};

    for (RHI::PassAttachment* attachment : pass.GetImageAttachments())
    {
        if (attachment->usage == RHI::AttachmentUsage::RenderTarget)
        {
            ImageView& view = static_cast<ImageView&>(*attachment->imageView);

            vk::RenderingAttachmentInfo colorAttachment {};
            colorAttachment.setImageView(view.GetHandle());
            colorAttachment.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
            colorAttachment.setLoadOp(ConvertLoadOp(attachment->imageInfo.loadStoreOperations.loadOperation));
            colorAttachment.setStoreOp(ConvertStoreOp(attachment->imageInfo.loadStoreOperations.storeOperation));
            colorAttachment.clearValue.color.float32[0] = attachment->imageInfo.clearValue.asFloat.r;
            colorAttachment.clearValue.color.float32[1] = attachment->imageInfo.clearValue.asFloat.g;
            colorAttachment.clearValue.color.float32[2] = attachment->imageInfo.clearValue.asFloat.b;
            colorAttachment.clearValue.color.float32[3] = attachment->imageInfo.clearValue.asFloat.a;
        }
    }

    RHI::PassAttachment* attachment = pass.GetDepthStencilAttachment();
    if (attachment != nullptr && attachment->usage == RHI::AttachmentUsage::DepthStencil)
    {
        ImageView& view = static_cast<ImageView&>(*attachment->imageView);

        vk::RenderingAttachmentInfo depthAttachment {};
        depthAttachment.setImageView(view.GetHandle());
        depthAttachment.setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal);
        depthAttachment.setLoadOp(ConvertLoadOp(attachment->imageInfo.loadStoreOperations.loadOperation));
        depthAttachment.setStoreOp(ConvertStoreOp(attachment->imageInfo.loadStoreOperations.storeOperation));
        depthAttachment.clearValue.depthStencil.setDepth(attachment->imageInfo.clearValue.asDepthStencil.depth);
        depthAttachment.clearValue.depthStencil.setStencil(attachment->imageInfo.clearValue.asDepthStencil.stencil);
        renderingInfo.setPDepthAttachment(&depthAttachment);
    }

    renderingInfo.setColorAttachments(colorAttachments);

    m_handle.beginRendering(renderingInfo);
}

void CommandList::EndRendering()
{
    m_handle.endRendering();
}

void CommandList::BindShaderResourceGroups(const PipelineState& pipelineState, std::span<RHI::ShaderResourceGroup*> srgs)
{
    std::vector<vk::DescriptorSet> descriptorSets;
    std::vector<vk::DeviceSize>    offsets;
    descriptorSets.reserve(srgs.size());
    for (auto _srg : srgs)
    {
        auto srg = static_cast<ShaderResourceGroup&>(*_srg);
        descriptorSets.push_back(srg.GetHandle());
    }

    m_handle.bindPipeline(pipelineState.GetBindPoint(), pipelineState.GetHandle());

    if (srgs.size())
    {
        // m_handle.bindDescriptorSets(pipelineState.GetBindPoint(), pipelineState.GetLayoutHandle(), 0u, descriptorSets, offsets);
    }
}

void CommandList::SetRenderArea(const RHI::DrawArea& drawArea)
{
    vk::Viewport viewport {};
    viewport.setWidth(static_cast<float>(drawArea.width));
    viewport.setHeight(static_cast<float>(drawArea.height));
    viewport.setMaxDepth(drawArea.maxDepth);
    viewport.setMinDepth(drawArea.minDepth);

    vk::Rect2D scissor {};
    scissor.setExtent({drawArea.width, drawArea.height});
    scissor.setOffset({drawArea.offsetX, drawArea.offsetY});

    m_handle.setViewport(0, {viewport});
    m_handle.setScissor(0, {scissor});
}

void CommandList::Submit(const RHI::Draw& cmd)
{
    std::vector<vk::Buffer>     vertexBufferHandles;
    std::vector<vk::DeviceSize> vertexBufferOffsets;

    vertexBufferHandles.reserve(cmd.vertexBuffersCount);
    vertexBufferOffsets.reserve(cmd.vertexBuffersCount);

    BindShaderResourceGroups(static_cast<const PipelineState&>(*cmd.pipelineState), cmd.shaderResourceGroups);

    for (uint32_t i = 0; i < cmd.vertexBuffersCount; i++)
    {
        auto& buffer = cmd.vertexBuffers[i];
        vertexBufferHandles.push_back(static_cast<const Buffer&>(*buffer).GetHandle());
        vertexBufferOffsets.push_back(0);
    }

    m_handle.bindVertexBuffers(0, vertexBufferHandles, vertexBufferOffsets.size());

    // Draw indexed
    if (cmd.indexBuffer)
    {
        auto& indexBuffer = static_cast<const Buffer&>(*cmd.indexBuffer);
        m_handle.bindIndexBuffer(indexBuffer.GetHandle(), 0, vk::IndexType::eUint32);

        m_handle.drawIndexed(cmd.indexedData.indexCount, cmd.instanceCount, cmd.indexedData.indexOffset, cmd.indexedData.vertexOffset, cmd.instanceOffset);
    }
    // Draw linear
    else
    {
        m_handle.draw(cmd.linearData.vertexCount, cmd.instanceCount, cmd.linearData.vertexOffset, cmd.instanceOffset);
    }
}

void CommandList::Submit(const RHI::Compute& cmd)
{
    RHI_ASSERT(cmd.pipelineState);
    auto pipeline = static_cast<PipelineState&>(*cmd.pipelineState);
    RHI_ASSERT(pipeline.GetBindPoint() == vk::PipelineBindPoint::eCompute);

    BindShaderResourceGroups(static_cast<const PipelineState&>(*cmd.pipelineState), cmd.shaderResourceGroups);

    m_handle.dispatchBase(cmd.offsetX, cmd.offsetY, cmd.offsetZ, cmd.countX, cmd.countY, cmd.countZ);
}

void CommandList::Submit(const RHI::Copy& cmd)
{
    switch (cmd.type)
    {
        case RHI::TransferCommandType::Buffer: {
            const auto& srcBuffer = static_cast<const Buffer&>(*cmd.buffer.srcBuffer);
            const auto& dstBuffer = static_cast<const Buffer&>(*cmd.buffer.dstBuffer);

            vk::BufferCopy2 copy {};
            copy.setSize(cmd.buffer.size);
            copy.setSrcOffset(cmd.buffer.srcOffset);
            copy.setDstOffset(cmd.buffer.dstOffset);

            vk::CopyBufferInfo2 copyInfo {};
            copyInfo.setRegionCount(1);
            copyInfo.setPRegions(&copy);
            copyInfo.setSrcBuffer(srcBuffer.GetHandle());
            copyInfo.setDstBuffer(dstBuffer.GetHandle());

            m_handle.copyBuffer2(copyInfo);
            break;
        }
        case RHI::TransferCommandType::Image: {
            const auto& srcImage = static_cast<const Image&>(*cmd.image.srcImage);
            const auto& dstImage = static_cast<const Image&>(*cmd.image.dstImage);

            vk::ImageCopy2 copy {};
            copy.setExtent({cmd.image.srcSize.width, cmd.image.srcSize.height, cmd.image.srcSize.depth});

            copy.setSrcOffset({cmd.image.srcOrigin.x, cmd.image.srcOrigin.y, cmd.image.srcOrigin.z});
            copy.setSrcSubresource(ConvertImageSubresourceLayers(cmd.image.srcSubresource));

            copy.setDstOffset({cmd.image.dstOrigin.x, cmd.image.dstOrigin.y, cmd.image.dstOrigin.z});
            copy.setDstSubresource(ConvertImageSubresourceLayers(cmd.image.dstSubresource));

            vk::CopyImageInfo2 copyInfo {};
            copyInfo.setRegionCount(1);
            copyInfo.setPRegions(&copy);
            copyInfo.setSrcImage(srcImage.GetHandle());
            copyInfo.setSrcImageLayout(vk::ImageLayout::eTransferSrcOptimal);
            copyInfo.setDstImage(dstImage.GetHandle());
            copyInfo.setDstImageLayout(vk::ImageLayout::eTransferDstOptimal);

            m_handle.copyImage2(copyInfo);
            break;
        }
        case RHI::TransferCommandType::BufferToImage: {
            const auto& srcBuffer = static_cast<const Buffer&>(*cmd.bufferToImage.srcBuffer);
            const auto& dstImage  = static_cast<const Image&>(*cmd.bufferToImage.dstImage);

            auto     alignment   = GetFormatDimensionAlignment(dstImage.GetInfo().format) - 1;
            uint32_t imageHeight = (cmd.bufferToImage.srcSize.height + alignment) & ~alignment;

            vk::BufferImageCopy2 copy {};
            copy.setBufferOffset(cmd.bufferToImage.srcOffset);
            copy.setBufferRowLength(cmd.bufferToImage.srcBytesPerRow);
            copy.setBufferImageHeight(imageHeight);
            copy.setImageSubresource(ConvertImageSubresourceLayers(cmd.bufferToImage.dstSubresource));
            copy.setImageOffset({cmd.bufferToImage.dstOrigin.x, cmd.bufferToImage.dstOrigin.y, cmd.bufferToImage.dstOrigin.z});
            copy.setImageExtent({cmd.bufferToImage.srcSize.width, cmd.bufferToImage.srcSize.height, cmd.bufferToImage.srcSize.depth});

            vk::CopyBufferToImageInfo2 copyInfo {};
            copyInfo.setRegionCount(1);
            copyInfo.setPRegions(&copy);
            copyInfo.setSrcBuffer(srcBuffer.GetHandle());
            copyInfo.setDstImage(dstImage.GetHandle());
            copyInfo.setDstImageLayout(vk::ImageLayout::eTransferDstOptimal);

            m_handle.copyBufferToImage2(copyInfo);
            break;
        }
        case RHI::TransferCommandType::ImageToBuffer: {
            const auto& srcImage  = static_cast<const Image&>(*cmd.imageToBuffer.srcImage);
            const auto& dstBuffer = static_cast<const Buffer&>(*cmd.imageToBuffer.dstBuffer);

            auto     alignment   = GetFormatDimensionAlignment(srcImage.GetInfo().format) - 1;
            uint32_t imageHeight = (cmd.imageToBuffer.srcSize.height + alignment) & ~alignment;

            vk::BufferImageCopy2 copy {};
            copy.setBufferOffset(cmd.imageToBuffer.dstOffset);
            copy.setBufferRowLength(cmd.imageToBuffer.dstBytesPerRow);
            copy.setBufferImageHeight(imageHeight);
            copy.setImageSubresource(ConvertImageSubresourceLayers(cmd.imageToBuffer.srcSubresource));
            copy.setImageOffset({cmd.imageToBuffer.srcOrigin.x, cmd.imageToBuffer.srcOrigin.y, cmd.imageToBuffer.srcOrigin.z});
            copy.setImageExtent({cmd.imageToBuffer.srcSize.width, cmd.imageToBuffer.srcSize.height, cmd.imageToBuffer.srcSize.depth});

            vk::CopyImageToBufferInfo2 copyInfo {};
            copyInfo.setRegionCount(1);
            copyInfo.setPRegions(&copy);
            copyInfo.setSrcImage(srcImage.GetHandle());
            copyInfo.setSrcImageLayout(vk::ImageLayout::eTransferSrcOptimal);
            copyInfo.setDstBuffer(dstBuffer.GetHandle());

            m_handle.copyImageToBuffer2(copyInfo);
            break;
        }
        default: {
            RHI_UNREACHABLE();
            break;
        }
    }
}

void CommandList::Begin()
{
    auto beginInfo = vk::CommandBufferBeginInfo {};
    auto result    = m_handle.begin(beginInfo);

    RHI_ASSERT(result == vk::Result::eSuccess);
}

void CommandList::End()
{
    auto result = m_handle.end();

    RHI_ASSERT(result == vk::Result::eSuccess);
}

}  // namespace Vulkan