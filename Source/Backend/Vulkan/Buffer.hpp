#pragma once
#include "RHI/Buffer.hpp"

#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

class Buffer final
    : public IBuffer
    , public Resource<VkBuffer>
{
public:
    Buffer(Device& device)
        : Resource(device)
    {
    }
    ~Buffer();

    VkResult Init(const AllocationDesc& allocationDesc, const BufferDesc& desc);

    ResultCode SetDataInternal(size_t byteOffset, const uint8_t* bufferData, size_t bufferDataByteSize) override;
};

class BufferView final
    : public IBufferView
    , public DeviceObject<VkBufferView>
{
public:
    BufferView(Device& device)
        : DeviceObject(device)
    {
    }
    ~BufferView();

    VkResult Init(const Buffer& buffer, const BufferViewDesc& desc);
};

}  // namespace Vulkan
}  // namespace RHI