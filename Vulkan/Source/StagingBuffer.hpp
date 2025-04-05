#pragma once

#include "Device.hpp"

namespace RHI::Vulkan
{
    // Simple bump allocator
    struct StagingBlock
    {
        Handle<Buffer> buffer;
        size_t         offset;
    };

    class StagingBuffer
    {
    public:
        StagingBuffer();
        ~StagingBuffer();

        ResultCode Init(IDevice* device);
        void       Shutdown();

        StagingBlock Allocate(size_t size);
        StagingBlock Allocate(TL::Block block);

        void ReleaseAll();

    private:
        struct Page
        {
            DeviceMemoryPtr ptr;
            Handle<Buffer>  buffer;
            size_t          offset;
            const size_t    size;

            size_t GetRemainingSize() const { return size - offset; }
        };

        IDevice*         m_device;
        TL::Vector<Page> m_pages;
    };
} // namespace RHI::Vulkan