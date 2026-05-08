#pragma once

#include <RHI/Resources.hpp>
#include <RHI/CommandList.hpp>
#include <RHI/Device.hpp>
#include <RHI/Format.hpp>

namespace RHI
{
    struct StagingBlock
    {
        Buffer*  buffer;
        uint64_t offset;
        uint64_t size;
    };

    struct PendingBufferWrite
    {
        Buffer*            dstBuffer;
        uint64_t           dstOffset;
        StagingBlock       block;
        BufferBarrierState srcBarrierState;
        BufferBarrierState dstBarrierState;
    };

    struct PendingImageWrite
    {
        ImageCopyInfo     dstImage;
        ImageSize3D       size;
        ImageMemoryLayout layout;
        StagingBlock      stagingBlock;
        ImageBarrierState srcBarrierState;
        ImageBarrierState dstBarrierState;
    };

    inline ImageMemoryLayout GetImageMemoryLayout(Format format, ImageSize3D size, uint64_t offset = 0)
    {
        const FormatInfo& info = GetFormatInfo(format);
        return ImageMemoryLayout{
            .offset       = offset,
            .bytesPerRow  = ((size.width + info.blockSize - 1) / info.blockSize) * info.bytesPerBlock,
            .rowsPerImage = (size.height + info.blockSize - 1) / info.blockSize,
        };
    }

    // TODO: Handle case when attempting to stream data with size larger total staging-capacity ~= [capacity/uploadFramesCount]

    struct StreamingBuffer
    {
        ResultCode init(uint64_t capacity, uint32_t uploadFramesCount, Device* device, QueueType queueType, Queue* queue);
        void       shutdown();

        bool       tryAllocate(TL::Block block, uint32_t alignment, StagingBlock& outBlock);

        bool       tryStreamImage(const ImageCopyInfo& dstImage, ImageSize3D size, uint32_t bytesPerRow, uint32_t rowsPerImage, TL::Block data, ImageBarrierState srcState, ImageBarrierState dstState);
        bool       tryStreamBuffer(const Buffer* buffer, uint64_t offset, TL::Block data, BufferBarrierState srcState, BufferBarrierState dstState);

        void       streamImage(const ImageCopyInfo& dstImage, ImageSize3D size, uint32_t bytesPerRow, uint32_t rowsPerImage, TL::Block data, ImageBarrierState srcState, ImageBarrierState dstState);
        void       streamBuffer(Buffer* buffer, uint64_t offset, TL::Block data, BufferBarrierState srcState, BufferBarrierState dstState);

        void       flush();

        Buffer*    getStagingBuffer() const { return stagingBuffer; }

        Fence*     getTransferFence() const { return fence; }

        uint64_t   getLastFlushedValue() const { return fenceValue; }

    private:
        static constexpr uint32_t      MaxStreamingFrames = 3;

        Device*                        device;
        Queue*                         transferQueue;
        CommandPool*                   transferCommandPool[MaxStreamingFrames];
        Fence*                         fence;
        uint64_t                       fenceValue;

        Buffer*                        stagingBuffer;
        DeviceMemoryPtr                stagingBufferMappedPtr;
        uint64_t                       stagingBufferCapacity;

        uint32_t                       framesInFlightCount;
        uint32_t                       activeFrameIndex;

        uint64_t                       stagingBufferCursor[MaxStreamingFrames];
        uint64_t                       stagingBufferRegionSignal[MaxStreamingFrames];

        TL::Vector<PendingBufferWrite> m_pendingBufferWrites;
        TL::Vector<PendingImageWrite>  m_pendingImageWrites;
    };
} // namespace RHI