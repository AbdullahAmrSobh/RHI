#pragma once

#include <RHI/Buffer.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

#include "MemoryAllocator.hpp"

namespace RHI::Vulkan
{
    class IDevice;

    VkBufferUsageFlags ConvertBufferUsageFlags(TL::Flags<BufferUsage> bufferUsageFlags);

    struct IBuffer : Buffer
    {
        DeviceAllocation    allocation;
        VkBuffer            handle;
        VkBufferCreateFlags flags;
        size_t              size;
        VkBufferUsageFlags  usage;

        ResultCode Init(IDevice* device, const BufferCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        VkMemoryRequirements GetMemoryRequirements(IDevice* device) const;
    };
} // namespace RHI::Vulkan