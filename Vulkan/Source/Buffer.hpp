#pragma once

#include <RHI/Result.hpp>
#include <RHI/Buffer.hpp>

#include "MemoryAllocator.hpp"

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;

    VkBufferUsageFlagBits ConvertBufferUsage(BufferUsage bufferUsage);

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