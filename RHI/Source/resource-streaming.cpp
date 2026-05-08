#include <RHI/resource-streaming.h>
#include <TL/Assert.hpp>

#include <cstring>

#include <tracy/Tracy.hpp>

namespace RHI
{
    ResultCode StreamingBuffer::init(uint64_t capacity, uint32_t streamingFramesInFlightCount, Device* pDevice, QueueType queueType, Queue* queue)
    {
        ZoneScoped;

        TL_ASSERT(streamingFramesInFlightCount > 0 && streamingFramesInFlightCount <= MaxStreamingFrames);

        device                = pDevice;
        transferQueue         = queue;
        stagingBufferCapacity = capacity;
        framesInFlightCount   = streamingFramesInFlightCount;
        activeFrameIndex      = 0;
        fenceValue            = 0;
        for (uint32_t i = 0; i < MaxStreamingFrames; i++)
        {
            transferCommandPool[i]       = nullptr;
            stagingBufferCursor[i]       = 0;
            stagingBufferRegionSignal[i] = 0;
        }

        stagingBuffer = device->CreateBuffer({
            .name       = "StreamingBuffer",
            .usageFlags = BufferUsage::HostMapped | BufferUsage::CopySrc,
            .byteSize   = capacity,
        });
        if (stagingBuffer == nullptr)
            return ResultCode::ErrorAllocationFailed;

        stagingBufferMappedPtr = device->MapBuffer(stagingBuffer, 0, capacity);
        if (stagingBufferMappedPtr == nullptr)
        {
            device->DestroyBuffer(stagingBuffer);
            stagingBuffer = nullptr;
            return ResultCode::ErrorAllocationFailed;
        }

        fence = device->CreateFence({.name = "StreamingFence", .initialValue = 0});
        if (fence == nullptr)
        {
            device->UnmapBuffer(stagingBuffer);
            device->DestroyBuffer(stagingBuffer);
            stagingBuffer          = nullptr;
            stagingBufferMappedPtr = nullptr;
            return ResultCode::ErrorAllocationFailed;
        }

        for (uint32_t i = 0; i < framesInFlightCount; i++)
        {
            transferCommandPool[i] = device->CreateCommandPool({.name = "StreamingCommandPool", .queue = queueType});
            if (transferCommandPool[i] == nullptr)
            {
                for (uint32_t j = 0; j < i; j++)
                    device->DestroyCommandPool(transferCommandPool[j]);
                device->DestroyFence(fence);
                device->UnmapBuffer(stagingBuffer);
                device->DestroyBuffer(stagingBuffer);
                fence                  = nullptr;
                stagingBufferMappedPtr = nullptr;
                stagingBuffer          = nullptr;
                return ResultCode::ErrorAllocationFailed;
            }
        }

        return ResultCode::Success;
    }

    void StreamingBuffer::shutdown()
    {
        ZoneScoped;

        flush();
        transferQueue->WaitFence(fence, fenceValue);

        device->DestroyFence(fence);

        for (uint32_t i = 0; i < framesInFlightCount; i++)
            device->DestroyCommandPool(transferCommandPool[i]);

        device->UnmapBuffer(stagingBuffer);
        device->DestroyBuffer(stagingBuffer);

        fence                  = nullptr;
        stagingBufferMappedPtr = nullptr;
        stagingBuffer          = nullptr;
    }

    bool StreamingBuffer::tryAllocate(TL::Block block, uint32_t alignment, StagingBlock& outBlock)
    {
        ZoneScoped;

        uint64_t regionSize  = stagingBufferCapacity / framesInFlightCount;
        uint64_t regionStart = activeFrameIndex * regionSize;
        uint64_t cursor      = stagingBufferCursor[activeFrameIndex];

        uint64_t alignedCursor = alignment > 1 ? (cursor + alignment - 1) & ~(uint64_t(alignment) - 1) : cursor;
        if (alignedCursor + block.size > regionSize)
            return false;

        outBlock.buffer = stagingBuffer;
        outBlock.offset = regionStart + alignedCursor;
        outBlock.size   = block.size;

        memcpy(static_cast<char*>(stagingBufferMappedPtr) + regionStart + alignedCursor, block.ptr, block.size);

        stagingBufferCursor[activeFrameIndex] = alignedCursor + block.size;
        return true;
    }

    bool StreamingBuffer::tryStreamImage(const ImageCopyInfo& dstImage, ImageSize3D size, uint32_t bytesPerRow, uint32_t rowsPerImage, TL::Block data, ImageBarrierState srcState, ImageBarrierState dstState)
    {
        ZoneScoped;

        StagingBlock block;
        if (!tryAllocate(data, 256, block))
            return false;

        m_pendingImageWrites.push_back(PendingImageWrite{
            .dstImage        = dstImage,
            .size            = size,
            .layout          = {.offset = block.offset, .bytesPerRow = bytesPerRow, .rowsPerImage = rowsPerImage},
            .stagingBlock    = block,
            .srcBarrierState = srcState,
            .dstBarrierState = dstState,
        });
        return true;
    }

    bool StreamingBuffer::tryStreamBuffer(const Buffer* buffer, uint64_t offset, TL::Block data, BufferBarrierState srcState, BufferBarrierState dstState)
    {
        ZoneScoped;

        StagingBlock block;
        if (!tryAllocate(data, 1, block))
            return false;

        m_pendingBufferWrites.push_back(PendingBufferWrite{
            .dstBuffer       = const_cast<Buffer*>(buffer),
            .dstOffset       = offset,
            .block           = block,
            .srcBarrierState = srcState,
            .dstBarrierState = dstState,
        });
        return true;
    }

