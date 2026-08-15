#include "command-list.h"

#include "device.h"
#include "resources.h"

#include <TL/Allocator/Allocator.hpp>
#include <TL/Containers/InlineVector.hpp>

#include <algorithm>

namespace RHI::WebGPU
{
    inline static WGPUStringView MakeStringView(const char* value)
    {
        return WGPUStringView{
            .data = value,
            .length = WGPU_STRLEN,
        };
    }

    inline static WGPULoadOp ConvertLoadOperation(LoadOperation operation)
    {
        return operation == LoadOperation::Load ? WGPULoadOp_Load : WGPULoadOp_Clear;
    }

    inline static WGPUStoreOp ConvertStoreOperation(StoreOperation operation)
    {
        return operation == StoreOperation::Store ? WGPUStoreOp_Store : WGPUStoreOp_Discard;
    }

    inline static WGPUTextureAspect ConvertImageAspect(ImageAspect aspect)
    {
        if (aspect == ImageAspect::Depth)
            return WGPUTextureAspect_DepthOnly;
        if (aspect == ImageAspect::Stencil)
            return WGPUTextureAspect_StencilOnly;
        return WGPUTextureAspect_All;
    }

    inline static uint64_t ResolveBufferRange(const IBuffer* buffer, uint64_t offset, uint64_t range)
    {
        if (range == 0 || range == RemainingSize)
            return buffer->size - offset;
        return range;
    }

    inline static WGPUColor MakeClearColor(const IImage* image, const ClearValue& value)
    {
        FormatInfo formatInfo = GetFormatInfo(image->format);
        if (formatInfo.type == FormatType::Integer && !formatInfo.isSigned)
        {
            return WGPUColor{
                .r = (double)value.u32.r,
                .g = (double)value.u32.g,
                .b = (double)value.u32.b,
                .a = (double)value.u32.a,
            };
        }
        if (formatInfo.type == FormatType::Integer && formatInfo.isSigned)
        {
            return WGPUColor{
                .r = (double)value.i32.r,
                .g = (double)value.i32.g,
                .b = (double)value.i32.b,
                .a = (double)value.i32.a,
            };
        }
        return WGPUColor{
            .r = value.f32.r,
            .g = value.f32.g,
            .b = value.f32.b,
            .a = value.f32.a,
        };
    }

    inline static WGPUTexelCopyTextureInfo MakeTextureCopy(const ImageCopyInfo& copyInfo)
    {
        auto* image = (IImage*)copyInfo.image;
        return WGPUTexelCopyTextureInfo{
            .texture = image->handle,
            .mipLevel = copyInfo.mipLevel,
            .origin = WGPUOrigin3D{
                .x = (uint32_t)std::max(copyInfo.offset.x, 0),
                .y = (uint32_t)std::max(copyInfo.offset.y, 0),
                .z = image->type == ImageType::Image3D ? (uint32_t)copyInfo.offset.z : copyInfo.arrayLayer,
            },
            .aspect = ConvertImageAspect(copyInfo.aspect),
        };
    }

    inline static WGPUExtent3D GetMipExtent(const IImage* image, uint32_t mipLevel)
    {
        return WGPUExtent3D{
            .width = std::max(1u, image->size.width >> mipLevel),
            .height = std::max(1u, image->size.height >> mipLevel),
            .depthOrArrayLayers = image->type == ImageType::Image3D ? std::max(1u, image->size.depth >> mipLevel) : 1u,
        };
    }

    void ICommandList::Reset()
    {
        wgpuCommandBufferRelease(commandBuffer);
        commandBuffer = nullptr;
    }

    void ICommandPool::Init(IDevice* device)
    {
        this->device = device;
    }

    void ICommandPool::Shutdown(IDevice* device)
    {
        for (ICommandList* commandList : commandLists)
        {
            commandList->Reset();
            TL::destructFrom(TL::Context::getDefaultAllocator(), commandList);
        }
        commandLists.clear();
        this->device = nullptr;
    }

