#pragma once

#include <RHI/Buffer.hpp>
#include <RHI/Result.hpp>

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    class IDevice;

    VkBufferUsageFlags ConvertBufferUsageFlags(TL::Flags<BufferUsage> bufferUsageFlags);

    struct IBuffer : Buffer
    {
        VmaAllocation   allocation;
        VkBuffer        handle;
        VkBufferView    view; // optional
        Format          format;
        BufferSubregion subregion;

        ResultCode Init(IDevice* device, const BufferCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        VkMemoryRequirements GetMemoryRequirements(IDevice* device) const;
    };
} // namespace RHI::Vulkan