    void StreamingBuffer::streamImage(const ImageCopyInfo& dstImage, ImageSize3D size, uint32_t bytesPerRow, uint32_t rowsPerImage, TL::Block data, ImageBarrierState srcState, ImageBarrierState dstState)
    {
        ZoneScoped;

        bool success = tryStreamImage(dstImage, size, bytesPerRow, rowsPerImage, data, srcState, dstState);
        if (!success)
        {
            flush();
            success = tryStreamImage(dstImage, size, bytesPerRow, rowsPerImage, data, srcState, dstState);
            TL_ASSERT(success, "Unexpected!");
        }
    }

    void StreamingBuffer::streamBuffer(Buffer* buffer, uint64_t offset, TL::Block data, BufferBarrierState srcState, BufferBarrierState dstState)
    {
        ZoneScoped;

        bool success = tryStreamBuffer(buffer, offset, data, srcState, dstState);
        if (!success)
        {
            flush();
            success = tryStreamBuffer(buffer, offset, data, srcState, dstState);
            TL_ASSERT(success, "Unexpected!");
        }
    }

    void StreamingBuffer::flush()
    {
        ZoneScoped;

        if (m_pendingBufferWrites.empty() && m_pendingImageWrites.empty())
            return;

        CommandList* commandList = transferCommandPool[activeFrameIndex]->Allocate();
        commandList->Begin();

        // Emit pre-copy barriers to transition resources to CopyDst state
        {
            TL::Vector<ImageBarrierInfo>  imageBarriers;
            TL::Vector<BufferBarrierInfo> bufferBarriers;

            // One barrier per unique image (multiple mip writes share the same image handle)
            for (const auto& write : m_pendingImageWrites)
            {
                bool seen = false;
                for (const auto& b : imageBarriers)
                    if (b.image == write.dstImage.image)
                    {
                        seen = true;
                        break;
                    }
                if (!seen)
                {
                    imageBarriers.push_back(ImageBarrierInfo{
                        .image    = write.dstImage.image,
                        .srcState = {.usage = ImageUsage::None, .stage = PipelineStage::None, .access = Access::None},
                        .dstState = {.usage = ImageUsage::CopyDst, .stage = PipelineStage::Copy, .access = Access::Write},
                    });
                }
            }

            // Add barrier for each pending buffer write
            for (const auto& write : m_pendingBufferWrites)
            {
                bufferBarriers.push_back(BufferBarrierInfo{
                    .buffer    = write.dstBuffer,
                    .srcState  = {.usage = BufferUsage::None, .stage = PipelineStage::None, .access = Access::None},
                    .dstState  = {.usage = BufferUsage::CopyDst, .stage = PipelineStage::Copy, .access = Access::Write},
                    .subregion = {.offset = write.dstOffset, .size = write.block.size},
                });
            }

            if (!imageBarriers.empty() || !bufferBarriers.empty())
            {
                commandList->AddPipelineBarrier({}, imageBarriers, bufferBarriers);
            }
        }

        for (const auto& write : m_pendingBufferWrites)
            commandList->CopyBuffer(write.block.buffer, write.block.offset, write.dstBuffer, write.dstOffset, write.block.size);
        for (const auto& write : m_pendingImageWrites)
            commandList->CopyBufferToImage(write.stagingBlock.buffer, write.dstImage, write.layout);

        // Emit post-copy barriers to transition images to shader-readable state
        {
            TL::Vector<ImageBarrierInfo> imageBarriers;

            // One barrier per unique image
            for (const auto& write : m_pendingImageWrites)
            {
                bool seen = false;
                for (const auto& b : imageBarriers)
                    if (b.image == write.dstImage.image)
                    {
                        seen = true;
                        break;
                    }
                if (!seen)
                {
                    imageBarriers.push_back(ImageBarrierInfo{
                        .image    = write.dstImage.image,
                        .srcState = {.usage = ImageUsage::CopyDst, .stage = PipelineStage::Copy, .access = Access::Write},
                        .dstState = {.usage = ImageUsage::ShaderResource, .stage = PipelineStage::AllCommands, .access = Access::Read},
                    });
                }
            }

            if (!imageBarriers.empty())
            {
                commandList->AddPipelineBarrier({}, imageBarriers, {});
            }
        }

        // Clear pending operations for next batch
        m_pendingBufferWrites.clear();
        m_pendingImageWrites.clear();

        commandList->End();

        fenceValue++;
        stagingBufferRegionSignal[activeFrameIndex] = fenceValue;

        transferQueue->Submit({
            .commandLists = commandList,
            .signalFences = {{.fence = fence, .value = fenceValue, .stage = PipelineStage::Copy}},
        });

        // Advance to the next frame's region.
        activeFrameIndex = (activeFrameIndex + 1) % framesInFlightCount;

        // Only block if the slot we're about to reuse is still in flight on the GPU.
        if (stagingBufferRegionSignal[activeFrameIndex] > 0 && stagingBufferRegionSignal[activeFrameIndex] > fenceValue - framesInFlightCount)
            transferQueue->WaitFence(fence, stagingBufferRegionSignal[activeFrameIndex]);

        stagingBufferCursor[activeFrameIndex] = 0;

        transferCommandPool[activeFrameIndex]->Reset();
    }
} // namespace RHI
