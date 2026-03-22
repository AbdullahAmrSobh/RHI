#include "CommandList.hpp"

#include <tracy/Tracy.hpp>

#include "Device.hpp"
#include "Resources.hpp"

namespace RHI::D3D12
{
    ///////////////////////////////////////////////////////////
    // ICommandPool
    ///////////////////////////////////////////////////////////

    ResultCode ICommandPool::Init(IDevice* device, const CommandPoolCreateInfo& createInfo)
    {
        ZoneScoped;
        m_device = device;

        switch (createInfo.queue)
        {
        case QueueType::Graphics: m_type = D3D12_COMMAND_LIST_TYPE_DIRECT;  break;
        case QueueType::Compute:  m_type = D3D12_COMMAND_LIST_TYPE_COMPUTE; break;
        case QueueType::Transfer: m_type = D3D12_COMMAND_LIST_TYPE_COPY;    break;
        default:                  m_type = D3D12_COMMAND_LIST_TYPE_DIRECT;  break;
        }

        HRESULT hr = device->m_device->CreateCommandAllocator(m_type, IID_PPV_ARGS(&m_allocator));
        if (FAILED(hr)) return ConvertFromHRESULT(hr);

        return ResultCode::Success;
    }

    void ICommandPool::Shutdown(IDevice* /*device*/)
    {
        for (auto* cmdList : m_commandLists)
        {
            cmdList->Shutdown();
            TL::destruct(cmdList);
        }
        m_commandLists.clear();

        if (m_allocator) { m_allocator->Release(); m_allocator = nullptr; }
    }

    void ICommandPool::Reset()
    {
        ZoneScoped;
        if (m_allocator)
            m_allocator->Reset();
    }

    CommandList* ICommandPool::Allocate()
    {
        ZoneScoped;
        auto* cmdList = TL::construct<ICommandList>();
        CommandListCreateInfo createInfo{};
        if (IsSuccess(cmdList->Init(m_device, this, createInfo)))
        {
            m_commandLists.push_back(cmdList);
            return cmdList;
        }
        TL::destruct(cmdList);
        return nullptr;
    }

    ///////////////////////////////////////////////////////////
    // ICommandList
    ///////////////////////////////////////////////////////////

    ICommandList::ICommandList()  = default;
    ICommandList::~ICommandList() = default;

    ResultCode ICommandList::Init(IDevice* device, ICommandPool* pool, const CommandListCreateInfo& /*createInfo*/)
    {
        ZoneScoped;
        m_device = device;
        m_pool   = pool;

        HRESULT hr = device->m_device->CreateCommandList(
            0,
            pool->m_type,
            pool->m_allocator,
            nullptr,
            IID_PPV_ARGS(&m_cmdLst));

        if (FAILED(hr)) return ConvertFromHRESULT(hr);

        // Command lists start in recording state after creation, close immediately
        m_cmdLst->Close();
        return ResultCode::Success;
    }

    void ICommandList::Shutdown()
    {
        if (m_cmdLst) { m_cmdLst->Release(); m_cmdLst = nullptr; }
    }

    void ICommandList::Begin()
    {
        ZoneScoped;
        m_cmdLst->Reset(m_pool->m_allocator, nullptr);
    }

    void ICommandList::End()
    {
        ZoneScoped;
        m_cmdLst->Close();
    }

    void ICommandList::AddPipelineBarrier(
        TL::Span<const BarrierInfo>       barriers,
        TL::Span<const ImageBarrierInfo>  imageBarriers,
        TL::Span<const BufferBarrierInfo> bufferBarriers)
    {
        ZoneScoped;

        // TODO: convert RHI barriers to D3D12_BARRIER_GROUP and call Barrier()
        (void)barriers;
        (void)imageBarriers;
        (void)bufferBarriers;
    }

