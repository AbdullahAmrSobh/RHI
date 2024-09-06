#pragma once

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    struct Allocation
    {
        VmaAllocation handle;
        VmaAllocationInfo info;
        size_t offset;
        VmaVirtualBlock virtualBlock;
        VmaVirtualAllocation virtualHandle;
    };

    class MemoryAllocator
    {
    public:

    };
}