    void commandPoolReset(ICommandPool* self)
    {
        for (ICommandList* commandList : self->commandLists)
        {
            commandList->Reset();
            TL::destructFrom(TL::Context::getDefaultAllocator(), commandList);
        }
        self->commandLists.clear();
    }

    CommandList* commandPoolAllocate(ICommandPool* self)
    {
        auto* commandList = TL::constructFrom<ICommandList>(TL::Context::getDefaultAllocator());
        commandList->device = self->device;
        self->commandLists.push_back(commandList);
        return commandList;
    }

    void cmdBegin(ICommandList* self)
    {
        WGPUCommandEncoderDescriptor descriptor{
            .nextInChain = nullptr,
            .label = {},
        };
        self->encoder = wgpuDeviceCreateCommandEncoder(self->device->m_device, &descriptor);
    }

    void cmdEnd(ICommandList* self)
    {
        WGPUCommandBufferDescriptor descriptor{
            .nextInChain = nullptr,
            .label = {},
        };
        self->commandBuffer = wgpuCommandEncoderFinish(self->encoder, &descriptor);
        wgpuCommandEncoderRelease(self->encoder);
        self->encoder = nullptr;
    }

    void cmdPushDebugMarker(ICommandList* self, const char* name, uint32_t bgra)
    {
        wgpuCommandEncoderPushDebugGroup(self->encoder, MakeStringView(name));
    }

    void cmdPopDebugMarker(ICommandList* self)
    {
        wgpuCommandEncoderPopDebugGroup(self->encoder);
    }

    void cmdInsertDebugMarker(ICommandList* self, const char* name, uint32_t bgra)
    {
        wgpuCommandEncoderInsertDebugMarker(self->encoder, MakeStringView(name));
    }

    void cmdAddPipelineBarrier(ICommandList* self, TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers)
    {
        // WebGPU performs resource-state transitions implicitly.
    }

    void cmdBeginRenderPass(ICommandList* self, const RenderPassBeginInfo& beginInfo)
    {
        TL::InlineVector<WGPURenderPassColorAttachment, MaxColorAttachments> colorAttachments;
        for (auto attachment : beginInfo.colorAttachments)
        {
            auto* image = (IImage*)attachment.view;
            auto* resolveImage = (IImage*)attachment.resolveView;
            colorAttachments.push_back(WGPURenderPassColorAttachment{
                .nextInChain = nullptr,
                .view = image->viewHandle,
                .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
                .resolveTarget = attachment.resolveMode == ResolveMode::Avg ? resolveImage->viewHandle : nullptr,
                .loadOp = ConvertLoadOperation(attachment.loadOp),
                .storeOp = ConvertStoreOperation(attachment.storeOp),
                .clearValue = MakeClearColor(image, attachment.clearValue),
            });
        }

        auto* depthImage = (IImage*)beginInfo.depthStencilAttachment.view;
        FormatInfo formatInfo = GetFormatInfo(depthImage->format);
        WGPURenderPassDepthStencilAttachment depthAttachment{
            .nextInChain = nullptr,
            .view = depthImage->viewHandle,
            .depthLoadOp = formatInfo.hasDepth ? ConvertLoadOperation(beginInfo.depthStencilAttachment.depthLoadOp) : WGPULoadOp_Undefined,
            .depthStoreOp = formatInfo.hasDepth ? ConvertStoreOperation(beginInfo.depthStencilAttachment.depthStoreOp) : WGPUStoreOp_Undefined,
            .depthClearValue = beginInfo.depthStencilAttachment.clearValue.depthValue,
            .depthReadOnly = WGPU_FALSE,
            .stencilLoadOp = formatInfo.hasStencil ? ConvertLoadOperation(beginInfo.depthStencilAttachment.stencilLoadOp) : WGPULoadOp_Undefined,
            .stencilStoreOp = formatInfo.hasStencil ? ConvertStoreOperation(beginInfo.depthStencilAttachment.stencilStoreOp) : WGPUStoreOp_Undefined,
            .stencilClearValue = beginInfo.depthStencilAttachment.clearValue.stencilValue,
            .stencilReadOnly = WGPU_FALSE,
        };

        WGPURenderPassDescriptor descriptor{
            .nextInChain = nullptr,
            .label = {},
            .colorAttachmentCount = colorAttachments.size(),
            .colorAttachments = colorAttachments.data(),
            .depthStencilAttachment = &depthAttachment,
            .occlusionQuerySet = nullptr,
            .timestampWrites = nullptr,
        };
        self->renderPass = wgpuCommandEncoderBeginRenderPass(self->encoder, &descriptor);
    }

