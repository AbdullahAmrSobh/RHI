
#include "CommandList.hpp"

#include <RHI/Format.hpp>

#include <tracy/Tracy.hpp>

#include "Common.hpp"
#include "Device.hpp"
#include "Resources.hpp"

namespace RHI::WebGPU
{
    ICommandList::ICommandList()  = default;
    ICommandList::~ICommandList() = default;

    ResultCode ICommandList::Init(IDevice* device, const CommandListCreateInfo& createInfo)
    {
        WGPUCommandEncoderDescriptor desc{
            .nextInChain = {},
            .label       = ConvertToStringView(createInfo.name),
        };
        m_cmdEncoder = wgpuDeviceCreateCommandEncoder(m_device->m_device, &desc);
        return ResultCode::Success;
    }

    void ICommandList::Shutdown()
    {
        wgpuCommandEncoderRelease(m_cmdEncoder);
    }

    void ICommandList::Begin()
    {
        if (m_isRenderPass)
        {
            TL::Vector<WGPURenderPassColorAttachment>          colorAttachments;
            TL::Optional<WGPURenderPassDepthStencilAttachment> depthStencilAttachment;

            WGPURenderPassDescriptor passDesc{
                .nextInChain            = nullptr,
                .label                  = {},
                .colorAttachmentCount   = colorAttachments.size(),
                .colorAttachments       = colorAttachments.data(),
                .depthStencilAttachment = depthStencilAttachment ? &*depthStencilAttachment : nullptr,
                .occlusionQuerySet      = {},
                .timestampWrites        = {},
            };
            m_renderPassEncoder = wgpuCommandEncoderBeginRenderPass(m_cmdEncoder, &passDesc);
        }
        else
        {
            WGPUComputePassDescriptor passDesc{
                .nextInChain     = nullptr,
                .label           = {},
                .timestampWrites = {},
            };
            m_computePassEncoder = wgpuCommandEncoderBeginComputePass(m_cmdEncoder, &passDesc);
        }
    }

    void ICommandList::End()
    {
        if (m_isRenderPass)
        {
            wgpuRenderPassEncoderEnd(m_renderPassEncoder);
        }
        else
        {
            wgpuComputePassEncoderEnd(m_computePassEncoder);
        }

        WGPUCommandBufferDescriptor descriptor{
            .nextInChain = nullptr,
            .label       = {},
        };
        m_cmdBuffer = wgpuCommandEncoderFinish(m_cmdEncoder, &descriptor);
    }

    void ICommandList::DebugMarkerPush(const char* name, ColorValue<float> color)
    {
        // TODO: in case we are render/compuet pass we should use different function??
        wgpuCommandEncoderPushDebugGroup(m_cmdEncoder, ConvertToStringView(name));
    }

    void ICommandList::DebugMarkerPop()
    {
        // TODO: in case we are render/compuet pass we should use different function??
        wgpuCommandEncoderPopDebugGroup(m_cmdEncoder);
    }