    void ICommandList::BeginRenderPass(const RenderPassBeginInfo& beginInfo)
    {
        ZoneScoped;

        // Build render target descriptors
        UINT numRenderTargets = (UINT)beginInfo.colorAttachments.size();
        TL::Vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> rtDescs;
        rtDescs.reserve(numRenderTargets);

        for (auto& attachment : beginInfo.colorAttachments)
        {
            auto* img = (IImage*)attachment.view;
            D3D12_RENDER_PASS_RENDER_TARGET_DESC desc{};
            desc.cpuDescriptor                             = img->rtvDsvHandle;
            desc.BeginningAccess.Type                      = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
            desc.BeginningAccess.Clear.ClearValue.Format   = ConvertToFormat(img->format);
            desc.EndingAccess.Type                         = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
            rtDescs.push_back(desc);
        }

        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* dsDescPtr = nullptr;
        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC  dsDesc{};
        if (beginInfo.depthStencilAttachment.view)
        {
            auto* img                        = (IImage*)beginInfo.depthStencilAttachment.view;
            dsDesc.cpuDescriptor             = img->rtvDsvHandle;
            dsDesc.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
            dsDesc.DepthEndingAccess.Type    = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
            dsDescPtr                        = &dsDesc;
        }

        m_cmdLst->BeginRenderPass(numRenderTargets, rtDescs.data(), dsDescPtr, D3D12_RENDER_PASS_FLAG_NONE);
    }

    void ICommandList::EndRenderPass()
    {
        ZoneScoped;
        m_cmdLst->EndRenderPass();
    }

    void ICommandList::BeginComputePass(const ComputePassBeginInfo& /*beginInfo*/)
    {
        ZoneScoped;
        // No explicit compute pass in D3D12
    }

    void ICommandList::EndComputePass()
    {
        ZoneScoped;
    }

    void ICommandList::PushDebugMarker(const char* name, uint32_t bgra)
    {
        ZoneScoped;
        (void)name; (void)bgra;
        // PIX: PIXBeginEvent(m_cmdLst, bgra, name);
    }

    void ICommandList::PopDebugMarker()
    {
        ZoneScoped;
        // PIX: PIXEndEvent(m_cmdLst);
    }

    void ICommandList::InsertDebugMarker(const char* name, uint32_t bgra)
    {
        ZoneScoped;
        (void)name; (void)bgra;
        // PIX: PIXSetMarker(m_cmdLst, bgra, name);
    }