    void cmdEndRenderPass(ICommandList* self)
    {
        wgpuRenderPassEncoderEnd(self->renderPass);
        wgpuRenderPassEncoderRelease(self->renderPass);
        self->renderPass = nullptr;
    }

    void cmdBeginComputePass(ICommandList* self, const ComputePassBeginInfo& beginInfo)
    {
        WGPUComputePassDescriptor descriptor{
            .nextInChain = nullptr,
            .label = MakeStringView(beginInfo.name),
            .timestampWrites = nullptr,
        };
        self->computePass = wgpuCommandEncoderBeginComputePass(self->encoder, &descriptor);
    }

    void cmdEndComputePass(ICommandList* self)
    {
        wgpuComputePassEncoderEnd(self->computePass);
        wgpuComputePassEncoderRelease(self->computePass);
        self->computePass = nullptr;
    }

    void cmdBeginConditionalCommands(ICommandList* self, const BufferBindingInfo& conditionBuffer, bool inverted)
    {
        // Conditional rendering is not exposed by WebGPU.
    }

    void cmdEndConditionalCommands(ICommandList* self)
    {
        // Conditional rendering is not exposed by WebGPU.
    }

    void cmdExecute(ICommandList* self, TL::Span<const CommandList*> commandLists)
    {
        // WebGPU does not expose secondary command buffers.
    }

    void cmdBindPipelineLayout(ICommandList* self, BindPoint bindPoint, const PipelineLayout* pipelineLayout)
    {
    }

    void cmdSetPushConstants(ICommandList* self, BindPoint bindPoint, uint32_t offset, TL::Block content)
    {
        if (bindPoint == BindPoint::Graphics)
        {
            wgpuRenderPassEncoderSetImmediates(self->renderPass, offset, content.ptr, content.size);
        }
        else
        {
            wgpuComputePassEncoderSetImmediates(self->computePass, offset, content.ptr, content.size);
        }
    }

    void cmdPushBindGroup(ICommandList* self, BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos)
    {
        // WebGPU bind groups are immutable and cannot be pushed.
    }

    void cmdSetBindGroups(ICommandList* self, BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        for (uint32_t groupIndex = 0; groupIndex < bindGroups.size(); ++groupIndex)
        {
            auto bindingInfo = bindGroups[groupIndex];
            auto* bindGroup = (IBindGroup*)bindingInfo.bindGroup;
            if (bindPoint == BindPoint::Graphics)
            {
                wgpuRenderPassEncoderSetBindGroup(
                    self->renderPass,
                    groupIndex,
                    bindGroup->handle,
                    bindingInfo.dynamicOffsets.size(),
                    bindingInfo.dynamicOffsets.data());
            }
            else
            {
                wgpuComputePassEncoderSetBindGroup(
                    self->computePass,
                    groupIndex,
                    bindGroup->handle,
                    bindingInfo.dynamicOffsets.size(),
                    bindingInfo.dynamicOffsets.data());
            }
        }
    }

    void cmdBindGraphicsPipeline(ICommandList* self, const GraphicsPipeline* pipelineState)
    {
        auto* pipeline = (const IGraphicsPipeline*)pipelineState;
        wgpuRenderPassEncoderSetPipeline(self->renderPass, pipeline->handle);
    }

    void cmdBindComputePipeline(ICommandList* self, const ComputePipeline* pipelineState)
    {
        auto* pipeline = (const IComputePipeline*)pipelineState;
        wgpuComputePassEncoderSetPipeline(self->computePass, pipeline->handle);
    }

