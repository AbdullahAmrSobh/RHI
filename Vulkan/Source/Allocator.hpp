#pragma once

#include <RHI/Handle.hpp>
#include <vk_mem_alloc.h>

namespace Vulkan
{

    enum class AllocationType
    {
        Default,
        /// @brief allocation is made from blocks managed internally
        Dedicated,
        /// @brief allocation creates its own dedicated block
        Aliasing,
        /// @brief multiple resources may share the same allocation
    };

    struct VirtualAllocation
    {
        VmaVirtualBlock blockHandle;
        VmaVirtualAllocation handle;
        size_t offset;
    };

    struct Allocation
    {
        VmaAllocation handle;
        VmaAllocationInfo info;
        AllocationType type;
        VirtualAllocation virtualAllocation;
    };

} // namespace Vulkan