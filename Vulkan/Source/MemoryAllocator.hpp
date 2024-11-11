#pragma once
#include "Common.hpp"

namespace RHI::Vulkan
{
    struct Allocation
    {
        VmaAllocation        handle;
        VmaAllocationInfo    info;
        size_t               offset;
        VmaVirtualBlock      virtualBlock;
        VmaVirtualAllocation virtualHandle;
    };
} // namespace RHI::Vulkan