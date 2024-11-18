#pragma once
#include "Common.hpp"

namespace RHI::Vulkan
{
    struct DeviceAllocation
    {
        VmaAllocation     handle;
        VmaAllocationInfo info;
    };
} // namespace RHI::Vulkan