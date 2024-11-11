#pragma once

#include "Device.hpp"

namespace RHI::Vulkan
{
    // Simple bump allocator
    class StagingBufferAllocator
    {
    public:
        StagingBufferAllocator();
        ~StagingBufferAllocator();

        ResultCode Init(IDevice* device);
        void       Shutdown();

        StagingBuffer Allocate(size_t size);

        void ReleaseAll();

    private:
        struct Page
        {
            DeviceMemoryPtr ptr;
            Handle<Buffer>  buffer;
            size_t          offset;
            size_t          size;

            size_t GetRemainingSize() const { return size - offset; }
        };

        IDevice*         m_device;
        TL::Vector<Page> m_pages;
    };
} // namespace RHI::Vulkan