    void cmdBindRayTracingPipeline(ICommandList* self, const RayTracingPipeline* pipelineState)
    {
        // Ray tracing is not exposed by WebGPU.
    }

    void cmdSetViewport(ICommandList* self, float offsetX, float offsetY, float width, float height, float minDepth, float maxDepth)
    {
        wgpuRenderPassEncoderSetViewport(self->renderPass, offsetX, offsetY, width, height, minDepth, maxDepth);
    }

    void cmdSetScissor(ICommandList* self, int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height)
    {
        wgpuRenderPassEncoderSetScissorRect(self->renderPass, (uint32_t)offsetX, (uint32_t)offsetY, width, height);
    }

    void cmdBindVertexBuffers(ICommandList* self, uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers)
    {
        for (uint32_t index = 0; index < vertexBuffers.size(); ++index)
        {
            auto binding = vertexBuffers[index];
            auto* buffer = (IBuffer*)binding.buffer;
            wgpuRenderPassEncoderSetVertexBuffer(
                self->renderPass,
                firstBinding + index,
                buffer->handle,
                binding.offset,
                ResolveBufferRange(buffer, binding.offset, binding.range));
        }
    }

    void cmdBindIndexBuffer(ICommandList* self, const BufferBindingInfo& indexBuffer, IndexType indexType)
    {
        auto* buffer = (IBuffer*)indexBuffer.buffer;
        WGPUIndexFormat format = indexType == IndexType::uint32 ? WGPUIndexFormat_Uint32 : WGPUIndexFormat_Uint16;
        wgpuRenderPassEncoderSetIndexBuffer(
            self->renderPass,
            buffer->handle,
            format,
            indexBuffer.offset,
            ResolveBufferRange(buffer, indexBuffer.offset, indexBuffer.range));
    }

    void cmdDraw(ICommandList* self, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        wgpuRenderPassEncoderDraw(self->renderPass, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void cmdDrawIndexed(ICommandList* self, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
    {
        wgpuRenderPassEncoderDrawIndexed(self->renderPass, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void cmdDrawMeshTasks(ICommandList* self, uint32_t x, uint32_t y, uint32_t z)
    {
        // Mesh shading is not exposed by WebGPU.
    }

    void cmdDrawIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        auto* arguments = (IBuffer*)argumentBuffer.buffer;
        for (uint32_t drawIndex = 0; drawIndex < maxDrawCount; ++drawIndex)
            wgpuRenderPassEncoderDrawIndirect(self->renderPass, arguments->handle, argumentBuffer.offset + (uint64_t)drawIndex * stride);
    }

    void cmdDrawIndexedIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        auto* arguments = (IBuffer*)argumentBuffer.buffer;
        for (uint32_t drawIndex = 0; drawIndex < maxDrawCount; ++drawIndex)
            wgpuRenderPassEncoderDrawIndexedIndirect(self->renderPass, arguments->handle, argumentBuffer.offset + (uint64_t)drawIndex * stride);
    }

    void cmdDrawMeshTasksIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride)
    {
        // Mesh shading is not exposed by WebGPU.
    }

    void cmdDispatch(ICommandList* self, uint32_t x, uint32_t y, uint32_t z)
    {
        wgpuComputePassEncoderDispatchWorkgroups(self->computePass, x, y, z);
    }

    void cmdDispatchIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer)
    {
        auto* buffer = (IBuffer*)argumentBuffer.buffer;
        wgpuComputePassEncoderDispatchWorkgroupsIndirect(self->computePass, buffer->handle, argumentBuffer.offset);
    }

    void cmdDispatchRays(ICommandList* self, const DispatchRaysInfo& dispatchRaysDesc)
    {
        // Ray tracing is not exposed by WebGPU.
    }

    void cmdDispatchRaysIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer)
    {
        // Ray tracing is not exposed by WebGPU.
    }

    void cmdCopyBuffer(ICommandList* self, const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size)
    {
        auto* source = (const IBuffer*)srcBuffer;
        auto* destination = (const IBuffer*)dstBuffer;
        wgpuCommandEncoderCopyBufferToBuffer(self->encoder, source->handle, srcOffset, destination->handle, dstOffset, size);
    }

