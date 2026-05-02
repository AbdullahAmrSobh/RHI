
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
        // Nothing to do here!
    }

    void ICommandList::End()
    {
        WGPUCommandBufferDescriptor descriptor{
            .nextInChain = nullptr,
            .label       = {},
        };
        m_cmdBuffer = wgpuCommandEncoderFinish(m_cmdEncoder, &descriptor);
        wgpuCommandEncoderRelease(m_cmdEncoder);
        m_state = State::CommandBuffer;
    }

    void ICommandList::AddPipelineBarrier(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers)
    {
    }

    void ICommandList::BeginRenderPass(const RenderPassBeginInfo& beginInfo)
    {
        ZoneScoped;

        if (pass.GetQueueType() == QueueType::Compute)
        {
            const WGPUComputePassDescriptor descriptor{
                .label = WGPUStringView(pass.GetName()),
            };
            m_computePassEncoder = wgpuCommandEncoderBeginComputePass(m_cmdEncoder, &descriptor);
            m_state              = State::ComputePassEncoder;
            return;
        }

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
            auto formatInfo        = GetFormatInfo(depthStencilImage->format);

            auto depthReadOnly =
                (depthStencilAttachmentRG->depthLoadOp == LoadOperation::Load) && (depthStencilAttachmentRG->depthStoreOp != StoreOperation::Store);
            auto stencilReadOnly =
                (depthStencilAttachmentRG->stencilLoadOp == LoadOperation::Load) && (depthStencilAttachmentRG->stencilStoreOp != StoreOperation::Store);

            TL_ASSERT(formatInfo.hasDepth || formatInfo.hasStencil);

            depthStencilAttachment = WGPURenderPassDepthStencilAttachment{
                .nextInChain       = nullptr,
                .view              = depthStencilImage->view,
                .depthLoadOp       = formatInfo.hasDepth ? ConvertLoadOp(depthStencilAttachmentRG->depthLoadOp) : WGPULoadOp_Undefined,
                .depthStoreOp      = formatInfo.hasDepth ? ConvertStoreOp(depthStencilAttachmentRG->depthStoreOp) : WGPUStoreOp_Undefined,
                .depthClearValue   = formatInfo.hasDepth ? depthStencilAttachmentRG->clearValue.depthValue : 0.0f,
                .depthReadOnly     = formatInfo.hasDepth ? depthReadOnly : false,
                .stencilLoadOp     = formatInfo.hasStencil ? ConvertLoadOp(depthStencilAttachmentRG->stencilLoadOp) : WGPULoadOp_Undefined,
                .stencilStoreOp    = formatInfo.hasStencil ? ConvertStoreOp(depthStencilAttachmentRG->stencilStoreOp) : WGPUStoreOp_Undefined,
                .stencilClearValue = formatInfo.hasStencil ? depthStencilAttachmentRG->clearValue.stencilValue : uint8_t(0),
                .stencilReadOnly   = formatInfo.hasStencil ? stencilReadOnly : false,
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
        if (m_state == State::ComputePassEncoder)
        {
            wgpuComputePassEncoderEnd(m_computePassEncoder);
            return;
        }
        wgpuRenderPassEncoderEnd(m_renderPassEncoder);
        m_state = State::CommandEncoder;
    }

    void ICommandList::BeginComputePass(const ComputePassBeginInfo& beginInfo)
    {
    }

    void ICommandList::EndComputePass()
    {
    }

    void ICommandList::PushDebugMarker(const char* name, [[maybe_unused]] uint32_t bgra)
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

    void ICommandList::PopDebugMarker()
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

    void ICommandList::InsertDebugMarker(const char* name, [[maybe_unused]] uint32_t bgra)
    {
        switch (m_state)
        {
        case State::CommandBuffer:      TL_UNREACHABLE(); break;
        case State::CommandEncoder:     wgpuCommandEncoderInsertDebugMarker(m_cmdEncoder, ConvertToStringView(name)); break;
        case State::RenderBundle:       wgpuRenderBundleEncoderInsertDebugMarker(m_bundleEncoder, ConvertToStringView(name)); break;
        case State::RenderPassEncoder:  wgpuRenderPassEncoderInsertDebugMarker(m_renderPassEncoder, ConvertToStringView(name)); break;
        case State::ComputePassEncoder: wgpuComputePassEncoderInsertDebugMarker(m_computePassEncoder, ConvertToStringView(name)); break;
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

    void ICommandList::BindGraphicsPipeline(const GraphicsPipeline* pipelineState)
    {
        auto pipeline = m_device->m_graphicsPipelineOwner.Get(pipelineState);
        wgpuRenderPassEncoderSetPipeline(m_renderPassEncoder, pipeline->pipeline);
    }

    void ICommandList::BindComputePipeline(const ComputePipeline* pipelineState)
    {
        auto pipeline = m_device->m_computePipelineOwner.Get(pipelineState);
        wgpuComputePassEncoderSetPipeline(m_computePassEncoder, pipeline->pipeline);
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

        if (countBuffer.buffer)
        {
            auto countBufferResource = m_device->m_bufferOwner.Get(countBuffer.buffer);
            wgpuRenderPassEncoderMultiDrawIndexedIndirect(
                m_renderPassEncoder,
                argumentBufferResource->buffer,
                argumentBuffer.offset,
                maxDrawCount,
                countBufferResource->buffer,
                countBuffer.offset);
        }
        else
        {
            for (uint32_t drawOffset = argumentBuffer.offset; drawOffset < (stride * maxDrawCount); drawOffset += stride)
                wgpuRenderPassEncoderDrawIndirect(m_renderPassEncoder, argumentBufferResource->buffer, drawOffset);
        }
    }

    void ICommandList::Dispatch(const DispatchParameters& parameters)
    {
        wgpuComputePassEncoderDispatchWorkgroups(
            m_computePassEncoder, parameters.x, parameters.y, parameters.z);
    }

    void ICommandList::DispatchIndirect(const BufferBindingInfo& argumentBuffer)
    {
        auto argumentBufferResource = m_device->m_bufferOwner.Get(argumentBuffer.buffer);
        wgpuComputePassEncoderDispatchWorkgroupsIndirect(
            m_computePassEncoder,
            argumentBufferResource->buffer,
            argumentBuffer.offset);
    }

    void ICommandList::BindPipelineLayout([[maybe_unused]] BindPoint bindPoint, [[maybe_unused]] const PipelineLayout* pipelineLayout) { TL_UNREACHABLE(); }
    void ICommandList::SetPushConstants([[maybe_unused]] BindPoint bindPoint, [[maybe_unused]] uint32_t offset, [[maybe_unused]] TL::Block content) { TL_UNREACHABLE(); }
    void ICommandList::PushBindGroup([[maybe_unused]] BindPoint bindPoint, [[maybe_unused]] uint32_t firstGroup, [[maybe_unused]] TL::Span<const BindGroupUpdateInfo> updateInfos) { TL_UNREACHABLE(); }
    void ICommandList::SetBindGroups([[maybe_unused]] BindPoint bindPoint, [[maybe_unused]] TL::Span<const BindGroupBindingInfo> bindGroups) { TL_UNREACHABLE(); }

    void ICommandList::DrawMeshTasks([[maybe_unused]] const DispatchParameters drawMeshTasksDesc) { TL_UNREACHABLE(); }
    void ICommandList::DrawMeshTasksIndirect([[maybe_unused]] const BufferBindingInfo& argumentBuffer, [[maybe_unused]] const BufferBindingInfo& countBuffer, [[maybe_unused]] uint32_t drawNum, [[maybe_unused]] uint32_t stride) { TL_UNREACHABLE(); }
    void ICommandList::DispatchRays([[maybe_unused]] const DispatchRaysInfo& dispatchRaysDesc) { TL_UNREACHABLE(); }
    void ICommandList::DispatchRaysIndirect([[maybe_unused]] const BufferBindingInfo& argumentBuffer) { TL_UNREACHABLE(); }

    void ICommandList::CopyBuffer([[maybe_unused]] const Buffer* srcBuffer, [[maybe_unused]] uint64_t srcOffset, [[maybe_unused]] const Buffer* dstBuffer, [[maybe_unused]] uint64_t dstOffset, [[maybe_unused]] uint64_t size) { TL_UNREACHABLE(); }
    void ICommandList::CopyImage([[maybe_unused]] const ImageCopyInfo& srcImage, [[maybe_unused]] const ImageCopyInfo& dstImage, [[maybe_unused]] const ImageSize3D& size) { TL_UNREACHABLE(); }
    void ICommandList::CopyImageToBuffer([[maybe_unused]] const ImageCopyInfo& srcImage, [[maybe_unused]] const ImageMemoryLayout& layout, [[maybe_unused]] const Buffer* dstBuffer) { TL_UNREACHABLE(); }
    void ICommandList::CopyBufferToImage([[maybe_unused]] const Buffer* srcBuffer, [[maybe_unused]] const ImageCopyInfo& dstImage, [[maybe_unused]] const ImageMemoryLayout& layout) { TL_UNREACHABLE(); }
    void ICommandList::CopyMicromap([[maybe_unused]] Micromap* dst, [[maybe_unused]] const Micromap* src, [[maybe_unused]] CopyMode copyMode) { TL_UNREACHABLE(); }
    void ICommandList::CopyAccelerationStructure([[maybe_unused]] AccelerationStructure* dst, [[maybe_unused]] const AccelerationStructure* src, [[maybe_unused]] CopyMode copyMode) { TL_UNREACHABLE(); }

    void ICommandList::BuildMicromaps([[maybe_unused]] TL::Span<const MicromapBuildInfo> buildInfos) { TL_UNREACHABLE(); }
    void ICommandList::WriteMicromapsSizes([[maybe_unused]] TL::Span<const Micromap*> micromaps, [[maybe_unused]] QueryPool* queryPool, [[maybe_unused]] uint32_t queryPoolOffset) { TL_UNREACHABLE(); }
    void ICommandList::BuildTopLevelAccelerationStructures([[maybe_unused]] TL::Span<const TopLevelAccelerationStructureBuildInfo> buildInfos) { TL_UNREACHABLE(); }
    void ICommandList::BuildBottomLevelAccelerationStructures([[maybe_unused]] TL::Span<const BottomLevelAccelerationStructureBuildInfo> buildInfos) { TL_UNREACHABLE(); }
    void ICommandList::WriteAccelerationStructuresSizes([[maybe_unused]] TL::Span<const AccelerationStructure*> accelerationStructures, [[maybe_unused]] QueryPool* queryPool, [[maybe_unused]] uint32_t queryPoolOffset) { TL_UNREACHABLE(); }

} // namespace RHI::WebGPU
