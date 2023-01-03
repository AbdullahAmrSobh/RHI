#include "RHI/Pch.hpp"

#include "Backend/Vulkan/Common.hpp"

#include "Backend/Vulkan/Buffer.hpp"

#include "Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{

Expected<Unique<IBuffer>> Device::CreateBuffer(const AllocationDesc& allocationDesc, const BufferDesc& desc)
{
    Unique<Buffer> buffer = CreateUnique<Buffer>(*this);
    VkResult       result = buffer->Init(allocationDesc, desc);

    if (Utils::IsSuccess(result))
        return std::move(buffer);

    return Unexpected(ConvertResult(result));
}

Expected<Unique<IBufferView>> Device::CreateBufferView(const IBuffer& buffer, const BufferViewDesc& desc)
{
    Unique<BufferView> bufferView = CreateUnique<BufferView>(*this);
    VkResult           result     = bufferView->Init(static_cast<const Buffer&>(buffer), desc);

    if (Utils::IsSuccess(result))
        return std::move(bufferView);

    return Unexpected(ConvertResult(result));
}

Buffer::~Buffer()
{
    if (m_handle)
    {
        vmaDestroyBuffer(m_device->GetAllocator(), m_handle, m_allocation);
    }
}

VkResult Buffer::Init(const AllocationDesc& allocationDesc, const BufferDesc& desc)
{
    *m_desc = desc;

    VkBufferCreateInfo createInfo {};
    createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext                 = nullptr;
    createInfo.flags                 = 0;
    createInfo.size                  = desc.size;
    createInfo.usage                 = ConvertBufferUsage(desc.usage);
    createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices   = nullptr;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage                   = ConvertMemoryUsage(allocationDesc.usage);

    VkResult result =
        vmaCreateBuffer(m_device->GetAllocator(), &createInfo, &allocationCreateInfo, &m_handle, &m_allocation, &m_allocationInfo);

    if (Utils::IsSuccess(result))
    {
        m_memorySize = m_allocationInfo.size;
    }

    return result;
}

ResultCode Buffer::SetDataInternal(size_t byteOffset, const uint8_t* bufferData, size_t bufferDataByteSize)
{
    uint8_t* data   = nullptr;
    VkResult result = vmaMapMemory(m_device->GetAllocator(), m_allocation, reinterpret_cast<void**>(&data));
    if (Utils::IsError(result))
    {
        return ConvertResult(result);
    }

    std::memcpy(data + byteOffset, bufferData, bufferDataByteSize);

    vmaUnmapMemory(m_device->GetAllocator(), m_allocation);
    return ResultCode::Success;
}

BufferView::~BufferView()
{
    if (m_handle)
    {
        vkDestroyBufferView(m_device->GetHandle(), m_handle, nullptr);
    }
}

VkResult BufferView::Init(const Buffer& buffer, const BufferViewDesc& desc)
{
    *m_desc = desc;

    VkBufferViewCreateInfo createInfo {};
    createInfo.sType  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    createInfo.pNext  = nullptr;
    createInfo.flags  = 0;
    createInfo.buffer = buffer.GetHandle();
    createInfo.format = ConvertFormat(desc.format);
    createInfo.offset = desc.range.byteOffset;
    createInfo.range  = desc.range.byteRange;

    return vkCreateBufferView(m_device->GetHandle(), &createInfo, nullptr, &m_handle);
}

}  // namespace Vulkan
}  // namespace RHI