#include "Buffer.hpp"
#include "Common.hpp"
#include "Context.hpp"

namespace RHI::Vulkan
{

    VkBufferUsageFlagBits ConvertBufferUsage(BufferUsage bufferUsage)
    {
        switch (bufferUsage)
        {
        case BufferUsage::None:    return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
        case BufferUsage::Storage: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        case BufferUsage::Uniform: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        case BufferUsage::Vertex:  return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BufferUsage::Index:   return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case BufferUsage::CopySrc: return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        case BufferUsage::CopyDst: return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        default:                   TL_UNREACHABLE(); return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    VkBufferUsageFlags ConvertBufferUsageFlags(TL::Flags<BufferUsage> bufferUsageFlags)
    {
        VkBufferUsageFlags result = 0;
        if (bufferUsageFlags & BufferUsage::Storage)
            result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Uniform)
            result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Vertex)
            result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Index)
            result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::CopySrc)
            result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if (bufferUsageFlags & BufferUsage::CopyDst)
            result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        return result;
    }

    ResultCode IBuffer::Init(IContext* context, const BufferCreateInfo& createInfo)
    {
        this->flags = {};
        this->size = createInfo.byteSize;
        this->usage = ConvertBufferUsageFlags(createInfo.usageFlags);

        auto vmaUsage = createInfo.heapType == MemoryType::GPUShared ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        VmaAllocationCreateInfo allocationInfo{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
            .usage = vmaUsage,
            .requiredFlags = 0,
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool = VK_NULL_HANDLE,
            .pUserData = nullptr,
            .priority = 0.0f
        };
        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = this->flags,
            .size = this->size,
            .usage = this->usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
        };
        auto result = vmaCreateBuffer(
            context->m_allocator,
            &bufferCI,
            &allocationInfo,
            &handle,
            &allocation.handle,
            &allocation.info);

        if (result == VK_SUCCESS && createInfo.name)
        {
            context->SetDebugName(handle, createInfo.name);
        }

        return ConvertResult(result);
    }

    void IBuffer::Shutdown(IContext* context)
    {
        vmaDestroyBuffer(context->m_allocator, handle, allocation.handle);
    }

    VkMemoryRequirements IBuffer::GetMemoryRequirements(IContext* context) const
    {
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(context->m_device, handle, &requirements);
        return requirements;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// BufferView
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IBufferView::Init(IContext* context, const BufferViewCreateInfo& createInfo)
    {
        auto buffer = context->m_bufferOwner.Get(createInfo.buffer);
        TL_ASSERT(buffer);

        VkBufferViewCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .buffer = buffer->handle,
            .format = ConvertFormat(createInfo.format),
            .offset = createInfo.subregion.offset,
            .range = createInfo.subregion.size,
        };
        auto result = vkCreateBufferView(context->m_device, &bufferCI, nullptr, &handle);

        if (result == VK_SUCCESS && createInfo.name)
        {
            context->SetDebugName(this->handle, createInfo.name);
        }

        return ConvertResult(result);
    }

    void IBufferView::Shutdown(IContext* context)
    {
        vkDestroyBufferView(context->m_device, handle, nullptr);
    }

} // namespace RHI::Vulkan