    void ICommandList::BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted)
    {
        ZoneScoped;
        auto* buffer = (IBuffer*)conditionBuffer.buffer;
        m_cmdLst->SetPredication(
            buffer->resource,
            conditionBuffer.offset,
            inverted ? D3D12_PREDICATION_OP_EQUAL_ZERO : D3D12_PREDICATION_OP_NOT_EQUAL_ZERO);
    }

    void ICommandList::EndConditionalCommands()
    {
        ZoneScoped;
        m_cmdLst->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
    }

    void ICommandList::Execute(TL::Span<const CommandList*> commandLists)
    {
        ZoneScoped;
        for (auto* cmdList : commandLists)
            m_cmdLst->ExecuteBundle(((ICommandList*)cmdList)->m_cmdLst);
    }

    void ICommandList::BindPipelineLayout(BindPoint /*bindPoint*/, const PipelineLayout* /*pipelineLayout*/)
    {
        ZoneScoped;
        // Root signature is set during BindGraphicsPipeline / BindComputePipeline
    }

    void ICommandList::SetPushConstants(BindPoint bindPoint, uint32_t offset, TL::Block content)
    {
        ZoneScoped;
        UINT num32BitValues = (UINT)(content.size / 4);
        if (bindPoint == BindPoint::Compute)
            m_cmdLst->SetComputeRoot32BitConstants(offset, num32BitValues, content.ptr, 0);
        else
            m_cmdLst->SetGraphicsRoot32BitConstants(offset, num32BitValues, content.ptr, 0);
    }

    void ICommandList::PushBindGroup(BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos)
    {
        ZoneScoped;
        (void)bindPoint; (void)firstGroup; (void)updateInfos;
        // TODO: implement inline descriptor updates
    }

    void ICommandList::SetBindGroups(BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScoped;
        for (size_t i = 0; i < bindGroups.size(); i++)
        {
            auto* bindGroup = (IBindGroup*)bindGroups[i].bindGroup;
            if (bindPoint == BindPoint::Compute)
                m_cmdLst->SetComputeRootDescriptorTable((UINT)i, bindGroup->heap->GetGPUDescriptorHandleForHeapStart());
            else
                m_cmdLst->SetGraphicsRootDescriptorTable((UINT)i, bindGroup->heap->GetGPUDescriptorHandleForHeapStart());
        }
    }

    void ICommandList::BindGraphicsPipeline(const GraphicsPipeline* pipelineState)
    {
        ZoneScoped;
        auto* pipeline = (IGraphicsPipeline*)pipelineState;
        m_cmdLst->SetPipelineState(pipeline->pipelineState);
        if (pipeline->layout)
            m_cmdLst->SetGraphicsRootSignature(pipeline->layout->rootSignature);
    }

    void ICommandList::BindComputePipeline(const ComputePipeline* pipelineState)
    {
        ZoneScoped;
        auto* pipeline = (IComputePipeline*)pipelineState;
        m_cmdLst->SetPipelineState(pipeline->pipelineState);
        if (pipeline->layout)
            m_cmdLst->SetComputeRootSignature(pipeline->layout->rootSignature);
    }

    void ICommandList::SetViewport(const Viewport& viewport)
    {
        ZoneScoped;
        D3D12_VIEWPORT vp{
            .TopLeftX = viewport.offsetX,
            .TopLeftY = viewport.offsetY,
            .Width    = viewport.width,
            .Height   = viewport.height,
            .MinDepth = viewport.minDepth,
            .MaxDepth = viewport.maxDepth,
        };
        m_cmdLst->RSSetViewports(1, &vp);
    }

    void ICommandList::SetScissor(const Scissor& scissor)
    {
        ZoneScoped;
        D3D12_RECT rect{
            .left   = (LONG)scissor.offsetX,
            .top    = (LONG)scissor.offsetY,
            .right  = (LONG)(scissor.offsetX + scissor.width),
            .bottom = (LONG)(scissor.offsetY + scissor.height),
        };
        m_cmdLst->RSSetScissorRects(1, &rect);
    }

    void ICommandList::BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers)
    {
        ZoneScoped;
        TL::Vector<D3D12_VERTEX_BUFFER_VIEW> views;
        views.reserve(vertexBuffers.size());
        for (size_t i = 0; i < vertexBuffers.size(); i++)
        {
            auto* buffer = (IBuffer*)vertexBuffers[i].buffer;
            views.push_back({
                .BufferLocation = buffer->resource->GetGPUVirtualAddress() + vertexBuffers[i].offset,
                .SizeInBytes    = (UINT)buffer->allocation->GetSize(),
                .StrideInBytes  = GetVertexBufferStrideFromBoundPipeline((uint32_t)i),
            });
        }
        m_cmdLst->IASetVertexBuffers(firstBinding, (UINT)views.size(), views.data());
    }

    void ICommandList::BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType)
    {
        ZoneScoped;
        auto* buffer = (IBuffer*)indexBuffer.buffer;
        D3D12_INDEX_BUFFER_VIEW view{
            .BufferLocation = buffer->resource->GetGPUVirtualAddress() + indexBuffer.offset,
            .SizeInBytes    = (UINT)buffer->allocation->GetSize(),
            .Format         = indexType == IndexType::uint32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT,
        };
        m_cmdLst->IASetIndexBuffer(&view);
        m_cmdLst->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void ICommandList::Draw(const DrawParameters& parameters)
    {
        ZoneScoped;
        m_cmdLst->DrawInstanced(parameters.vertexCount, parameters.instanceCount, parameters.firstVertex, parameters.firstInstance);
    }

    void ICommandList::DrawIndexed(const DrawIndexedParameters& parameters)
    {
        ZoneScoped;
        m_cmdLst->DrawIndexedInstanced(parameters.indexCount, parameters.instanceCount, parameters.firstIndex, parameters.vertexOffset, parameters.firstInstance);
    }

    void ICommandList::DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        ZoneScoped;
        auto* args  = (IBuffer*)argumentBuffer.buffer;
        auto* count = (IBuffer*)countBuffer.buffer;
        auto* sig   = GetCommandSignatureFromBoundPipeline(D3D12_INDIRECT_ARGUMENT_TYPE_DRAW, stride);
        m_cmdLst->ExecuteIndirect(sig, maxDrawCount, args->resource, argumentBuffer.offset, count->resource, countBuffer.offset);
    }

    void ICommandList::DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        ZoneScoped;
        auto* args  = (IBuffer*)argumentBuffer.buffer;
        auto* count = (IBuffer*)countBuffer.buffer;
        auto* sig   = GetCommandSignatureFromBoundPipeline(D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED, stride);
        m_cmdLst->ExecuteIndirect(sig, maxDrawCount, args->resource, argumentBuffer.offset, count->resource, countBuffer.offset);
    }

    void ICommandList::DrawMeshTasks(const DispatchParameters parameters)
    {
        ZoneScoped;
        m_cmdLst->DispatchMesh(parameters.x, parameters.y, parameters.z);
    }

    void ICommandList::DrawMeshTasksIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride)
    {
        ZoneScoped;
        auto* args  = (IBuffer*)argumentBuffer.buffer;
        auto* count = (IBuffer*)countBuffer.buffer;
        auto* sig   = GetCommandSignatureFromBoundPipeline(D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH, stride);
        m_cmdLst->ExecuteIndirect(sig, drawNum, args->resource, argumentBuffer.offset, count->resource, countBuffer.offset);
    }

    void ICommandList::DispatchRays(const DispatchRaysInfo& /*dispatchRaysDesc*/)
    {
        ZoneScoped;
        // TODO: implement ray tracing dispatch
    }

    void ICommandList::DispatchRaysIndirect(const BufferBindingInfo& /*argumentBuffer*/)
    {
        ZoneScoped;
        // TODO: implement indirect ray tracing dispatch
    }

    void ICommandList::Dispatch(const DispatchParameters& parameters)
    {
        ZoneScoped;
        m_cmdLst->Dispatch(parameters.x, parameters.y, parameters.z);
    }

    void ICommandList::DispatchIndirect(const BufferBindingInfo& argumentBuffer)
    {
        ZoneScoped;
        auto* buf = (IBuffer*)argumentBuffer.buffer;
        auto* sig = GetCommandSignatureFromBoundPipeline(D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH);
        m_cmdLst->ExecuteIndirect(sig, 1, buf->resource, argumentBuffer.offset, nullptr, 0);
    }

    void ICommandList::CopyBuffer(const BufferCopyInfo& copyInfo)
    {
        ZoneScoped;
        auto* src = (IBuffer*)copyInfo.srcBuffer;
        auto* dst = (IBuffer*)copyInfo.dstBuffer;
        m_cmdLst->CopyBufferRegion(dst->resource, copyInfo.dstOffset, src->resource, copyInfo.srcOffset, copyInfo.size);
    }

    void ICommandList::CopyImage(const ImageCopyInfo& copyInfo)
    {
        ZoneScoped;
        auto* src = (IImage*)copyInfo.srcImage;
        auto* dst = (IImage*)copyInfo.dstImage;

        D3D12_TEXTURE_COPY_LOCATION srcLoc{
            .pResource        = src->resource,
            .Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
            .SubresourceIndex = copyInfo.srcSubresource.arrayBase,
        };
        D3D12_TEXTURE_COPY_LOCATION dstLoc{
            .pResource        = dst->resource,
            .Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
            .SubresourceIndex = copyInfo.dstSubresource.arrayBase,
        };
        D3D12_BOX box{
            .left   = (UINT)copyInfo.srcOffset.x,
            .top    = (UINT)copyInfo.srcOffset.y,
            .front  = (UINT)copyInfo.srcOffset.z,
            .right  = (UINT)(copyInfo.srcOffset.x + copyInfo.srcSize.width),
            .bottom = (UINT)(copyInfo.srcOffset.y + copyInfo.srcSize.height),
            .back   = (UINT)(copyInfo.srcOffset.z + copyInfo.srcSize.depth),
        };
        m_cmdLst->CopyTextureRegion(&dstLoc, copyInfo.dstOffset.x, copyInfo.dstOffset.y, copyInfo.dstOffset.z, &srcLoc, &box);
    }

    void ICommandList::CopyImageToBuffer(const BufferImageCopyInfo& copyInfo)
    {
        ZoneScoped;
        auto* img = (IImage*)copyInfo.image;
        auto* buf = (IBuffer*)copyInfo.buffer;

        D3D12_TEXTURE_COPY_LOCATION srcLoc{
            .pResource        = img->resource,
            .Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
            .SubresourceIndex = copyInfo.subresource.arrayBase,
        };
        D3D12_TEXTURE_COPY_LOCATION dstLoc{
            .pResource       = buf->resource,
            .Type            = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
            .PlacedFootprint = {
                .Offset    = copyInfo.bufferOffset,
                .Footprint = {
                    .Format   = DXGI_FORMAT_UNKNOWN,
                    .Width    = copyInfo.imageSize.width,
                    .Height   = copyInfo.imageSize.height,
                    .Depth    = copyInfo.imageSize.depth,
                    .RowPitch = copyInfo.bytesPerRow,
                },
            },
        };
        D3D12_BOX box{
            .left   = (UINT)copyInfo.imageOffset.x,
            .top    = (UINT)copyInfo.imageOffset.y,
            .front  = (UINT)copyInfo.imageOffset.z,
            .right  = (UINT)(copyInfo.imageOffset.x + copyInfo.imageSize.width),
            .bottom = (UINT)(copyInfo.imageOffset.y + copyInfo.imageSize.height),
            .back   = (UINT)(copyInfo.imageOffset.z + copyInfo.imageSize.depth),
        };
        m_cmdLst->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, &box);
    }

    void ICommandList::CopyBufferToImage(const BufferImageCopyInfo& copyInfo)
    {
        ZoneScoped;
        auto* buf = (IBuffer*)copyInfo.buffer;
        auto* img = (IImage*)copyInfo.image;

        D3D12_TEXTURE_COPY_LOCATION srcLoc{
            .pResource       = buf->resource,
            .Type            = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
            .PlacedFootprint = {
                .Offset    = copyInfo.bufferOffset,
                .Footprint = {
                    .Format   = DXGI_FORMAT_UNKNOWN,
                    .Width    = copyInfo.imageSize.width,
                    .Height   = copyInfo.imageSize.height,
                    .Depth    = copyInfo.imageSize.depth,
                    .RowPitch = copyInfo.bytesPerRow,
                },
            },
        };
        D3D12_TEXTURE_COPY_LOCATION dstLoc{
            .pResource        = img->resource,
            .Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
            .SubresourceIndex = copyInfo.subresource.arrayBase,
        };
        m_cmdLst->CopyTextureRegion(&dstLoc, (UINT)copyInfo.imageOffset.x, (UINT)copyInfo.imageOffset.y, (UINT)copyInfo.imageOffset.z, &srcLoc, nullptr);
    }

    // RT/Micromap stubs
    void ICommandList::BuildMicromaps(TL::Span<const BuildMicromapDesc> /*descs*/) {}
    void ICommandList::WriteMicromapsSizes(TL::Span<const Micromap*> /*micromaps*/, QueryPool* /*queryPool*/, uint32_t /*offset*/) {}
    void ICommandList::CopyMicromap(Micromap* /*dst*/, const Micromap* /*src*/, CopyMode /*mode*/) {}
    void ICommandList::BuildTopLevelAccelerationStructures(TL::Span<const BuildTopLevelAccelerationStructureDesc> /*descs*/) {}
    void ICommandList::BuildBottomLevelAccelerationStructures(TL::Span<const BuildBottomLevelAccelerationStructureDesc> /*descs*/) {}
    void ICommandList::WriteAccelerationStructuresSizes(TL::Span<const AccelerationStructure*> /*as*/, QueryPool* /*queryPool*/, uint32_t /*offset*/) {}
    void ICommandList::CopyAccelerationStructure(AccelerationStructure* /*dst*/, const AccelerationStructure* /*src*/, CopyMode /*mode*/) {}

    ///////////////////////////////////////////////////////////
    // Helpers
    ///////////////////////////////////////////////////////////

    UINT ICommandList::GetVertexBufferStrideFromBoundPipeline(uint32_t /*binding*/)
    {
        // TODO: cache bound pipeline and query vertex input stride
        return 0;
    }

    ID3D12CommandSignature* ICommandList::GetCommandSignatureFromBoundPipeline(D3D12_INDIRECT_ARGUMENT_TYPE /*type*/, UINT /*stride*/)
    {
        // TODO: create/cache command signatures per pipeline
        return nullptr;
    }

} // namespace RHI::D3D12
