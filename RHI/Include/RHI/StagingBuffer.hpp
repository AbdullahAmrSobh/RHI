#pragma once
#include "RHI/Resources.hpp"

namespace RHI
{
    struct TempBuffer
    {
        DeviceMemoryPtr pData;
        Handle<Buffer>  buffer;
        size_t          offset;
        size_t          size;
    };

    class RHI_EXPORT StagingBuffer
    {
    public:
        virtual ~StagingBuffer()                         = default;

        virtual TempBuffer Allocate(size_t newSize)      = 0;
        virtual void       Free(TempBuffer mappedBuffer) = 0;
        virtual void       Flush()                       = 0;
    };
} // namespace RHI