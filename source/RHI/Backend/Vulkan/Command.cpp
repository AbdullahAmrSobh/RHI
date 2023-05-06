#pragma once

#include "RHI/Backend/Vulkan/Command.hpp"

#include "RHI/Attachments.hpp"
#include "RHI/Backend/Vulkan/Conversion.inl"
#include "RHI/Backend/Vulkan/Pipeline.hpp"
#include "RHI/Backend/Vulkan/Resources.hpp"
#include "RHI/Backend/Vulkan/ShaderResourceGroup.hpp"
#include "RHI/Backend/Vulkan/Vulkan.hpp"

namespace Vulkan
{

void CommandList::Begin()
{
}

void CommandList::End()
{
}

void CommandList::BeginRendering()
{
}

void CommandList::EndRendering()
{
}

void CommandList::CopyImage(const RHI::ImageSize& imageSize, const RHI::ImageCopyInfo& srcImageInfo, RHI::ImageCopyInfo& dstImageInfo)
{
    Image& srcImage = static_cast<Image&>(*srcImageInfo.image);
    Image& dstImage = static_cast<Image&>(*dstImageInfo.image);

    auto srcImageAttachment = m_barrierTracker->GetImagePassAttachment(srcImage);
    auto srcLayout          = m_barrierTracker->GetImageLayout(*srcImageAttachment);
    auto dstImageAttachment = m_barrierTracker->GetImagePassAttachment(dstImage);
    auto dstLayout          = m_barrierTracker->GetImageLayout(*dstImageAttachment);

    vk::CopyImageInfo2 copyInfo {};
    copyInfo.setSrcImage(srcImage.GetHandle());
    copyInfo.setSrcImageLayout(srcLayout);
    copyInfo.setDstImage(dstImage.GetHandle());
    copyInfo.setDstImageLayout(dstLayout);

    vk::ImageSubresourceLayers srcSubresource {};
    vk::ImageSubresourceLayers dstSubresource {};

    vk::ImageCopy2 region {};
    region.setExtent(ConvertExtent3D(imageSize));
    region.setSrcOffset(ConvertOffset3D(srcImageInfo.offset));
    region.setSrcSubresource(srcSubresource);
    region.setExtent(ConvertExtent3D(imageSize));
    region.setSrcOffset(ConvertOffset3D(srcImageInfo.offset));
    region.setSrcSubresource(srcSubresource);

    copyInfo.setRegionCount(1);
    copyInfo.setPRegions(&region);

    m_handle.copyImage2(copyInfo);
}

void CommandList::CopyBuffer(const RHI::BufferCopyInfo& srcBuffer, RHI::ImageCopyInfo& dstBuffer)
{
}

void CommandList::CopyBufferToImage(const RHI::BufferCopyInfo& srcBuffer, RHI::ImageCopyInfo& dstImage)
{
}

void CommandList::CopyImageToBuffer(const RHI::ImageCopyInfo& srcImage, RHI::BufferCopyInfo& dstBuffer)
{
}

void CommandList::BeginPredication(const RHI::Buffer& buffer, uint64_t offset, RHI::PredicationOp operation)
{
}

void CommandList::EndPredication()
{
}

void CommandList::SetPipelineState(const RHI::PipelineState& pso)
{
}

void CommandList::BindShaderResourceGroup(const RHI::ShaderResourceGroup& group)
{
}

void CommandList::SetRenderArea(const RHI::DrawArea& drawArea)
{
}

void CommandList::DrawIndexed(const RHI::DrawIndexedData& drawData)
{
    if (drawData.indexBuffer)
    {
        Buffer& buffer = static_cast<Buffer&>(*drawData.indexBuffer);
    }

    if (drawData.vertexBuffer)
    {
        Buffer& buffer = static_cast<Buffer&>(*drawData.vertexBuffer);
    }

    if (drawData.instanceBuffer)
    {
        Buffer& buffer = static_cast<Buffer&>(*drawData.instanceBuffer);
    }

    m_handle.drawIndexed(drawData.indexCount, drawData.instanceCount, drawData.firstIndexOffset, 0, 0);
}

void CommandList::DrawUnindexed(const RHI::DrawUnindexedData& drawData)
{
    if (drawData.vertexBuffer)
    {
        Buffer& buffer = static_cast<Buffer&>(*drawData.vertexBuffer);
    }

    if (drawData.instanceBuffer)
    {
        Buffer& buffer = static_cast<Buffer&>(*drawData.instanceBuffer);
    }

    m_handle.draw(drawData.verticesCount, drawData.instanceCount, drawData.firstVertexOffset, 0);
}

void CommandList::DispatchRay(const RHI::DispatchRaysData& raysData)
{
}

void CommandList::Dispatch(uint32_t offset[3], uint32_t groupCount[3])
{
    m_handle.dispatchBase(offset[0], offset[1], offset[2], groupCount[0], groupCount[1], groupCount[2]);
}

}  // namespace Vulkan