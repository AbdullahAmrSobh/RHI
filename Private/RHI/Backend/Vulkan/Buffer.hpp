#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Buffer.hpp"

namespace RHI
{
namespace Vulkan
{
    class Buffer final
        : public IBuffer
        , public DeviceObject<VkBuffer>
    {
    public:
        inline Buffer(Device& device)
            : DeviceObject(device)
        {
        }
        ~Buffer();

        VkResult Init(const MemoryAllocationDesc& allocDesc, const BufferDesc& desc);

        inline virtual size_t GetSize() const override { return m_allocationInfo.size; }
        
        inline virtual Expected<DeviceAddress> Map(size_t offset, size_t range) override
        {
            DeviceAddress deviceAddress;
            VkResult      result = vmaMapMemory(m_pDevice->GetAllocator(), m_allocation, &deviceAddress);
            if (result != VK_SUCCESS)
                return tl::unexpected(ToResultCode(result));
            return deviceAddress;
        }
        
        inline virtual void Unmap() override { vmaMapMemory(m_pDevice->GetAllocator(), m_allocation, &m_deviceAddress); }
    
    private:
        VmaAllocation     m_allocation = VK_NULL_HANDLE;
        VmaAllocationInfo m_allocationInfo;
		DeviceAddress     m_deviceAddress = nullptr;
    };

    class BufferView final
        : public IBufferView
        , public DeviceObject<VkBufferView>
    {
    public:
        inline BufferView(Device& device)
            : DeviceObject(device)
        {
        }
        ~BufferView();

        VkResult Init(const BufferViewDesc& desc);
    };

} // namespace Vulkan
} // namespace RHI