    void cmdCopyImage(ICommandList* self, const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size)
    {
        WGPUTexelCopyTextureInfo source = MakeTextureCopy(srcImage);
        WGPUTexelCopyTextureInfo destination = MakeTextureCopy(dstImage);
        WGPUExtent3D extent{
            .width = size.width,
            .height = size.height,
            .depthOrArrayLayers = size.depth,
        };
        wgpuCommandEncoderCopyTextureToTexture(self->encoder, &source, &destination, &extent);
    }

    void cmdCopyImageToBuffer(ICommandList* self, const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer)
    {
        auto* image = (IImage*)srcImage.image;
        auto* buffer = (const IBuffer*)dstBuffer;
        WGPUTexelCopyTextureInfo source = MakeTextureCopy(srcImage);
        WGPUTexelCopyBufferInfo destination{
            .layout = WGPUTexelCopyBufferLayout{
                .offset = layout.offset,
                .bytesPerRow = layout.bytesPerRow ? layout.bytesPerRow : WGPU_COPY_STRIDE_UNDEFINED,
                .rowsPerImage = layout.rowsPerImage ? layout.rowsPerImage : WGPU_COPY_STRIDE_UNDEFINED,
            },
            .buffer = buffer->handle,
        };
        WGPUExtent3D extent = GetMipExtent(image, srcImage.mipLevel);
        wgpuCommandEncoderCopyTextureToBuffer(self->encoder, &source, &destination, &extent);
    }

    void cmdCopyBufferToImage(ICommandList* self, const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout)
    {
        auto* buffer = (const IBuffer*)srcBuffer;
        auto* image = (IImage*)dstImage.image;
        WGPUTexelCopyBufferInfo source{
            .layout = WGPUTexelCopyBufferLayout{
                .offset = layout.offset,
                .bytesPerRow = layout.bytesPerRow ? layout.bytesPerRow : WGPU_COPY_STRIDE_UNDEFINED,
                .rowsPerImage = layout.rowsPerImage ? layout.rowsPerImage : WGPU_COPY_STRIDE_UNDEFINED,
            },
            .buffer = buffer->handle,
        };
        WGPUTexelCopyTextureInfo destination = MakeTextureCopy(dstImage);
        WGPUExtent3D extent = GetMipExtent(image, dstImage.mipLevel);
        wgpuCommandEncoderCopyBufferToTexture(self->encoder, &source, &destination, &extent);
    }

    void cmdCopyAccelerationStructure(ICommandList* self, AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode)
    {
        // Acceleration structures are not exposed by WebGPU.
    }

    void cmdCopyMicromap(ICommandList* self, Micromap* dst, const Micromap* src, CopyMode copyMode)
    {
        // Micromaps are not exposed by WebGPU.
    }

    void cmdClearBuffer(ICommandList* self, Buffer* dst, size_t offset, size_t size)
    {
        auto* buffer = (IBuffer*)dst;
        uint64_t clearSize = size == (size_t)RemainingSize ? buffer->size - offset : size;
        wgpuCommandEncoderClearBuffer(self->encoder, buffer->handle, offset, clearSize);
    }

    void cmdBuildTlas(ICommandList* self, TL::Span<const TlasBuildInfo> buildInfos)
    {
        // Acceleration structures are not exposed by WebGPU.
    }

    void cmdBuildBlas(ICommandList* self, TL::Span<const BlasBuildInfo> buildInfos)
    {
        // Acceleration structures are not exposed by WebGPU.
    }

    void cmdBuildMicromaps(ICommandList* self, TL::Span<const MicromapBuildInfo> buildInfos)
    {
        // Micromaps are not exposed by WebGPU.
    }

    void cmdWriteAccelerationStructuresSizes(ICommandList* self, TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset)
    {
        // Acceleration structures are not exposed by WebGPU.
    }

    void cmdWriteMicromapsSizes(ICommandList* self, TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset)
    {
        // Micromaps are not exposed by WebGPU.
    }
} // namespace RHI::WebGPU