    void ICommandList::BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted)
    {
        TL_UNREACHABLE();
    }

    void ICommandList::EndConditionalCommands()
    {
        TL_UNREACHABLE();
    }

    void ICommandList::Execute(TL::Span<const CommandList*> commandLists)
    {
        TL::Vector<WGPURenderBundle> bundles;
        for (auto _commandList : commandLists)
        {
            auto commandList = (ICommandList*)_commandList;
            bundles.push_back(commandList->m_bundle);
        }
        wgpuRenderPassEncoderExecuteBundles(m_renderPassEncoder, bundles.size(), bundles.data());
    }

    void ICommandList::BindGraphicsPipeline(Handle<GraphicsPipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        auto pipeline = m_device->m_graphicsPipelineOwner.Get(pipelineState);
        wgpuRenderPassEncoderSetPipeline(m_renderPassEncoder, pipeline->pipeline);

        uint32_t index = 0;
        for (const auto& bindingInfo : bindGroups)
        {
            auto bindGroup = m_device->m_bindGroupOwner.Get(bindingInfo.bindGroup);
            wgpuRenderPassEncoderSetBindGroup(m_renderPassEncoder, index++, bindGroup->bindGroup, bindingInfo.dynamicOffsets.size(), bindingInfo.dynamicOffsets.data());
        }
    }

    void ICommandList::BindComputePipeline(Handle<ComputePipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        auto pipeline = m_device->m_computePipelineOwner.Get(pipelineState);
        wgpuComputePassEncoderSetPipeline(m_computePassEncoder, pipeline->pipeline);

        uint32_t index = 0;
        for (const auto& bindingInfo : bindGroups)
        {
            auto bindGroup = m_device->m_bindGroupOwner.Get(bindingInfo.bindGroup);
            wgpuComputePassEncoderSetBindGroup(m_computePassEncoder, index++, bindGroup->bindGroup, bindingInfo.dynamicOffsets.size(), bindingInfo.dynamicOffsets.data());
        }
    }

    void ICommandList::SetViewport(const Viewport& viewport)
    {
        wgpuRenderPassEncoderSetViewport(m_renderPassEncoder, viewport.offsetX, viewport.offsetY, viewport.width, viewport.height, viewport.minDepth, viewport.maxDepth);
    }

    void ICommandList::SetScissor(const Scissor& sicssor)
    {
        wgpuRenderPassEncoderSetScissorRect(m_renderPassEncoder, sicssor.offsetX, sicssor.offsetY, sicssor.width, sicssor.height);
    }

    void ICommandList::BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers)
    {
        for (uint32_t i = firstBinding; i < vertexBuffers.size(); ++i)
        {
            auto buffer = m_device->m_bufferOwner.Get(vertexBuffers[i].buffer);
            wgpuRenderPassEncoderSetVertexBuffer(m_renderPassEncoder, i, buffer->buffer, 0, WGPU_WHOLE_SIZE);
        }
    }

    void ICommandList::BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType)
    {
        auto buffer = m_device->m_bufferOwner.Get(indexBuffer.buffer);
        wgpuRenderPassEncoderSetIndexBuffer(m_renderPassEncoder, buffer->buffer, indexType == IndexType::uint16 ? WGPUIndexFormat_Uint16 : WGPUIndexFormat_Uint32, 0, WGPU_WHOLE_SIZE);
    }

    void ICommandList::Draw(const DrawParameters& parameters)
    {
        wgpuRenderPassEncoderDraw(m_renderPassEncoder, parameters.vertexCount, parameters.instanceCount, parameters.firstVertex, parameters.firstInstance);
    }

    void ICommandList::DrawIndexed(const DrawIndexedParameters& parameters)
    {
        wgpuRenderPassEncoderDrawIndexed(m_renderPassEncoder, parameters.indexCount, parameters.instanceCount, parameters.firstIndex, parameters.vertexOffset, parameters.firstInstance);
    }

    void ICommandList::DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        auto argumentBufferResource = m_device->m_bufferOwner.Get(argumentBuffer.buffer);
        auto countBufferResource    = m_device->m_bufferOwner.Get(countBuffer.buffer);

        wgpuRenderPassEncoderMultiDrawIndirect(
            m_renderPassEncoder,
            argumentBufferResource->buffer,
            argumentBuffer.offset,
            maxDrawCount,
            countBufferResource->buffer,
            countBuffer.offset);
    }

    void ICommandList::DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        auto argumentBufferResource = m_device->m_bufferOwner.Get(argumentBuffer.buffer);
        auto countBufferResource    = m_device->m_bufferOwner.Get(countBuffer.buffer);

        wgpuRenderPassEncoderMultiDrawIndexedIndirect(
            m_renderPassEncoder,
            argumentBufferResource->buffer,
            argumentBuffer.offset,
            maxDrawCount,
            countBufferResource->buffer,
            countBuffer.offset);
    }

    void ICommandList::Dispatch(const DispatchParameters& parameters)
    {
        wgpuComputePassEncoderDispatchWorkgroups(
            m_computePassEncoder, parameters.countX, parameters.countY, parameters.countZ);
    }

    void ICommandList::DispatchIndirect(const BufferBindingInfo& argumentBuffer)
    {
        auto argumentBufferResource = m_device->m_bufferOwner.Get(argumentBuffer.buffer);
        wgpuComputePassEncoderDispatchWorkgroupsIndirect(
            m_computePassEncoder,
            argumentBufferResource->buffer,
            argumentBuffer.offset);
    }

    void ICommandList::CopyBuffer(const BufferCopyInfo& copyInfo)
    {
        auto srcBuffer = m_device->m_bufferOwner.Get(copyInfo.srcBuffer);
        auto dstBuffer = m_device->m_bufferOwner.Get(copyInfo.dstBuffer);

        wgpuCommandEncoderCopyBufferToBuffer(
            m_cmdEncoder,
            srcBuffer->buffer,
            copyInfo.srcOffset,
            dstBuffer->buffer,
            copyInfo.dstOffset,
            copyInfo.size);
    }

    void ICommandList::CopyImage(const ImageCopyInfo& copyInfo)
    {
        auto srcImage = m_device->m_imageOwner.Get(copyInfo.srcImage);
        auto dstImage = m_device->m_imageOwner.Get(copyInfo.dstImage);

        WGPUTexelCopyTextureInfo source{
            .texture  = srcImage->texture,
            .mipLevel = copyInfo.srcSubresource.mipLevel,
            .origin   = ConvertToOffset3D(copyInfo.srcOffset),
            .aspect   = WGPUTextureAspect_All,
        };
        WGPUTexelCopyTextureInfo destination{
            .texture  = dstImage->texture,
            .mipLevel = copyInfo.dstSubresource.mipLevel,
            .origin   = ConvertToOffset3D(copyInfo.dstOffset),
            .aspect   = WGPUTextureAspect_All,
        };
        WGPUExtent3D copySize = ConvertToExtent3D(copyInfo.srcSize);
        wgpuCommandEncoderCopyTextureToTexture(m_cmdEncoder, &source, &destination, &copySize);
    }

    void ICommandList::CopyImageToBuffer(const BufferImageCopyInfo& copyInfo)
    {
        auto srcImage  = m_device->m_imageOwner.Get(copyInfo.image);
        auto dstBuffer = m_device->m_bufferOwner.Get(copyInfo.buffer);

        WGPUTexelCopyTextureInfo source{
            .texture  = srcImage->texture,
            .mipLevel = copyInfo.subresource.mipLevel,
            .origin   = ConvertToOffset3D(copyInfo.imageOffset),
            .aspect   = WGPUTextureAspect_All,
        };
        WGPUTexelCopyBufferInfo destination{
            .layout = {
                       .offset       = copyInfo.bufferOffset,
                       .bytesPerRow  = copyInfo.bytesPerRow,
                       .rowsPerImage = copyInfo.bytesPerImage,
                       },
            .buffer = dstBuffer->buffer,
        };
        WGPUExtent3D copySize = ConvertToExtent3D(copyInfo.imageSize);
        wgpuCommandEncoderCopyTextureToBuffer(m_cmdEncoder, &source, &destination, &copySize);
    }

    void ICommandList::CopyBufferToImage(const BufferImageCopyInfo& copyInfo)
    {
        auto srcBuffer = m_device->m_bufferOwner.Get(copyInfo.buffer);
        auto dstImage  = m_device->m_imageOwner.Get(copyInfo.image);

        WGPUTexelCopyTextureInfo source{
            .texture  = dstImage->texture,
            .mipLevel = copyInfo.subresource.mipLevel,
            .origin   = ConvertToOffset3D(copyInfo.imageOffset),
            .aspect   = WGPUTextureAspect_All,
        };
        WGPUTexelCopyBufferInfo destination{
            .layout = {
                       .offset       = copyInfo.bufferOffset,
                       .bytesPerRow  = copyInfo.bytesPerRow,
                       .rowsPerImage = copyInfo.bytesPerImage,
                       },
            .buffer = srcBuffer->buffer,
        };
        WGPUExtent3D copySize = ConvertToExtent3D(copyInfo.imageSize);
        wgpuCommandEncoderCopyBufferToTexture(m_cmdEncoder, &destination, &source, &copySize);
    }
} // namespace RHI::WebGPU
