#pragma once

#include <RHI/Buffer.hpp>

#include "MemoryAllocator.hpp"

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

    VkBufferUsageFlagBits ConvertBufferUsage(BufferUsage bufferUsage);

    VkBufferUsageFlags ConvertBufferUsageFlags(TL::Flags<BufferUsage> bufferUsageFlags);

    struct IBuffer : Buffer
    {
        Allocation allocation;
        VkBuffer handle;
        VkBufferCreateFlags flags;
        size_t size;
        VkBufferUsageFlags usage;

        ResultCode Init(IContext* context, const BufferCreateInfo& createInfo);
        void Shutdown(IContext* context);

        VkMemoryRequirements GetMemoryRequirements(IContext* context) const;
    };

    struct IBufferView : BufferView
    {
        VkBufferView handle;

        ResultCode Init(IContext* context, const BufferViewCreateInfo& useInfo);
        void Shutdown(IContext* context);
    };
} // namespace RHI::Vulkan