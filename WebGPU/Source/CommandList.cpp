
#include "CommandList.hpp"

#include <RHI/Format.hpp>

#include <tracy/Tracy.hpp>

#include "Common.hpp"
#include "Device.hpp"
#include "Resources.hpp"

namespace RHI::WebGPU
{
    inline static WGPULoadOp ConvertLoadOp(LoadOperation op)
    {
        switch (op)
        {
        case LoadOperation::DontCare: return WGPULoadOp_Undefined;
        case LoadOperation::Load:     return WGPULoadOp_Load;
        case LoadOperation::Discard:  return WGPULoadOp_Clear;
        default:                      TL_UNREACHABLE(); return WGPULoadOp_Force32;
        }
    }

    inline static WGPUStoreOp ConvertStoreOp(StoreOperation op)
    {
        switch (op)
        {
        case StoreOperation::DontCare: return WGPUStoreOp_Undefined;
        case StoreOperation::Store:    return WGPUStoreOp_Store;
        case StoreOperation::Discard:  return WGPUStoreOp_Discard;
        default:                       TL_UNREACHABLE(); return WGPUStoreOp_Force32;
        }
    }

    /////

    ICommandList::ICommandList()  = default;
    ICommandList::~ICommandList() = default;

    ResultCode ICommandList::Init(IDevice* device, const CommandListCreateInfo& createInfo)
    {
        m_device = device;

        m_state = State::CommandEncoder;
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
        wgpuCommandBufferRelease(m_cmdBuffer);

        TL_ASSERT(m_state == State::CommandBuffer && m_state == State::RenderBundle);
    }

    void ICommandList::Begin()
    {
    }

    void ICommandList::End()
    {
        WGPUCommandBufferDescriptor descriptor{
            .nextInChain = nullptr,
            .label       = {},
        };
        m_cmdBuffer = wgpuCommandEncoderFinish(m_cmdEncoder, &descriptor);
        wgpuCommandEncoderRelease(m_cmdEncoder);
        m_state     = State::CommandBuffer;
    }

    void ICommandList::BeginRenderPass(const Pass& pass)
    {
        ZoneScoped;

        m_state = State::RenderPassEncoder;

        TL::Vector<WGPURenderPassColorAttachment>          colorAttachments{};
        TL::Optional<WGPURenderPassDepthStencilAttachment> depthStencilAttachment{};

        for (const auto& colorAttachmentRG : pass.GetColorAttachment())
        {
            auto colorImage  = m_device->m_imageOwner.Get(colorAttachmentRG.view->GetImage());
            auto resolveView = colorAttachmentRG.resolveView ? m_device->m_imageOwner.Get(colorAttachmentRG.resolveView->GetImage()) : nullptr;
            colorAttachments.push_back({
                .nextInChain   = nullptr,
                .view          = colorImage->view,
                .depthSlice    = WGPU_DEPTH_SLICE_UNDEFINED,
                .resolveTarget = resolveView ? resolveView->view : nullptr,
                .loadOp        = ConvertLoadOp(colorAttachmentRG.loadOp),
                .storeOp       = ConvertStoreOp(colorAttachmentRG.storeOp),
                .clearValue    = ConvertToColor(colorAttachmentRG.clearValue),
            });
        }

        if (auto depthStencilAttachmentRG = pass.GetDepthStencilAttachment())
        {
            auto depthStencilImage = m_device->m_imageOwner.Get(depthStencilAttachmentRG->view->GetImage());
            depthStencilAttachment = WGPURenderPassDepthStencilAttachment{
                .nextInChain       = nullptr,
                .view              = depthStencilImage->view,
                .depthLoadOp       = ConvertLoadOp(depthStencilAttachmentRG->depthLoadOp),
                .depthStoreOp      = ConvertStoreOp(depthStencilAttachmentRG->depthStoreOp),
                .depthClearValue   = depthStencilAttachmentRG->clearValue.depthValue,
                .depthReadOnly     = true,
                .stencilLoadOp     = ConvertLoadOp(depthStencilAttachmentRG->stencilLoadOp),
                .stencilStoreOp    = ConvertStoreOp(depthStencilAttachmentRG->stencilStoreOp),
                .stencilClearValue = depthStencilAttachmentRG->clearValue.stencilValue,
                .stencilReadOnly   = false,
            };
        }

        WGPURenderPassDescriptor descriptor{
            .nextInChain            = nullptr,
            .label                  = ConvertToStringView(pass.GetName()),
            .colorAttachmentCount   = colorAttachments.size(),
            .colorAttachments       = colorAttachments.data(),
            .depthStencilAttachment = depthStencilAttachment ? &depthStencilAttachment.value() : nullptr,
            .occlusionQuerySet      = nullptr,
            .timestampWrites        = nullptr,
        };
        m_renderPassEncoder = wgpuCommandEncoderBeginRenderPass(m_cmdEncoder, &descriptor);
    }

    void ICommandList::EndRenderPass()
    {
        ZoneScoped;
        wgpuRenderPassEncoderEnd(m_renderPassEncoder);
        m_state = State::CommandEncoder;
    }

    void ICommandList::DebugMarkerPush(const char* name, [[maybe_unused]] ColorValue<float> color)
    {
        switch (m_state)
        {
        case State::CommandBuffer:      TL_UNREACHABLE(); break;
        case State::CommandEncoder:     wgpuCommandEncoderPushDebugGroup(m_cmdEncoder, ConvertToStringView(name)); break;
        case State::RenderBundle:       wgpuRenderBundleEncoderPushDebugGroup(m_bundleEncoder, ConvertToStringView(name)); break;
        case State::RenderPassEncoder:  wgpuRenderPassEncoderPushDebugGroup(m_renderPassEncoder, ConvertToStringView(name)); break;
        case State::ComputePassEncoder: wgpuComputePassEncoderPushDebugGroup(m_computePassEncoder, ConvertToStringView(name)); break;
        }
    }

    void ICommandList::DebugMarkerPop()
    {
        switch (m_state)
        {
        case State::CommandBuffer:      TL_UNREACHABLE(); break;
        case State::CommandEncoder:     wgpuCommandEncoderPopDebugGroup(m_cmdEncoder); break;
        case State::RenderBundle:       wgpuRenderBundleEncoderPopDebugGroup(m_bundleEncoder); break;
        case State::RenderPassEncoder:  wgpuRenderPassEncoderPopDebugGroup(m_renderPassEncoder); break;
        case State::ComputePassEncoder: wgpuComputePassEncoderPopDebugGroup(m_computePassEncoder); break;
        }
    }

    void ICommandList::BeginConditionalCommands([[maybe_unused]] const BufferBindingInfo& conditionBuffer, [[maybe_unused]] bool inverted)
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
